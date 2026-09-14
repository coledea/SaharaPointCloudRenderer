#include "MultiTemporalPointCloudProvider.h"

#include "geometry/StaticPointCloudFactory.h"
#include "io/CSVLoadingDialog.h"
#include "utils/NumericLimits.h"
#include "utils/QVector3DUtilities.h"

#include <QFile>
#include <QtConcurrent>
#include <algorithm>
#include <optional>

namespace sahara::rendering
{

template <typename T>
std::unique_ptr<geometry::AttributeMetadata> createCombinedTypedMetadata(const std::vector<std::unique_ptr<geometry::StaticPointCloud>>& pointclouds, const geometry::AttributeMetadata& representative_attribute)
{
	T minimum = sahara::utils::numeric_limits<T>::max();
	T maximum = sahara::utils::numeric_limits<T>::lowest();

	for (const auto& pointcloud : pointclouds)
	{
		auto attribute_metadata = pointcloud->attributeMetadata(representative_attribute);
		if (attribute_metadata == nullptr)
		{
			continue;
		}

		const auto typed_metadata = dynamic_cast<const geometry::TypedAttributeMetadata<T>*>(attribute_metadata);
		using namespace std; // Necessary for the compiler to find overloads for native datatypes
		minimum = min(minimum, typed_metadata->minimum);
		maximum = max(maximum, typed_metadata->maximum);
	}

	return std::make_unique<geometry::TypedAttributeMetadata<T>>(representative_attribute.name, representative_attribute.semantic, minimum, maximum);
}

std::unique_ptr<geometry::AttributeMetadata> createCombinedMetadata(const std::vector<std::unique_ptr<geometry::StaticPointCloud>>& pointclouds, const geometry::AttributeMetadata& representative_attribute)
{
	switch (representative_attribute.type)
	{
		case geometry::AttributeType::Int:
			return createCombinedTypedMetadata<int>(pointclouds, representative_attribute);
		case geometry::AttributeType::Uint:
			return createCombinedTypedMetadata<uint>(pointclouds, representative_attribute);
		case geometry::AttributeType::Float:
			return createCombinedTypedMetadata<float>(pointclouds, representative_attribute);
		case geometry::AttributeType::Vector3D:
			return createCombinedTypedMetadata<QVector3D>(pointclouds, representative_attribute);
		case geometry::AttributeType::Color:
			return createCombinedTypedMetadata<geometry::Color>(pointclouds, representative_attribute);
		default:
			assert(false && "Unsupported attribute type");
			return nullptr;
	}
}

MultiTemporalPointCloudProvider::MultiTemporalPointCloudProvider(const std::filesystem::path& filepath, rendering::OpenGLContext* opengl_context)
	: AbstractPointCloudProvider(filepath.filename().string().c_str(), opengl_context, 1)
	, m_number_of_points(0)
	, m_provide_multiple_timestamps(false)
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
	auto column_settings = io::CSVColumnSettings::empty;
	while (!file.atEnd())
	{
		QString path = QFileInfo(file).absoluteDir().path() + "/" + file.readLine().trimmed();
		bool is_csv = path.endsWith(".csv") || path.endsWith(".txt") || path.endsWith(".xyz") || path.endsWith(".ascii");
		if (is_csv && column_settings.m_columns.empty())
		{
			auto loading_dialog = sahara::io::CSVLoadingDialog(path);
			if (loading_dialog.exec() == QDialog::Rejected)
			{
				file.close();
				return;
			}
			column_settings = loading_dialog.columnSettings();
		}
		tasks.push_back(std::async(std::launch::async, [path, column_settings]() { return geometry::StaticPointCloudFactory::loadFromPointCloudFile(path.toStdString(), column_settings); }));
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

// Since the different timestamps might possess different attributes, we need to unify the attributes across all timestamps
void MultiTemporalPointCloudProvider::createCombinedAttributeMetadata()
{
	std::optional<geometry::AttributeSemantic> next_custom_attribute = geometry::AttributeSemantic::Custom0;

	for (const auto& pointcloud : m_pointclouds)
	{
		for (const auto& [semantic, attribute] : pointcloud->attributes())
		{
			if (std::ranges::any_of(m_combined_attribute_metadata, [&attribute](const auto& combined_metadata) { return *combined_metadata == *attribute.metadata; }))
			{
				continue;
			}

			if (!geometry::attributeIsCustom(attribute.metadata->semantic))
			{
				m_combined_attribute_metadata.push_back(createCombinedMetadata(m_pointclouds, *attribute.metadata));
				m_available_attributes.emplace(semantic, m_combined_attribute_metadata.back().get());
				continue;
			}

			if (!next_custom_attribute.has_value())
			{
				qWarning() << "Too many custom attributes in multi-temporal point cloud. Attribute skipped:" << attribute.metadata->name;
				continue;
			}

			geometry::AttributeMetadata custom_metadata(attribute.metadata->name, next_custom_attribute.value(), attribute.metadata->type);
			m_combined_attribute_metadata.push_back(createCombinedMetadata(m_pointclouds, custom_metadata));
			m_available_attributes.emplace(next_custom_attribute.value(), m_combined_attribute_metadata.back().get());

			next_custom_attribute = geometry::nextCustomSemantic(next_custom_attribute.value());
		}
	}
}

void MultiTemporalPointCloudProvider::createGPUBuffers()
{
	createCombinedAttributeMetadata();

	// create and fill the attribute buffers
	for (const auto& attribute : m_combined_attribute_metadata)
	{
		const auto semantic = attribute->semantic;

		m_gpu_buffers.emplace(semantic, QOpenGLBuffer::Type::VertexBuffer);
		m_gpu_buffers[semantic].create();
		m_gpu_buffers[semantic].setUsagePattern(QOpenGLBuffer::UsagePattern::StaticDraw);

		m_gpu_buffers[semantic].bind();
		m_gpu_buffers[semantic].allocate(m_number_of_points * attribute->singleEntrySizeInBytes());

		int offset = 0;
		for (const auto& pointcloud : m_pointclouds)
		{
			const auto number_of_bytes = static_cast<int>(pointcloud->numberOfPoints() * attribute->singleEntrySizeInBytes());

			if (pointcloud->hasAttribute(*attribute))
			{
				m_gpu_buffers[semantic].write(offset, pointcloud->attributeData(*attribute)->rawData(), number_of_bytes);
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
	return { RasterizerType::PointPrimitiveRasterizer, RasterizerType::MultiTemporalPointPrimitiveRasterizer };
}

void MultiTemporalPointCloudProvider::update()
{
	// Nothing to do here. The point clouds were already loaded in full.
}

void MultiTemporalPointCloudProvider::updateCommandBuffer()
{
	const auto current_epoch = m_epoch_parameter->value();
	m_draw_command_buffer.updateDrawCommand(0, m_pointclouds[current_epoch]->numberOfPoints(), m_buffer_offsets[current_epoch], 0, true);

	if (m_provide_multiple_timestamps && current_epoch + 1 < m_pointclouds.size())
	{
		m_draw_command_buffer.updateDrawCommand(1, m_pointclouds[current_epoch + 1]->numberOfPoints(), m_buffer_offsets[current_epoch + 1], 1, true);
	}

	m_draw_command_buffer.updateOnGPU();
}

void MultiTemporalPointCloudProvider::bindGPUBuffer(geometry::AttributeSemantic semantic)
{
	assert(hasAttribute(semantic));
	m_gpu_buffers[semantic].bind();
}

GLuint MultiTemporalPointCloudProvider::getGPUBuffer(geometry::AttributeSemantic semantic)
{
	assert(hasAttribute(semantic));
	return m_gpu_buffers[semantic].bufferId();
}

void MultiTemporalPointCloudProvider::releaseGPUBuffer(geometry::AttributeSemantic semantic)
{
	assert(hasAttribute(semantic));
	m_gpu_buffers[semantic].release();
}

void MultiTemporalPointCloudProvider::setProvideMultipleTimestamps(const bool provide_multiple_timestamps)
{
	if (provide_multiple_timestamps == m_provide_multiple_timestamps)
	{
		return;
	}

	m_provide_multiple_timestamps = provide_multiple_timestamps;
	m_draw_command_buffer.setCapacity(m_provide_multiple_timestamps ? 2 : 1);
	m_draw_command_buffer.resetCommands(m_provide_multiple_timestamps ? 2 : 1);
	updateCommandBuffer();
}

void MultiTemporalPointCloudProvider::decreaseTimestamp()
{
	m_epoch_parameter->setValue((m_epoch_parameter->value() - 1 + m_epoch_parameter->maximum()) % m_epoch_parameter->maximum());
	updateCommandBuffer();
}

void MultiTemporalPointCloudProvider::increaseTimestamp()
{
	m_epoch_parameter->setValue((m_epoch_parameter->value() + 1) % m_epoch_parameter->maximum());
	updateCommandBuffer();
}

}
