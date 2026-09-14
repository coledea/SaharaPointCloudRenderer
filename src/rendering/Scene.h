#pragma once

#include "Renderer.h"
#include "navigation/NavigationHandler.h"
#include "rendering/pointcloudProviders/AbstractPointCloudProvider.h"

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QPoint>
#include <QString>
#include <QVector3D>
#include <optional>

namespace sahara::rendering
{

class Scene : public QObject
{
	Q_OBJECT

public:
	struct UserStudyAnnotation
	{
		uint pointcloud_id;
		QString type;
		QVector3D position;
	};

	Scene(OpenGLContext* opengl_context, Framebuffer* framebuffer, navigation::Camera* camera, navigation::NavigationHandler* navigation_handler);

	const AbstractPointCloudProvider& pointCloud(uint id);
	Renderer& renderer(uint pointcloud_id);
	const std::vector<UserStudyAnnotation>& userStudyAnnotations() const noexcept;
	std::vector<AbstractRasterizer::AnnotationViewport> annotationViewports(uint pointcloud_id) const;
	std::optional<AbstractRasterizer::AnnotationViewport> annotationViewportAt(uint pointcloud_id, const QPoint& point) const;

	void render();
	void addPointCloud(const QString& filepath);
	void removePointCloud(uint id);
	void reloadShaders();
	void releaseOpenGLResources();
	void addUserStudyAnnotation(uint pointcloud_id, const QString& type, const QVector3D& position);
	void removeUserStudyAnnotation(size_t index);
	void setHighlightedUserStudyAnnotation(std::optional<size_t> index);
	void setUserStudyAnnotationsVisible(bool visible);

	void setPointCloudVisible(uint id, bool visible);
	void focusOnPointCloud(uint id);

signals:
	void pointCloudAdded(const AbstractPointCloudProvider& pointcloud_provider);
	void pointCloudRemoved(uint id);
	void rendererModulesChanged(uint id);
	void userStudyAnnotationsChanged();

private:
	OpenGLContext* m_opengl_context;
	Framebuffer* m_framebuffer;
	std::unordered_map<uint, std::unique_ptr<AbstractPointCloudProvider>> m_pointclouds;
	std::unordered_map<uint, std::unique_ptr<Renderer>> m_renderers;
	navigation::Camera* m_camera;
	navigation::NavigationHandler* m_navigation_handler;
	std::unordered_map<uint, bool> m_visibility_flags;

	std::vector<UserStudyAnnotation> m_user_study_annotations;
	QOpenGLShaderProgram m_annotation_shader_program;
	QOpenGLBuffer m_annotation_vbo;
	QOpenGLVertexArrayObject m_annotation_vao;
	bool m_annotation_renderer_initialized = false;
	bool m_user_study_annotations_visible = true;
	std::optional<size_t> m_highlighted_user_study_annotation;

	void renderUserStudyAnnotations();
	void initializeAnnotationRenderer();
	void updateAnnotationBuffer();
};

}
