#include "OOCPointCloudLoader.h"

#include "utils/Profiler.h"

namespace sahara::io
{
OOCPointCloudLoader::OOCPointCloudLoader(const std::filesystem::path& data_path, uint number_of_points)
	: m_data_directory(data_path)
	, m_number_of_points(number_of_points)
	, m_threads_should_stop(false)
	, m_loading_queue(4000)
{
	m_worker_threads.setExpiryTimeout(-1);
}

OOCPointCloudLoader::~OOCPointCloudLoader()
{
	m_threads_should_stop = true;
	m_loading_queue_not_empty.wakeAll();
	m_eviction_queue_not_empty.wakeAll();
	m_worker_threads.waitForDone();
}

void OOCPointCloudLoader::resetLoadingQueueAndPauseLoading()
{
	m_loading_queue_mutex.lock();
	m_attribute_data_mutex.lock(true);
	m_loading_queue.clear();
}

void OOCPointCloudLoader::addToLoadingQueue(geometry::LodOctreeNode* const node)
{
	if (!node->m_is_loading)
	{
		m_loading_queue.push(node);
	}
}

void OOCPointCloudLoader::resumeLoading()
{
	m_queue_eviction_future.wait();
	m_loading_queue_not_empty.wakeAll();
	m_attribute_data_mutex.unlock();
	m_loading_queue_mutex.unlock();
}

void OOCPointCloudLoader::setRequiredAttributes(std::vector<const geometry::AttributeMetadata*>& required_attributes)
{
	m_required_attributes = required_attributes;

	m_threads_should_stop = true;
	m_loading_queue_not_empty.wakeAll();
	m_eviction_queue_not_empty.wakeAll();
	m_worker_threads.waitForDone();
	m_threads_should_stop = false;

	for (int i = 0; i < NUMBER_OF_LOADING_THREADS; i++)
	{
		m_attribute_loaders[i].clear();
		for (const auto attribute : m_required_attributes)
		{
			createAttributeLoaderForThread(i, attribute);
		}
		m_worker_threads.start(std::bind(&OOCPointCloudLoader::processLoadingQueue, this, i));
	}
	m_worker_threads.start(std::bind(&OOCPointCloudLoader::processEvictionQueue, this));
}

void OOCPointCloudLoader::createAttributeLoaderForThread(int index, const geometry::AttributeMetadata* attribute)
{
	// auto filepath = m_data_directory / (attribute->name.toStdString() + ".bin");
	m_attribute_loaders[index][attribute->semantic] = std::array<std::unique_ptr<io::AbstractOOCAttributeLoader>, 2>{};
	switch (attribute->semantic)
	{
		case geometry::AttributeSemantic::Position:
			m_attribute_loaders[index][attribute->semantic][0] = std::move(std::make_unique<io::OOCAttributeLoader<QVector3D>>(m_data_directory / "points.bin"));
			m_attribute_loaders[index][attribute->semantic][1] = std::move(std::make_unique<io::OOCAttributeLoader<uint32_t>>(m_data_directory / "voxels.bin"));
			break;
		case geometry::AttributeSemantic::Normal:
			m_attribute_loaders[index][attribute->semantic][0] = std::move(std::make_unique<io::OOCAttributeLoader<QVector3D>>(m_data_directory / (attribute->name.toStdString() + "_points.bin")));
			m_attribute_loaders[index][attribute->semantic][1] = std::move(std::make_unique<io::OOCAttributeLoader<QVector3D>>(m_data_directory / (attribute->name.toStdString() + "_voxels.bin")));
			break;
		case geometry::AttributeSemantic::Color:
			m_attribute_loaders[index][attribute->semantic][0] = std::move(std::make_unique<io::OOCAttributeLoader<geometry::Color>>(m_data_directory / (attribute->name.toStdString() + "_points.bin")));
			m_attribute_loaders[index][attribute->semantic][1] = std::move(std::make_unique<io::OOCAttributeLoader<geometry::Color>>(m_data_directory / (attribute->name.toStdString() + "_voxels.bin")));
			break;
		case geometry::AttributeSemantic::SegmentID:
		case geometry::AttributeSemantic::ID:
			m_attribute_loaders[index][attribute->semantic][0] = std::move(std::make_unique<io::OOCAttributeLoader<uint>>(m_data_directory / (attribute->name.toStdString() + "_points.bin")));
			m_attribute_loaders[index][attribute->semantic][1] = std::move(std::make_unique<io::OOCAttributeLoader<uint>>(m_data_directory / (attribute->name.toStdString() + "_voxels.bin")));
			break;
		default:
			switch (attribute->type)
			{
				case geometry::AttributeType::Float:
					m_attribute_loaders[index][attribute->semantic][0] = std::move(std::make_unique<io::OOCAttributeLoader<float>>(m_data_directory / (attribute->name.toStdString() + "_points.bin")));
					m_attribute_loaders[index][attribute->semantic][1] = std::move(std::make_unique<io::OOCAttributeLoader<float>>(m_data_directory / (attribute->name.toStdString() + "_voxels.bin")));
					break;
				case geometry::AttributeType::Int:
					m_attribute_loaders[index][attribute->semantic][0] = std::move(std::make_unique<io::OOCAttributeLoader<int>>(m_data_directory / (attribute->name.toStdString() + "_points.bin")));
					m_attribute_loaders[index][attribute->semantic][1] = std::move(std::make_unique<io::OOCAttributeLoader<int>>(m_data_directory / (attribute->name.toStdString() + "_voxels.bin")));
					break;
				case geometry::AttributeType::Uint:
					m_attribute_loaders[index][attribute->semantic][0] = std::move(std::make_unique<io::OOCAttributeLoader<uint>>(m_data_directory / (attribute->name.toStdString() + "_points.bin")));
					m_attribute_loaders[index][attribute->semantic][1] = std::move(std::make_unique<io::OOCAttributeLoader<uint>>(m_data_directory / (attribute->name.toStdString() + "_voxels.bin")));
					break;
				case geometry::AttributeType::Color:
					m_attribute_loaders[index][attribute->semantic][0] = std::move(std::make_unique<io::OOCAttributeLoader<geometry::Color>>(m_data_directory / (attribute->name.toStdString() + "_points.bin")));
					m_attribute_loaders[index][attribute->semantic][1] = std::move(std::make_unique<io::OOCAttributeLoader<geometry::Color>>(m_data_directory / (attribute->name.toStdString() + "_voxels.bin")));
					break;
				case geometry::AttributeType::Vector3D:
					m_attribute_loaders[index][attribute->semantic][0] = std::move(std::make_unique<io::OOCAttributeLoader<QVector3D>>(m_data_directory / (attribute->name.toStdString() + "_points.bin")));
					m_attribute_loaders[index][attribute->semantic][1] = std::move(std::make_unique<io::OOCAttributeLoader<QVector3D>>(m_data_directory / (attribute->name.toStdString() + "_voxels.bin")));
					break;
				default:
					assert(false && "Undefined attribute type");
					break;
			}
			break;
	}
}

void OOCPointCloudLoader::reset()
{
	for (auto it = m_loaded_nodes.begin(); it != m_loaded_nodes.end();)
	{
		(*it)->m_attribute_data.reset(nullptr);
		(*it)->m_priority = 0.0f;
		it = m_loaded_nodes.erase(it);
	}
	m_loaded_nodes.clear();
}

void OOCPointCloudLoader::pinNodes(std::vector<bool>& nodes_to_keep)
{
	utils::global_profiler.addMeasurement("Loaded_Nodes", static_cast<int>(m_loaded_nodes.size()));
	m_queue_eviction_future = std::async(std::launch::async, [this, nodes_to_keep]() {
		m_eviction_mutex.lock(true);
		for (auto it = m_loaded_nodes.begin(); it != m_loaded_nodes.end();)
		{
			if (nodes_to_keep[(*it)->m_index] == false)
			{
				m_eviction_queue.emplace(std::move((*it)->m_attribute_data));
				(*it)->m_attribute_data.release();
				(*it)->m_priority = 0.0f;
				it = m_loaded_nodes.erase(it);
			}
			else
			{
				it++;
			}
		}
		m_eviction_mutex.unlock();

		if (m_eviction_queue.size() > 0)
		{
			m_eviction_queue_not_empty.wakeAll();
		}
	});
}

void OOCPointCloudLoader::processLoadingQueue(int thread_index)
{
	while (!m_threads_should_stop)
	{
		m_loading_queue_mutex.lock();
		while (m_loading_queue.empty())
		{
			m_loading_queue_not_empty.wait(&m_loading_queue_mutex);
			if (m_threads_should_stop)
			{
				m_loading_queue_mutex.unlock();
				return;
			}
		}
		auto node = m_loading_queue.front();
		node->m_is_loading = true;
		m_loading_queue.pop();
		m_loading_queue_mutex.unlock();

		loadAttributeData(node, thread_index);
	}
}

// TODO: could probably refactor this and processLoadingQueue into some WorkerThread class that waits on a queue and processes items once signalled
void OOCPointCloudLoader::processEvictionQueue()
{
	while (!m_threads_should_stop)
	{
		m_eviction_mutex.lock();
		while (m_eviction_queue.empty())
		{
			m_eviction_mutex.wait(&m_eviction_queue_not_empty);
			if (m_threads_should_stop)
			{
				m_eviction_mutex.unlock();
				for (int i = 0; i < m_eviction_queue.size(); i++)
				{
					m_eviction_queue.pop();
				}
				return;
			}
		}

		if (!m_eviction_queue.empty())
		{
			m_eviction_queue.pop();
		}
		m_eviction_mutex.unlock();
	}

	for (int i = 0; i < m_eviction_queue.size(); i++)
	{
		m_eviction_queue.pop();
	}
}

void OOCPointCloudLoader::loadAttributeData(geometry::LodOctreeNode* const node, int thread_index)
{
	auto result = std::make_unique<geometry::NodeAttributeData>();
	for (const auto& metadata : m_required_attributes)
	{
		result->emplace(metadata->semantic, std::move(m_attribute_loaders[thread_index][metadata->semantic][node->m_is_leaf ? 0 : 1]->loadAttributeData(node->m_data_index, node->m_number_of_points)));
	}
	m_attribute_data_mutex.lock();
	node->m_attribute_data = std::move(result);
	node->m_is_loading = false;
	m_loaded_nodes.push_back(node);
	m_attribute_data_mutex.unlock();
}
}