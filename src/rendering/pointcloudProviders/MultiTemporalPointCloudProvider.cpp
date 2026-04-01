#include "MultiTemporalPointCloudProvider.h"

#include "geometry/StaticPointCloudFactory.h"

#include <QFile>
#include <QtConcurrent>

namespace sahara::rendering
{

MultiTemporalPointCloudProvider::MultiTemporalPointCloudProvider(const std::filesystem::path& filepath, rendering::OpenGLContext* opengl_context)
	: AbstractPointCloudProvider(filepath.filename().string().c_str(), opengl_context, 1)
	, m_number_of_points(0)
{
	loadEpochsConcurrently(filepath);

	m_opengl_context->makeCurrent();
	createGPUBuffers();

	m_epoch_parameter = std::make_unique<RangeParameter<int>>("Epoch", 0, 0, static_cast<int>(m_pointclouds.size() - 1), 1);
	connect(m_epoch_parameter.get(), &RangeParameter<int>::valueChanged, this, &MultiTemporalPointCloudProvider::updateCommandBuffer);
	m_parameters.push_back(m_epoch_parameter.get());

	updateCommandBuffer();

	m_opengl_context->doneCurrent();
	m_is_valid = true;
}

MultiTemporalPointCloudProvider::~MultiTemporalPointCloudProvider()
{
}

void MultiTemporalPointCloudProvider::loadEpochsConcurrently(const std::filesystem::path& filepath)
{
	QFile file(filepath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qWarning() << "Could not open file " << filepath.c_str();
		return;
	}

	std::vector<std::future<std::unique_ptr<geometry::StaticPointCloud>>> tasks;
	while (!file.atEnd())
	{
		QString path = QFileInfo(file).absoluteDir().path() + "/" + file.readLine().trimmed();
		tasks.push_back(std::async(std::launch::async, [path]() { return geometry::StaticPointCloudFactory::loadFromPointCloudFile(path.toStdString()); }));
	}
	file.close();

	for (auto& task : tasks)
	{
		m_pointclouds.emplace_back(std::move(task.get()));
		m_buffer_offsets.push_back(m_number_of_points);
		m_number_of_points += static_cast<int>(m_pointclouds.back()->numberOfPoints()); // as the maximum buffer size is limited by int, we also use int for m_number_of_points
		m_bounding_box.extendToIncludePoint(m_pointclouds.back()->boundingBox().minimum());
		m_bounding_box.extendToIncludePoint(m_pointclouds.back()->boundingBox().maximum());
	}
}

void MultiTemporalPointCloudProvider::createGPUBuffers()
{
	std::unordered_map<geometry::AttributeSemantic, geometry::AttributeMetadata*> available_attributes;
	for (const auto& pointcloud : m_pointclouds)
	{
		for (const auto& attribute : pointcloud->attributes())
		{
			available_attributes.insert_or_assign(attribute.first, attribute.second.metadata.get());
		}
	}

	// create and fill the attribute buffers
	for (const auto& attribute : available_attributes)
	{
		const auto semantic = attribute.first;
		m_available_attributes.emplace(semantic, attribute.second);

		m_gpu_buffers.emplace(semantic, QOpenGLBuffer::Type::VertexBuffer);
		m_gpu_buffers[semantic].create();
		m_gpu_buffers[semantic].setUsagePattern(QOpenGLBuffer::UsagePattern::StaticDraw);

		m_gpu_buffers[semantic].bind();
		m_gpu_buffers[semantic].allocate(m_number_of_points * attribute.second->singleEntrySizeInBytes());

		int offset = 0;
		for (const auto& pointcloud : m_pointclouds)
		{
			const auto number_of_bytes = static_cast<int>(pointcloud->numberOfPoints() * attribute.second->singleEntrySizeInBytes());
			if (pointcloud->hasAttribute(semantic))
			{
				m_gpu_buffers[semantic].write(offset, pointcloud->attributeData(semantic)->rawData(), number_of_bytes);
			}
			else
			{
				std::vector<uint8_t> zeros(number_of_bytes, 0);
				m_gpu_buffers[semantic].write(offset, static_cast<const void*>(zeros.data()), number_of_bytes);
			}
			offset += number_of_bytes;
		}
		m_gpu_buffers[semantic].release();
	}
}

size_t MultiTemporalPointCloudProvider::numberOfPoints() const
{
	return m_number_of_points;
}

int MultiTemporalPointCloudProvider::pointBudget() const
{
	return m_number_of_points;
}

const geometry::BoundingBox& MultiTemporalPointCloudProvider::boundingBox() const
{
	return m_bounding_box;
}

PointCloudProviderType MultiTemporalPointCloudProvider::type() const noexcept
{
	return PointCloudProviderType::MultiTemporalPointCloudProvider;
}

std::vector<RasterizerType> MultiTemporalPointCloudProvider::supportedRasterizers() const noexcept
{
	return { RasterizerType::PointPrimitiveRasterizer };
}

void MultiTemporalPointCloudProvider::update()
{
	// Nothing to do here. The point clouds were already loaded in full.
}

void MultiTemporalPointCloudProvider::updateCommandBuffer()
{
	const auto current_epoch = m_epoch_parameter->value();
	m_draw_command_buffer.updateDrawCommand(0, m_pointclouds[current_epoch]->numberOfPoints(), m_buffer_offsets[current_epoch], 0, true);
	m_draw_command_buffer.updateOnGPU();
}

void MultiTemporalPointCloudProvider::bindGPUBuffer(geometry::AttributeSemantic semantic)
{
	assert(hasAttribute(semantic));
	m_gpu_buffers[semantic].bind();
}

void MultiTemporalPointCloudProvider::releaseGPUBuffer(geometry::AttributeSemantic semantic)
{
	assert(hasAttribute(semantic));
	m_gpu_buffers[semantic].release();
}

}
