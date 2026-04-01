#include "Profiler.h"

#include <QTextStream>
#include <filesystem>

namespace sahara::utils
{
Profiler global_profiler;

Profiler::Profiler()
	: m_is_profiling(false)
{
}

Profiler::~Profiler()
{
#ifdef PROFILER_ENABLED
	flushAll();
#endif
}

void Profiler::startSession(const std::string& output_folder, size_t capacity)
{
#ifdef PROFILER_ENABLED
	if (m_is_profiling)
	{
		return; // if the profiler is already running, don't start a new session
	}
	m_session_start_time = std::chrono::steady_clock::now();
	m_output_folder = output_folder;
	std::filesystem::create_directories(m_output_folder);
	m_capacity = capacity;
	m_is_profiling = true;
#endif
}

void Profiler::endSession()
{
#ifdef PROFILER_ENABLED
	flushAll();
	m_is_profiling = false;
#endif
}

void Profiler::addMeasurement(const std::string& name, int value)
{
#ifdef PROFILER_ENABLED
	if (m_is_profiling)
	{
		createMeasurementTypeIfNotExists(name);
		m_measurements[name].values.push_back(value);
		m_measurements[name].timestamps.push_back(std::chrono::duration_cast<std::chrono::microseconds>(
													  std::chrono::steady_clock::now() - m_session_start_time)
													  .count());
		flushIfCapacityIsExceeded(name);
	}
#endif
}

void Profiler::startTimer(const std::string& name)
{
#ifdef PROFILER_ENABLED
	if (m_is_profiling)
	{
		m_started_timers[name] = std::chrono::steady_clock::now();
	}
#endif
}

void Profiler::stopTimer(const std::string& name)
{
#ifdef PROFILER_ENABLED
	if (m_is_profiling)
	{
		assert(m_started_timers.find(name) != m_started_timers.end() && "No timer with this name.");
		auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_session_start_time).count();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_started_timers[name]).count();
		m_started_timers.erase(name);

		createMeasurementTypeIfNotExists(name);
		m_measurements[name].values.push_back(static_cast<int>(duration));
		m_measurements[name].timestamps.push_back(timestamp);
		flushIfCapacityIsExceeded(name);
	}
#endif
}

bool Profiler::isRunning() const noexcept
{
	return m_is_profiling;
}

void Profiler::createMeasurementTypeIfNotExists(const std::string& name)
{
#ifdef PROFILER_ENABLED
	if (m_measurements.find(name) == m_measurements.end())
	{
		auto output_file_path = m_output_folder + "/" + name + ".txt";
		m_measurements.emplace(name, output_file_path);
	}
#endif
}

void Profiler::flushIfCapacityIsExceeded(const std::string& name)
{
#ifdef PROFILER_ENABLED
	if (m_measurements[name].values.size() > m_capacity)
	{
		flush(name);
	}
#endif
}

void Profiler::flush(const std::string& name)
{
#ifdef PROFILER_ENABLED
	QTextStream text_stream(&m_measurements[name].output_file);
	for (int i = 0; i < m_measurements[name].values.size(); i++)
	{
		text_stream << m_measurements[name].timestamps[i] << ";" << m_measurements[name].values[i] << "\n";
	}
	m_measurements[name].output_file.flush();
	m_measurements[name].values.clear();
	m_measurements[name].timestamps.clear();
#endif
}

void Profiler::flushAll()
{
#ifdef PROFILER_ENABLED
	for (const auto& entry : m_measurements)
	{
		flush(entry.first);
		m_measurements[entry.first].output_file.close();
	}
	m_measurements.clear();
#endif
}

}