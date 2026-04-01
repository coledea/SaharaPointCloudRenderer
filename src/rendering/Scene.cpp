#include "Scene.h"

#include "rendering/pointcloudProviders/PointCloudProviderFactory.h"

namespace sahara::rendering
{

Scene::Scene(OpenGLContext* opengl_context, Framebuffer* framebuffer, navigation::Camera* camera, navigation::NavigationHandler* navigation_handler)
	: m_opengl_context(opengl_context)
	, m_framebuffer(framebuffer)
	, m_camera(camera)
	, m_navigation_handler(navigation_handler)
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

void Scene::render()
{
	for (auto& renderer : m_renderers)
	{
		if (m_visibility_flags[renderer.first] == true)
		{
			renderer.second->render();
		}
	}
}

void Scene::addPointCloud(const QString& filepath)
{
	const auto path = std::filesystem::path(filepath.toStdString());
	auto pointcloud = PointCloudProviderFactory::createPointCloudProvider(path, m_opengl_context, m_camera, m_navigation_handler);

	if (pointcloud == nullptr)
	{
		qWarning() << "Could not create provider for point cloud " << path.c_str();
		return;
	}

	const uint id = pointcloud->id();
	m_pointclouds[id] = std::move(pointcloud);
	m_renderers.emplace(id, std::make_unique<Renderer>(m_pointclouds[id].get(), m_opengl_context, m_framebuffer, m_camera));
	m_visibility_flags[id] = true;

	focusOnPointCloud(id);

	emit pointCloudAdded(*m_pointclouds[id]);
}

void Scene::removePointCloud(uint id)
{
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
	m_renderers.clear();
	m_pointclouds.clear();
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

}
