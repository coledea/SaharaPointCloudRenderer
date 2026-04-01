#pragma once

#include <QFile>
#include <chrono>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace sahara::utils
{

struct Measurements
{
	std::vector<int> values;
	std::vector<ulong> timestamps;
	QFile output_file;

	Measurements() = default;

	Measurements(const std::string& output_file_path)
		: output_file(std::filesystem::path(output_file_path))
	{
		if (output_file.exists())
		{
			output_file.remove();
		}
		output_file.open(QIODevice::WriteOnly | QIODevice::Text);
	}
};

// Helper class to record measurements and write them to disk
class Profiler
{
public:
	Profiler();
	~Profiler();

	void startSession(const std::string& output_folder, size_t capacity);
	void endSession();

	void addMeasurement(const std::string& name, int value);
	void startTimer(const std::string& name);
	void stopTimer(const std::string& name);

	bool isRunning() const noexcept;

private:
	std::string m_output_folder;
	std::unordered_map<std::string, Measurements> m_measurements;
	std::unordered_map<std::string, std::chrono::time_point<std::chrono::steady_clock>> m_started_timers;
	size_t m_capacity;
	std::chrono::time_point<std::chrono::steady_clock> m_session_start_time;
	bool m_is_profiling;

	void createMeasurementTypeIfNotExists(const std::string& name);
	void flushIfCapacityIsExceeded(const std::string& name);
	void flush(const std::string& name);
	void flushAll();
};

extern Profiler global_profiler;
}
