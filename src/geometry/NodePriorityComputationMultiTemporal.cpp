#include "NodePriorityComputationMultiTemporal.h"

#include "utils/Profiler.h"
#include "utils/ShaderUtilities.h"

#include <QtMath>

namespace sahara::geometry
{

NodePriorityComputationMultiTemporal::NodePriorityComputationMultiTemporal(rendering::OpenGLContext* context, const std::vector<NodeGPUData>& node_data, const std::vector<std::vector<uint32_t>>& level_starts)
	: AbstractNodePriorityComputation(context, node_data.size(), "./data/shaders/processing/ComputePrioritiesMT.comp")
	, m_number_of_timestamps(level_starts.size())
	, m_selected_timestamp(0)
	, m_max_levels(0)
	, m_timestamp_priority_falloff(0.0f)
	, m_timestamp_distance_to_consider(0)
{
	m_opengl_context->makeCurrent();

	// construct priority computation dispatch information from the provided level start indices for each timestamp
	for (int timestamp = 0; timestamp < level_starts.size(); timestamp++)
	{
		m_level_bounds.push_back({});
		auto& timestamp_level_starts = level_starts[timestamp];
		for (int i = 0; i < timestamp_level_starts.size() - 1; i++)
		{
			auto workgroup_size = static_cast<GLuint>(std::ceil(static_cast<float>(timestamp_level_starts[i + 1] - timestamp_level_starts[i]) / 1024.0f));
			m_level_bounds.back().emplace_back(timestamp_level_starts[i], timestamp_level_starts[i + 1], workgroup_size);
		}
		auto level_end = (timestamp < level_starts.size() - 1) ? level_starts[timestamp + 1].front() : node_data.size();
		m_level_bounds.back().emplace_back(timestamp_level_starts.back(), level_end, static_cast<GLuint>(std::ceil(static_cast<float>(level_end - timestamp_level_starts.back()) / 1024.0f)));
	}

	m_max_levels = std::max_element(m_level_bounds.begin(), m_level_bounds.end(), [](const auto& a, const auto& b) { return a.size() < b.size(); })->size();

	m_opengl_context->gl()->glGenBuffers(1, &m_bbox_buffer);
	m_opengl_context->gl()->glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bbox_buffer);
	m_opengl_context->gl()->glBufferData(GL_SHADER_STORAGE_BUFFER, node_data.size() * sizeof(NodeGPUData), node_data.data(), GL_STATIC_DRAW);

	m_opengl_context->doneCurrent();

	m_temporal_priorities.resize(m_number_of_timestamps, 0.0f);
}

NodePriorityComputationMultiTemporal::~NodePriorityComputationMultiTemporal()
{
}

void NodePriorityComputationMultiTemporal::computePriorities(const navigation::Camera& camera)
{
	m_shader_program.bind();

	auto mvp = camera.viewProjectionMatrix();
	m_shader_program.setUniformValue("u_mvp", mvp);
	m_shader_program.setUniformValue("u_camera_position", camera.cameraSpecifications().eye);
	m_shader_program.setUniformValue("u_screen_width", camera.viewportWidth());
	m_shader_program.setUniformValue("u_screen_height", camera.viewportHeight());

	std::array<QVector4D, 6> view_frustum = {
		mvp.row(3) + mvp.row(0),
		mvp.row(3) - mvp.row(0),
		mvp.row(3) + mvp.row(1),
		mvp.row(3) - mvp.row(1),
		mvp.row(3) + mvp.row(2),
		mvp.row(3) - mvp.row(2)
	};
	m_shader_program.setUniformValueArray("u_camera_frustum", view_frustum.data(), 6);

	m_opengl_context->gl()->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_bbox_buffer);
	m_opengl_context->gl()->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_result_buffer.handle());

	// loop over all levels of all timestamps. We proceed level by level.
	for (uint level = 0; level < m_max_levels; level++)
	{
		for (int timestamp = 0; timestamp < m_number_of_timestamps; timestamp++)
		{
			// skip timestamps not within the range of interest
			int timestamp_distance = std::abs(timestamp - static_cast<int>(m_selected_timestamp));
			if (timestamp_distance > m_timestamp_distance_to_consider)
			{
				continue;
			}

			// skip levels that do not exist for this timestamp
			if (!(m_level_bounds[timestamp].size() > level))
			{
				continue;
			}

			m_opengl_context->gl()->glUniform1i(m_shader_program.uniformLocation("u_is_current_timestamp"), timestamp == m_selected_timestamp);
			m_opengl_context->gl()->glUniform1f(m_shader_program.uniformLocation("u_timestamp_factor"), m_temporal_priorities[timestamp_distance]);
			m_opengl_context->gl()->glUniform1ui(m_shader_program.uniformLocation("u_level_start"), m_level_bounds[timestamp][level].start); // no support for uint in QOpenGLShaderProgram::setUniformValue
			m_opengl_context->gl()->glUniform1ui(m_shader_program.uniformLocation("u_level_end"), m_level_bounds[timestamp][level].end);
			m_opengl_context->gl()->glDispatchCompute(m_level_bounds[timestamp][level].workgroup_size, 1, 1);
		}
		m_opengl_context->gl()->glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	m_shader_program.release();

	m_result_buffer.waitForGPUWrite();
}

void NodePriorityComputationMultiTemporal::setTimestamp(uint timestamp)
{
	m_selected_timestamp = timestamp;
}

void NodePriorityComputationMultiTemporal::setTimestampDistanceToConsider(uint distance)
{
	m_timestamp_distance_to_consider = distance;
	computeTemporalPriorities();
}

void NodePriorityComputationMultiTemporal::setTimestampPriorityFalloff(float falloff)
{
	m_timestamp_priority_falloff = falloff;
	computeTemporalPriorities();
}

// uses a superellipsis-based function to compute how to scale the priority for each timestamp (from only the current timestamp is important to all timestamps are equally important)
void NodePriorityComputationMultiTemporal::computeTemporalPriorities()
{
	const float PRIORITY_THRESHOLD = 0.001f;

	// we precompute the temporal priority factors for all possible timestamp distances (instead of per timestamp, since then we would have to recompute on timestamp changes)
	for (int distance = 0; distance < m_number_of_timestamps; distance++)
	{
		if (distance > m_timestamp_distance_to_consider)
		{
			m_temporal_priorities[distance] = 0.0f;
			continue;
		}

		if (m_timestamp_distance_to_consider == 0)
		{
			m_temporal_priorities[distance] = (distance == 0) ? 1.0f : 0.0f;
			continue;
		}

		float normalized_distance = static_cast<float>(distance) / static_cast<float>(m_timestamp_distance_to_consider);

		// this is the superellipsis parameter n with 0 < n < 1 leading to hyperbolic arcs, n = 1 leading to linear falloff, 1 < n < 2 leading to elliptic arcs, and n > 2 nearing a rectangular shape with rounded corners
		// we map the user parameter 0 to n = 0.1, 0.5 to n = 1.0, and 1.0 to n = 10.0
		float n = std::pow(10.0, 1.0 - 2.0 * (1.0 - m_timestamp_priority_falloff));

		float superellipse = std::pow(1.0f - std::pow(normalized_distance, n), 1.0 / n);
		float timestamp_factor = PRIORITY_THRESHOLD + (1.0f - PRIORITY_THRESHOLD) * superellipse;

		m_temporal_priorities[distance] = timestamp_factor; // this timestamp factor is multiplied with the (spatial) priority and thus influences loading/caching of nodes from different timestamps
	}
}

}
