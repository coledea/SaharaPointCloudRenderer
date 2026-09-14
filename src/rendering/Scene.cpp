#include "Scene.h"

#include "rendering/pointcloudProviders/PointCloudProviderFactory.h"

#include <algorithm>

namespace sahara::rendering
{

Scene::Scene(OpenGLContext* opengl_context, Framebuffer* framebuffer, navigation::Camera* camera, navigation::NavigationHandler* navigation_handler)
	: m_opengl_context(opengl_context)
	, m_framebuffer(framebuffer)
	, m_camera(camera)
	, m_navigation_handler(navigation_handler)
	, m_annotation_vbo(QOpenGLBuffer::VertexBuffer)
{
}

const AbstractPointCloudProvider& Scene::pointCloud(uint id)
{
	assert(m_pointclouds.find(id) != m_pointclouds.end());
	return *m_pointclouds[id];
}

Renderer& Scene::renderer(uint pointcloud_id)
{
	assert(m_renderers.find(pointcloud_id) != m_renderers.end());
	return *m_renderers[pointcloud_id];
}

const std::vector<Scene::UserStudyAnnotation>& Scene::userStudyAnnotations() const noexcept
{
	return m_user_study_annotations;
}

std::vector<AbstractRasterizer::AnnotationViewport> Scene::annotationViewports(uint pointcloud_id) const
{
	const auto renderer = m_renderers.find(pointcloud_id);
	if (renderer == m_renderers.end())
	{
		return {};
	}
	return renderer->second->annotationViewports();
}

std::optional<AbstractRasterizer::AnnotationViewport> Scene::annotationViewportAt(uint pointcloud_id, const QPoint& point) const
{
	for (const auto& annotation_viewport : annotationViewports(pointcloud_id))
	{
		if (annotation_viewport.viewport.contains(point))
		{
			return annotation_viewport;
		}
	}
	return std::nullopt;
}

void Scene::render()
{
	for (auto& renderer : m_renderers)
	{
		if (m_visibility_flags[renderer.first] == true)
		{
			renderer.second->render();
		}
	}
	renderUserStudyAnnotations();
}

void Scene::addPointCloud(const QString& filepath)
{
	const auto path = std::filesystem::path(filepath.toStdString());
	auto pointcloud = PointCloudProviderFactory::createPointCloudProvider(path, m_opengl_context, m_camera);

	if (pointcloud == nullptr)
	{
		qWarning() << "Could not create provider for point cloud " << path.c_str();
		return;
	}

	const uint id = pointcloud->id();
	m_pointclouds[id] = std::move(pointcloud);
	m_renderers.emplace(id, std::make_unique<Renderer>(m_pointclouds[id].get(), m_opengl_context, m_framebuffer, m_camera));
	connect(m_renderers[id].get(), &Renderer::modulesChanged, this, [this, id]() { emit rendererModulesChanged(id); });
	m_visibility_flags[id] = true;

	focusOnPointCloud(id);

	emit pointCloudAdded(*m_pointclouds[id]);
}

void Scene::removePointCloud(uint id)
{
	const auto removed_annotations = std::erase_if(m_user_study_annotations, [id](const auto& annotation) { return annotation.pointcloud_id == id; });
	if (removed_annotations > 0 && m_annotation_renderer_initialized)
	{
		m_opengl_context->makeCurrent();
		updateAnnotationBuffer();
		m_opengl_context->doneCurrent();
	}
	if (removed_annotations > 0)
	{
		m_highlighted_user_study_annotation.reset();
		emit userStudyAnnotationsChanged();
	}

	if (m_pointclouds.find(id) != m_pointclouds.end())
	{
		m_pointclouds.erase(id);
	}

	if (m_visibility_flags.find(id) != m_visibility_flags.end())
	{
		m_visibility_flags.erase(id);
	}

	if (m_renderers.find(id) != m_renderers.end())
	{
		m_renderers.erase(id);
	}
	emit pointCloudRemoved(id);
}

void Scene::reloadShaders()
{
	for (auto& renderer : m_renderers)
	{
		renderer.second->reloadShader();
	}
}

void Scene::releaseOpenGLResources()
{
	if (m_annotation_renderer_initialized)
	{
		m_annotation_vao.destroy();
		m_annotation_vbo.destroy();
		m_annotation_shader_program.removeAllShaders();
		m_annotation_renderer_initialized = false;
	}
	m_renderers.clear();
	m_pointclouds.clear();
}

void Scene::addUserStudyAnnotation(uint pointcloud_id, const QString& type, const QVector3D& position)
{
	m_user_study_annotations.push_back({ pointcloud_id, type, position });
	if (m_annotation_renderer_initialized)
	{
		m_opengl_context->makeCurrent();
	}
	updateAnnotationBuffer();
	if (m_annotation_renderer_initialized)
	{
		m_opengl_context->doneCurrent();
	}
	emit userStudyAnnotationsChanged();
}

void Scene::removeUserStudyAnnotation(size_t index)
{
	if (index >= m_user_study_annotations.size())
	{
		return;
	}

	m_user_study_annotations.erase(m_user_study_annotations.begin() + static_cast<std::ptrdiff_t>(index));
	if (m_highlighted_user_study_annotation && *m_highlighted_user_study_annotation == index)
	{
		m_highlighted_user_study_annotation.reset();
	}
	else if (m_highlighted_user_study_annotation && *m_highlighted_user_study_annotation > index)
	{
		m_highlighted_user_study_annotation = *m_highlighted_user_study_annotation - 1;
	}

	if (m_annotation_renderer_initialized)
	{
		m_opengl_context->makeCurrent();
		updateAnnotationBuffer();
		m_opengl_context->doneCurrent();
	}
	emit userStudyAnnotationsChanged();
}

void Scene::setHighlightedUserStudyAnnotation(std::optional<size_t> index)
{
	if (index && *index >= m_user_study_annotations.size())
	{
		m_highlighted_user_study_annotation.reset();
		return;
	}

	m_highlighted_user_study_annotation = index;
}

void Scene::setUserStudyAnnotationsVisible(bool visible)
{
	m_user_study_annotations_visible = visible;
}

void Scene::setPointCloudVisible(uint id, bool visible)
{
	assert(m_visibility_flags.find(id) != m_visibility_flags.end());
	m_visibility_flags[id] = visible;
}

void Scene::focusOnPointCloud(uint id)
{
	m_camera->setOptimalViewForBounds(m_pointclouds[id]->boundingBox());
	m_navigation_handler->setMoveSpeed(m_pointclouds[id]->boundingBox().extent().length() * 5.0f);
}

void Scene::renderUserStudyAnnotations()
{
	if (!m_user_study_annotations_visible || m_user_study_annotations.empty())
	{
		return;
	}

	initializeAnnotationRenderer();

	m_annotation_shader_program.bind();
	m_annotation_vao.bind();

	m_opengl_context->gl()->glEnable(GL_PROGRAM_POINT_SIZE);
	m_opengl_context->gl()->glDisable(GL_SCISSOR_TEST);
	m_opengl_context->gl()->glDisable(GL_DEPTH_TEST);

	for (size_t annotation_index = 0; annotation_index < m_user_study_annotations.size(); ++annotation_index)
	{
		const auto draw_index = static_cast<GLint>(annotation_index);
		for (const auto& annotation_viewport : annotationViewports(m_user_study_annotations[annotation_index].pointcloud_id))
		{
			m_opengl_context->gl()->glViewport(annotation_viewport.viewport.x(), annotation_viewport.viewport.y(), annotation_viewport.viewport.width(), annotation_viewport.viewport.height());
			m_annotation_shader_program.setUniformValue("u_mvp", annotation_viewport.view_projection_matrix);

			m_annotation_shader_program.setUniformValue("u_point_size", 24.0f);
			m_annotation_shader_program.setUniformValue("u_color", QVector3D(0.0f, 0.0f, 0.0f));
			m_opengl_context->gl()->glDrawArrays(GL_POINTS, draw_index, 1);

			m_annotation_shader_program.setUniformValue("u_point_size", 14.0f);
			m_annotation_shader_program.setUniformValue("u_color", QVector3D(1.0f, 0.85f, 0.05f));
			m_opengl_context->gl()->glDrawArrays(GL_POINTS, draw_index, 1);
		}
	}

	if (m_highlighted_user_study_annotation && *m_highlighted_user_study_annotation < m_user_study_annotations.size())
	{
		for (const auto& annotation_viewport : annotationViewports(m_user_study_annotations[*m_highlighted_user_study_annotation].pointcloud_id))
		{
			m_opengl_context->gl()->glViewport(annotation_viewport.viewport.x(), annotation_viewport.viewport.y(), annotation_viewport.viewport.width(), annotation_viewport.viewport.height());
			m_annotation_shader_program.setUniformValue("u_mvp", annotation_viewport.view_projection_matrix);
			const auto highlighted_index = static_cast<GLint>(*m_highlighted_user_study_annotation);
			m_annotation_shader_program.setUniformValue("u_point_size", 34.0f);
			m_annotation_shader_program.setUniformValue("u_color", QVector3D(0.0f, 0.0f, 0.0f));
			m_opengl_context->gl()->glDrawArrays(GL_POINTS, highlighted_index, 1);
			m_annotation_shader_program.setUniformValue("u_point_size", 24.0f);
			m_annotation_shader_program.setUniformValue("u_color", QVector3D(0.0f, 0.85f, 1.0f));
			m_opengl_context->gl()->glDrawArrays(GL_POINTS, highlighted_index, 1);
			m_annotation_shader_program.setUniformValue("u_point_size", 14.0f);
			m_annotation_shader_program.setUniformValue("u_color", QVector3D(1.0f, 0.85f, 0.05f));
			m_opengl_context->gl()->glDrawArrays(GL_POINTS, highlighted_index, 1);
		}
	}

	m_opengl_context->gl()->glViewport(0, 0, m_framebuffer->width(), m_framebuffer->height());
	m_opengl_context->gl()->glEnable(GL_DEPTH_TEST);
	m_annotation_vao.release();
	m_annotation_shader_program.release();
}

void Scene::initializeAnnotationRenderer()
{
	if (m_annotation_renderer_initialized)
	{
		return;
	}

	m_annotation_shader_program.addShaderFromSourceCode(QOpenGLShader::Vertex,
														"#version 450 core\n"
														"layout(location = 0) in vec3 a_position;\n"
														"uniform mat4 u_mvp;\n"
														"uniform float u_point_size;\n"
														"void main()\n"
														"{\n"
														"    gl_Position = u_mvp * vec4(a_position, 1.0);\n"
														"    gl_PointSize = u_point_size;\n"
														"}\n");
	m_annotation_shader_program.addShaderFromSourceCode(QOpenGLShader::Fragment,
														"#version 450 core\n"
														"uniform vec3 u_color;\n"
														"out vec4 fragColor;\n"
														"void main()\n"
														"{\n"
														"    vec2 p = gl_PointCoord * 2.0 - 1.0;\n"
														"    if (dot(p, p) > 1.0) discard;\n"
														"    fragColor = vec4(u_color, 1.0);\n"
														"}\n");
	m_annotation_shader_program.link();

	m_annotation_vao.create();
	m_annotation_vbo.create();

	m_annotation_vao.bind();
	m_annotation_vbo.bind();
	m_annotation_shader_program.enableAttributeArray(0);
	m_annotation_shader_program.setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(QVector3D));
	m_annotation_vbo.release();
	m_annotation_vao.release();

	m_annotation_renderer_initialized = true;
	updateAnnotationBuffer();
}

void Scene::updateAnnotationBuffer()
{
	if (!m_annotation_renderer_initialized)
	{
		return;
	}

	m_annotation_vbo.bind();
	std::vector<QVector3D> positions;
	positions.reserve(m_user_study_annotations.size());
	for (const auto& annotation : m_user_study_annotations)
	{
		positions.push_back(annotation.position);
	}
	m_annotation_vbo.allocate(positions.data(), static_cast<int>(positions.size() * sizeof(QVector3D)));
	m_annotation_vbo.release();
}

}
