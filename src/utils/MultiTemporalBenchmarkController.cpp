#include "MultiTemporalBenchmarkController.h"

#include "io/CameraPathIO.h"
#include "rendering/pointcloudProviders/OOCMultiTemporalPointCloudProvider.h"
#include "utils/Profiler.h"

namespace sahara::utils
{

const std::vector<float> MultiTemporalBenchmarkController::m_benchmark_timestamp_factors{ 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };
const std::vector<int> MultiTemporalBenchmarkController::m_benchmark_timestamp_ranges{ 0, 1, 5, 10, 25 };
const int MultiTemporalBenchmarkController::m_random_seed = 24979232; // fixed seed for comparability (same timestamp switches across different parameter configurations)

MultiTemporalBenchmarkController::MultiTemporalBenchmarkController(rendering::OOCMultiTemporalPointCloudProvider* provider)
	: m_pointcloud_provider(provider)
	, m_timestamp_parameter(provider->m_timestamp_parameter.get())
	, m_benchmark_mode(BenchmarkMode::Temporal)
	, m_benchmark_active(false)
	, m_number_of_timestamp_switches(0)
	, m_current_configuration_index(0)
	, m_benchmark_camera_path(nullptr)
	, m_next_switch_is_spatial(false)
	, m_next_camera_position(0)
{
	connect(m_pointcloud_provider->m_navigation_handler, &navigation::NavigationHandler::cameraPathAnimationStopped, this, &MultiTemporalBenchmarkController::onCameraAnimationFinished);
}

MultiTemporalBenchmarkController::~MultiTemporalBenchmarkController()
{
}

void MultiTemporalBenchmarkController::setBenchmarkMode(BenchmarkMode benchmark_mode) noexcept
{
	m_benchmark_mode = benchmark_mode;
}

void MultiTemporalBenchmarkController::setCameraPositionsPath(QString camera_positions_path)
{
	if (camera_positions_path.isEmpty())
	{
		return;
	}

	std::unique_ptr<navigation::CameraAnimationPath> camera_path(io::importCameraPathFromJson(camera_positions_path));
	if (camera_path->empty())
	{
		return;
	}

	m_benchmark_camera_path = std::move(camera_path);
}

int MultiTemporalBenchmarkController::timestampRangeForConfiguration(int configuration)
{
	return m_benchmark_timestamp_ranges[configuration / m_benchmark_timestamp_factors.size()];
}

float MultiTemporalBenchmarkController::timestampFactorForConfiguration(int configuration)
{
	return m_benchmark_timestamp_factors[configuration % m_benchmark_timestamp_factors.size()];
}

int MultiTemporalBenchmarkController::currentConfiguration() const
{
	return m_benchmark_configurations[m_current_configuration_index];
}

void MultiTemporalBenchmarkController::startBenchmark()
{
	m_benchmark_active = true;

	if (m_benchmark_mode == BenchmarkMode::Spatial && (m_benchmark_camera_path == nullptr || m_benchmark_camera_path->empty()))
	{
		qWarning() << "No valid camera path specified for benchmarking with camera animation!";
		m_pointcloud_provider->m_trigger_benchmark_parameter->deactivate();
		return;
	}

	initializeBenchmarkConfigurations();
	m_current_configuration_index = 0;
	startBenchmarkForCurrentConfiguration();
}

void MultiTemporalBenchmarkController::stopBenchmark()
{
	if (!m_benchmark_active)
	{
		return;
	}

	m_benchmark_active = false;
	if (m_benchmark_mode == BenchmarkMode::Temporal)
	{
		m_timestamp_switch_timer.stop();
	}
	else
	{
		m_pointcloud_provider->m_navigation_handler->stopAnimatedPathNavigation();
	}
	utils::global_profiler.endSession();
}

void MultiTemporalBenchmarkController::initializeBenchmarkConfigurations()
{
	m_benchmark_configurations.clear();

	// find last range value that is effective given the number of timestamps, to filter out configurations with identical effective timestamp ranges due to a low number of overall timestamps
	int last_timestamp_range = -1;
	for (auto range : m_benchmark_timestamp_ranges)
	{
		last_timestamp_range = range;
		if (m_pointcloud_provider->m_number_of_timestamps - 1 <= range) // the largest possible jump is from a border timestamp to the other border timestamp
		{
			break;
		}
	}

	bool range_zero_already_present = false;
	for (int configuration = 0; configuration < m_benchmark_timestamp_factors.size() * m_benchmark_timestamp_ranges.size(); ++configuration)
	{
		int timestamp_range = timestampRangeForConfiguration(configuration);

		if (timestamp_range == 0 && range_zero_already_present)
		{
			continue; // we only want one configuration with a timestamp range of zero, since the different factors don't matter there
		}

		if (timestamp_range > last_timestamp_range)
		{
			break;
		}

		m_benchmark_configurations.push_back(configuration);
		range_zero_already_present |= timestamp_range == 0;
	}
}

void MultiTemporalBenchmarkController::resetBenchmarkState()
{
	m_timestamp_parameter->setValue(0); // reset to first timestamp to have comparable starting conditions
	m_random_generator.seed(m_random_seed);
	m_number_of_timestamp_switches = 0;
	m_next_switch_is_spatial = false;
	m_next_camera_position = 0;
	m_pointcloud_provider->m_chunk_loader->reset(); // empty caches to avoid any caching effects between different benchmark configurations
}

void MultiTemporalBenchmarkController::startBenchmarkForCurrentConfiguration()
{
	resetBenchmarkState();

	m_pointcloud_provider->m_timestamp_range_parameter->setValue(timestampRangeForConfiguration(currentConfiguration()));
	m_pointcloud_provider->m_timestamp_falloff_parameter->setValue(timestampFactorForConfiguration(currentConfiguration()));
	utils::global_profiler.startSession("benchmark_results/benchmark_" + std::to_string(currentConfiguration()), 100000);

	if (m_benchmark_mode == BenchmarkMode::Temporal)
	{
		startTimestampSwitches();
	}
	else
	{
		startCameraPathNavigation();
	}
}

void MultiTemporalBenchmarkController::startTimestampSwitches()
{
	m_timestamp_switch_timer.start(m_pointcloud_provider->m_timestamp_duration_parameter->value() * 1000.0f, this);
}

void MultiTemporalBenchmarkController::startCameraPathNavigation()
{
	m_pointcloud_provider->m_navigation_handler->activateAnimatedPathNavigation(*m_benchmark_camera_path, true, false);
}

void MultiTemporalBenchmarkController::switchCameraPosition()
{
	m_pointcloud_provider->m_camera->setCameraSpecification(m_benchmark_camera_path->at(m_next_camera_position++ % m_benchmark_camera_path->size()).camera);
	m_next_switch_is_spatial = false;
}

void MultiTemporalBenchmarkController::switchTimestamp()
{
	int current_timestamp = m_timestamp_parameter->value();
	int new_timestamp = current_timestamp;
	int within_range_min = std::max(m_timestamp_parameter->minimum(), current_timestamp - m_benchmark_timestamp_ranges.back());
	int within_range_max = std::min(m_timestamp_parameter->maximum(), current_timestamp + m_benchmark_timestamp_ranges.back());
	int without_range_min = within_range_max + 1;
	int without_range_max = without_range_min + (m_pointcloud_provider->m_number_of_timestamps - (within_range_max - within_range_min + 1));

	while (new_timestamp == current_timestamp) // we want a different timestamp than the current one
	{
		if (within_range_min != within_range_max && m_random_generator.bounded(0, 10) < 9) // in 90% of the cases, we pick a timestamp within the maximum possible timestamp range. This amounts to ~21 switches per distance and 60 switches outside the range.
		{
			new_timestamp = m_random_generator.bounded(within_range_min, within_range_max + 1);
		}
		else
		{
			new_timestamp = m_random_generator.bounded(without_range_min, without_range_max) % (m_timestamp_parameter->maximum() + 1);
		}
	}
	m_timestamp_parameter->setValue(new_timestamp);
	m_next_switch_is_spatial = true;
}

void MultiTemporalBenchmarkController::switchToNextConfiguration()
{
	if (++m_current_configuration_index < m_benchmark_configurations.size())
	{
		utils::global_profiler.endSession();
		startBenchmarkForCurrentConfiguration();
		return;
	}

	stopBenchmark();
	m_pointcloud_provider->m_trigger_benchmark_parameter->deactivate();
}

void MultiTemporalBenchmarkController::timerEvent(QTimerEvent* e)
{
	if (e->timerId() != m_timestamp_switch_timer.timerId())
	{
		return;
	}

	if (m_benchmark_camera_path != nullptr && m_next_switch_is_spatial)
	{
		switchCameraPosition();
		return;
	}

	if (m_number_of_timestamp_switches++ < m_pointcloud_provider->m_benchmark_num_samples_parameter->value())
	{
		switchTimestamp();
		return;
	}

	switchToNextConfiguration();
}

void MultiTemporalBenchmarkController::onCameraAnimationFinished()
{
	if (!m_benchmark_active)
	{
		return;
	}

	if (m_number_of_timestamp_switches++ < m_pointcloud_provider->m_benchmark_num_samples_parameter->value() && m_timestamp_parameter->value() + 1 <= m_timestamp_parameter->maximum())
	{
		m_timestamp_parameter->setValue(m_timestamp_parameter->value() + 1);
		// Currently, the animation starts directly after timestamp switch, which means that the used camera path has to contain a waiting second in the beginning
		startCameraPathNavigation();
		return;
	}

	switchToNextConfiguration();
}
}