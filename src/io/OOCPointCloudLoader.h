#pragma once

#include "OOCAttributeLoader.h"
#include "geometry/AttributeData.h"
#include "geometry/AttributeSemantics.h"
#include "geometry/LodOctreeNode.h"
#include "utils/LimitedQueue.h"
#include "utils/PriorityMutex.h"

#include <QMutex>
#include <QThreadPool>
#include <QWaitCondition>
#include <queue>

namespace sahara::io
{

class OOCPointCloudLoader
{
public:
	OOCPointCloudLoader(const std::filesystem::path& data_path, uint number_of_points);
	~OOCPointCloudLoader();

	void resetLoadingQueueAndPauseLoading(); // locks the loading queue and attribute data mutexes
	void addToLoadingQueue(geometry::LodOctreeNode* const node);
	void resumeLoading(); // wakes the loading threads, waits for the eviction queue filling thread to finish, and unlocks the loading queue and attribute data mutexes

	void setRequiredAttributes(std::vector<const geometry::AttributeMetadata*>& required_attributes);
	void reset();
	void pinNodes(std::vector<bool>& nodes_to_keep);

private:
	static constexpr int NUMBER_OF_LOADING_THREADS = 10;

	uint m_number_of_points;

	std::filesystem::path m_data_directory;
	utils::LimitedQueue<geometry::LodOctreeNode*> m_loading_queue;
	std::vector<const geometry::AttributeMetadata*> m_required_attributes;

	std::queue<std::unique_ptr<geometry::NodeAttributeData>> m_eviction_queue;
	utils::PriorityMutex m_eviction_mutex;
	QWaitCondition m_eviction_queue_not_empty;

	QThreadPool m_worker_threads;
	QMutex m_loading_queue_mutex;
	QWaitCondition m_loading_queue_not_empty;
	utils::PriorityMutex m_attribute_data_mutex;

	std::future<void> m_queue_eviction_future;
	bool m_threads_should_stop;
	std::array<std::unordered_map<geometry::AttributeSemantic, std::array<std::unique_ptr<io::AbstractOOCAttributeLoader>, 2>>, NUMBER_OF_LOADING_THREADS> m_attribute_loaders;

	std::list<geometry::LodOctreeNode*> m_loaded_nodes;

	void processLoadingQueue(int thread_index);
	void processEvictionQueue();
	void createAttributeLoaderForThread(int index, const geometry::AttributeMetadata* attribute);
	void loadAttributeData(geometry::LodOctreeNode* const node, int thread_index);
};

}