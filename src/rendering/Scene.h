#pragma once

#include "Renderer.h"
#include "navigation/NavigationHandler.h"
#include "rendering/pointcloudProviders/AbstractPointCloudProvider.h"

namespace sahara::rendering
{

class Scene : public QObject
{
	Q_OBJECT

public:
	Scene(OpenGLContext* opengl_context, Framebuffer* framebuffer, navigation::Camera* camera, navigation::NavigationHandler* navigation_handler);

	const AbstractPointCloudProvider& pointCloud(uint id);
	Renderer& renderer(uint pointcloud_id);

	void render();
	void addPointCloud(const QString& filepath);
	void removePointCloud(uint id);
	void reloadShaders();
	void releaseOpenGLResources();

	void setPointCloudVisible(uint id, bool visible);
	void focusOnPointCloud(uint id);

signals:
	void pointCloudAdded(const AbstractPointCloudProvider& pointcloud_provider);
	void pointCloudRemoved(uint id);

private:
	OpenGLContext* m_opengl_context;
	Framebuffer* m_framebuffer;
	std::unordered_map<uint, std::unique_ptr<AbstractPointCloudProvider>> m_pointclouds;
	std::unordered_map<uint, std::unique_ptr<Renderer>> m_renderers;
	navigation::Camera* m_camera;
	navigation::NavigationHandler* m_navigation_handler;
	std::unordered_map<uint, bool> m_visibility_flags;
};

}
