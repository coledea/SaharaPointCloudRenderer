#pragma once

#include "OpenGLContext.h"
#include "utils/ShaderUtilities.h"

#include <QOpenGLFramebufferObject>
#include <QOpenGLTexture>
#include <set>

namespace sahara::rendering
{

enum class FramebufferAttachmentTypes
{
	COLOR,
	DEPTH,
	NORMAL,
	POSITION,
	SEGMENT,
	ID,
	CUSTOM_FLOAT,
	CUSTOM_INT,
	CUSTOM_UINT,
	CUSTOM_VEC
};

// This class maintains two FBOs to allow for ping-pong rendering during postprocessing.
// The FBOs share all textures except the color texture.
class Framebuffer
{
public:
	Framebuffer(OpenGLContext* context, int width = 1, int height = 1);

	int width() const noexcept;
	int height() const noexcept;
	int currentFBOIndex() const noexcept; // which of the two FBOs is currently active
	GLuint textureID(FramebufferAttachmentTypes type, int fbo_index) const;

	void resize(int width, int height);
	void setRequiredAttachments(const std::set<FramebufferAttachmentTypes>& required_attachments);

	void bind() const;
	void bindAsRead() const;
	void release() const;
	void destroy();
	void blitToDefaultFramebuffer(); // blits the current FBO to the default backbuffer
	void swapCurrentFBO();			 // implicitly releases the current FBO and binds the second one

private:
	OpenGLContext* m_context;

	int m_width, m_height;
	std::set<FramebufferAttachmentTypes> m_required_attachments;

	std::array<GLuint, 2> m_FBOs;
	std::array<std::unordered_map<FramebufferAttachmentTypes, std::shared_ptr<QOpenGLTexture>>, 2> m_textures;
	int m_current_FBO;

	void createTextures();
	std::shared_ptr<QOpenGLTexture> createTexture(QOpenGLTexture::TextureFormat format);

	void createFBOs();
	void createFBO(int fbo_index);

	void destroyFBOs();
	void bind(int fbox_index);
};

}
