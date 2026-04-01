#include "rendering/Framebuffer.h"
#include "rendering/Scene.h"
#include "ui/MainWindow.h"

#include <QApplication>
#include <QSurfaceFormat>

#if defined _WIN32 && !defined QT_CREATOR
#	include <windows.h>
#endif

int main(int argc, char* argv[])
{

	// To see the console output under Windows in some IDEs, we have to open a separate console
#if defined _WIN32 && !defined QT_CREATOR
	AllocConsole();
	FILE* stream = nullptr;
	freopen_s(&stream, "CONOUT$", "w", stdout);
	freopen_s(&stream, "CONOUT$", "w", stderr);
	atexit([]() { system("pause"); }); // we want the console to stay open even if the application crashes
#endif

	QSurfaceFormat format;
	format.setRenderableType(QSurfaceFormat::OpenGL);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setVersion(4, 5);
	QSurfaceFormat::setDefaultFormat(format);

	QApplication app(argc, argv);

	sahara::navigation::Camera camera;
	sahara::navigation::NavigationHandler navigation_handler(&camera);
	auto render_window = std::make_unique<sahara::ui::RenderWindow>(nullptr, &navigation_handler);
	render_window->create(); // this actually creates the platform-specific resources

	// We have to pass this to any class that deals with OpenGL (e.g., for initializing buffers in point cloud construction or rendering into the render window)
	sahara::rendering::OpenGLContext opengl_context(render_window.get());
	auto framebuffer = std::make_unique<sahara::rendering::Framebuffer>(&opengl_context, render_window->deviceScaledWidth(), render_window->deviceScaledHeight());
	sahara::rendering::Scene scene(&opengl_context, framebuffer.get(), &camera, &navigation_handler);

	sahara::ui::MainWindow mainWindow(nullptr, render_window.get(), &navigation_handler, &camera, &scene); // Note: ownership of render_window is transferred to mainWindow
	mainWindow.show();
	render_window->startRendering(&opengl_context, std::move(framebuffer), &scene, true);
	render_window->show();

	return app.exec();
}
