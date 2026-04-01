#pragma once

#include "navigation/CameraAnimationPath.h"
#include "rendering/parameters/RangeParameter.h"

#include <QBasicTimer>
#include <QRandomGenerator>

namespace sahara::rendering
{
class OOCMultiTemporalPointCloudProvider;
}

namespace sahara::utils
{

// this class manages benchmarking for the OOCMultiTemporalPointCloudProvider
class MultiTemporalBenchmarkController : public QObject
{
	Q_OBJECT

public:
	enum class BenchmarkMode
	{
		Temporal,
		Spatial
	};

	MultiTemporalBenchmarkController(rendering::OOCMultiTemporalPointCloudProvider* provider);
	~MultiTemporalBenchmarkController();

	void setBenchmarkMode(BenchmarkMode benchmark_mode) noexcept;
	void setCameraPositionsPath(QString camera_positions_path);
	void startBenchmark();
	void stopBenchmark();

private:
	static const std::vector<int> m_benchmark_timestamp_ranges;
	static const std::vector<float> m_benchmark_timestamp_factors;
	static const int m_random_seed;

	rendering::OOCMultiTemporalPointCloudProvider* m_pointcloud_provider;
	rendering::RangeParameter<int>* m_timestamp_parameter;

	BenchmarkMode m_benchmark_mode;
	std::unique_ptr<navigation::CameraAnimationPath> m_benchmark_camera_path;
	bool m_benchmark_active;

	QBasicTimer m_timestamp_switch_timer;
	QRandomGenerator m_random_generator;

	// for tracking benchmark state
	std::vector<int> m_benchmark_configurations;
	int m_current_configuration_index;
	int m_number_of_timestamp_switches;
	bool m_next_switch_is_spatial;
	int m_next_camera_position;

	int timestampRangeForConfiguration(int configuration);
	float timestampFactorForConfiguration(int configuration);
	int currentConfiguration() const;

	void initializeBenchmarkConfigurations();
	void resetBenchmarkState();

	void startBenchmarkForCurrentConfiguration();
	void startTimestampSwitches();
	void startCameraPathNavigation();
	void switchCameraPosition();
	void switchTimestamp();
	void switchToNextConfiguration();

	void onCameraAnimationFinished();
	void timerEvent(QTimerEvent* e) override;
};
}