#include "RenderWindow.h"

#include "utils/Profiler.h"

namespace sahara::ui
{

RenderWindow::RenderWindow(QWindow* parent, navigation::NavigationHandler* navigation_handler)
	: QWindow(parent)
	, m_navigation_handler(navigation_handler)
	, m_scene(nullptr)		 // to avoid circular initialization issues, we pass the scene in later when startRendering is called
	, m_context(nullptr)	 // to avoid circular initialization issues, we pass the context in later when startRendering is called
	, m_framebuffer(nullptr) // to avoid circular initialization issues, we pass the context in later when startRendering is called
	, m_use_vsync(true)
	, m_is_shutting_down(false)
{
	setSurfaceType(OpenGLSurface);
}

RenderWindow::~RenderWindow()
{
	// We need a destructor here, as Ui::RenderWindow was forward-declared
}

void RenderWindow::startRendering(rendering::OpenGLContext* context, std::unique_ptr<rendering::Framebuffer> framebuffer, rendering::Scene* scene, bool use_vsync)
{
	m_is_shutting_down = false;
	m_use_vsync = use_vsync;
	m_context = context;
	m_scene = scene;
	m_framebuffer = std::move(framebuffer);

	if (m_use_vsync)
	{
		requestUpdate();
	}
	else
	{
		m_render_timer.start(0, this); // currently, we are either using vsync, or rendering with full speed.
	}
}

void RenderWindow::shutdownRendering()
{
	if (m_is_shutting_down)
	{
		return;
	}

	m_is_shutting_down = true;
	m_render_timer.stop();

	if (m_context != nullptr && m_context->makeCurrent())
	{
		if (m_scene != nullptr)
		{
			m_scene->releaseOpenGLResources();
			m_scene = nullptr;
		}

		if (m_framebuffer != nullptr)
		{
			m_framebuffer->destroy();
			m_framebuffer.reset();
		}

		m_context->doneCurrent();
	}
}

int RenderWindow::deviceScaledWidth() const
{
	// Note: width() returns the width of the window without the frame and is, therefore, not equal with the size of the default backbuffer.
	return static_cast<int>(std::ceil(static_cast<qreal>(frameGeometry().width()) * devicePixelRatio()));
}

int RenderWindow::deviceScaledHeight() const
{
	// Note: height() returns the height of the window without the frame and is, therefore, not equal with the size of the default backbuffer.
	return static_cast<int>(std::ceil(static_cast<qreal>(frameGeometry().height()) * devicePixelRatio()));
}

void RenderWindow::render()
{
	if (m_is_shutting_down || m_context == nullptr || m_scene == nullptr || m_framebuffer == nullptr)
	{
		return;
	}

	if (!isExposed())
	{
		requestUpdate();
		return;
	}

	utils::global_profiler.startTimer("Whole_frame");

	m_context->makeCurrent();

	m_framebuffer->bind();
	glViewport(0, 0, deviceScaledWidth(), deviceScaledHeight());
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_scene->render();

	m_framebuffer->blitToDefaultFramebuffer();
	m_framebuffer->release();

	m_context->swapBuffers();
	m_context->doneCurrent();

	utils::global_profiler.stopTimer("Whole_frame");

	if (m_use_vsync)
	{
		requestUpdate();
	}
}

QPoint RenderWindow::scaledMousePosition(const QPoint& pos) const
{
	auto pos_scaled = static_cast<QPointF>(pos) * devicePixelRatio();
	return QPoint(static_cast<int>(std::ceil(pos_scaled.x())), static_cast<int>(std::ceil(pos_scaled.y())));
}

void RenderWindow::mousePressEvent(QMouseEvent* e)
{
	m_navigation_handler->mousePressEvent(scaledMousePosition(e->pos()), e->button());
}

void RenderWindow::mouseReleaseEvent(QMouseEvent* e)
{
	m_navigation_handler->mouseReleaseEvent(scaledMousePosition(e->pos()), e->button());
}

void RenderWindow::mouseMoveEvent(QMouseEvent* e)
{
	m_navigation_handler->mouseMoveEvent(scaledMousePosition(e->pos()));
}

void RenderWindow::wheelEvent(QWheelEvent* e)
{
	m_navigation_handler->wheelEvent(*e);
}

void RenderWindow::resizeEvent(QResizeEvent* e)
{
	if (m_framebuffer)
	{
		m_framebuffer->resize(deviceScaledWidth(), deviceScaledHeight());
	}
	emit resized();
}

bool RenderWindow::event(QEvent* e)
{
	if (e->type() == QEvent::UpdateRequest)
	{
		if (m_is_shutting_down)
		{
			return true;
		}
		render();
		return true;
	}
	// We need to release the OpenGL resources before the platform window is destroyed for which we created the OpenGL context.
	// For the resources managed by Qt, this will happen automatically, but in the case of the framebuffer, we manage the resources ourselves.
	else if (e->type() == QEvent::PlatformSurface)
	{
		QPlatformSurfaceEvent* platform_surface_event = static_cast<QPlatformSurfaceEvent*>(e);
		if (platform_surface_event->surfaceEventType() == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed)
		{
			if (!m_is_shutting_down)
			{
				shutdownRendering();
			}
			else
			{
				m_framebuffer = nullptr;
			}
		}
	}
	else if (e->type() == QEvent::Drop)
	{
		const auto event = dynamic_cast<QDropEvent*>(e);
		emit receivedDrop(event);
	}

	return QWindow::event(e);
}

void RenderWindow::timerEvent(QTimerEvent* e)
{
	if (e->timerId() != m_render_timer.timerId())
	{
		return;
	}
	render();
}

}
