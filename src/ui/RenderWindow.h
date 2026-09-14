#pragma once

#include "navigation/NavigationHandler.h"
#include "rendering/Framebuffer.h"
#include "rendering/InputEventCache.h"
#include "rendering/OpenGLContext.h"
#include "rendering/Scene.h"

#include <QBasicTimer>
#include <QWindow>

namespace sahara::ui
{

class RenderWindow : public QWindow
{
	Q_OBJECT

public:
	RenderWindow(QWindow* parent, navigation::NavigationHandler* navigation_handler);
	~RenderWindow();

	void startRendering(rendering::OpenGLContext* context, std::unique_ptr<rendering::Framebuffer> framebuffer, rendering::Scene* scene, bool use_vsync);
	void shutdownRendering();

	int deviceScaledWidth() const;
	int deviceScaledHeight() const;

	bool isPaused() const noexcept;
	void togglePause();

	std::optional<std::vector<float>> depthBuffer() const;
	void renderOnce();

	static InputEventCache s_event_cache;

signals:
	void receivedDrop(QDropEvent* e);
	void resized();
	void pauseStateChanged(bool paused);

private:
	QPointF scaledMousePosition(const QPointF& pos) const;
	void mousePressEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void wheelEvent(QWheelEvent* e) override;

	void keyPressEvent(QKeyEvent* e) override;
	void keyReleaseEvent(QKeyEvent* e) override;

	void resizeEvent(QResizeEvent* e) override;
	bool event(QEvent* e) override;
	void timerEvent(QTimerEvent* e) override;

	void render();

	navigation::NavigationHandler* m_navigation_handler;
	rendering::Scene* m_scene;
	rendering::OpenGLContext* m_context;

	std::unique_ptr<rendering::Framebuffer> m_framebuffer;
	QBasicTimer m_render_timer;
	bool m_use_vsync;
	bool m_is_shutting_down;
	bool m_paused;
};

}
