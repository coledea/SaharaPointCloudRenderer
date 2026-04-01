#include "Framebuffer.h"

#include <iostream>

namespace sahara::rendering
{

const std::unordered_map<FramebufferAttachmentTypes, QOpenGLTexture::TextureFormat> ATTACHMENT_TYPE_TO_TEXTURE_FORMAT{
	{ FramebufferAttachmentTypes::COLOR, QOpenGLTexture::RGBA8_UNorm },
	{ FramebufferAttachmentTypes::DEPTH, QOpenGLTexture::D24 },
	{ FramebufferAttachmentTypes::NORMAL, QOpenGLTexture::RGB8_UNorm },
	{ FramebufferAttachmentTypes::POSITION, QOpenGLTexture::RGB32F },
	{ FramebufferAttachmentTypes::ID, QOpenGLTexture::R32U },
	{ FramebufferAttachmentTypes::SEGMENT, QOpenGLTexture::R32U },
	{ FramebufferAttachmentTypes::CUSTOM_FLOAT, QOpenGLTexture::R32F },
	{ FramebufferAttachmentTypes::CUSTOM_INT, QOpenGLTexture::R32I },
	{ FramebufferAttachmentTypes::CUSTOM_UINT, QOpenGLTexture::R32U },
	{ FramebufferAttachmentTypes::CUSTOM_VEC, QOpenGLTexture::RGB32F },
};

Framebuffer::Framebuffer(OpenGLContext* context, int width, int height)
	: m_context(context)
	, m_width(width)
	, m_height(height)
	, m_FBOs{ 0, 0 }
	, m_current_FBO(0)
	, m_required_attachments{ FramebufferAttachmentTypes::COLOR, FramebufferAttachmentTypes::DEPTH }
{
	resize(width, height);
}

int Framebuffer::width() const noexcept
{
	return m_width;
}

int Framebuffer::height() const noexcept
{
	return m_height;
}

int Framebuffer::currentFBOIndex() const noexcept
{
	return m_current_FBO;
}

void Framebuffer::resize(int width, int height)
{
	// in some cases, the platform-specific window resources are already destroyed when a resize event is fired
	if (!m_context->makeCurrent())
	{
		return;
	}

	m_width = width;
	m_height = height;

	destroyFBOs();
	createTextures();
	createFBOs();
	m_context->doneCurrent();
}

void Framebuffer::createTextures()
{
	for (auto attachment : m_required_attachments)
	{
		if (attachment == FramebufferAttachmentTypes::COLOR)
		{
			m_textures[0].emplace(FramebufferAttachmentTypes::COLOR, createTexture(QOpenGLTexture::RGBA8_UNorm));
			m_textures[1].emplace(FramebufferAttachmentTypes::COLOR, createTexture(QOpenGLTexture::RGBA8_UNorm));
		}
		else
		{
			m_textures[0].emplace(attachment, createTexture(ATTACHMENT_TYPE_TO_TEXTURE_FORMAT.at(attachment)));
			m_textures[1].insert({ attachment, m_textures[0][attachment] });
		}
	}
}

std::shared_ptr<QOpenGLTexture> Framebuffer::createTexture(QOpenGLTexture::TextureFormat format)
{
	auto texture = std::make_shared<QOpenGLTexture>(QOpenGLTexture::Target2D);
	texture->setSize(m_width, m_height);
	texture->setFormat(format);
	texture->setWrapMode(QOpenGLTexture::WrapMode::ClampToEdge);
	texture->allocateStorage();
	return texture;
}

void Framebuffer::createFBOs()
{
	for (int i = 0; i < 2; i++)
	{
		createFBO(i);
	}
	m_current_FBO = 0;
}

void Framebuffer::createFBO(int fbo_index)
{
	m_context->gl()->glGenFramebuffers(1, &m_FBOs[fbo_index]);
	bind(fbo_index);

	std::vector<GLenum> draw_buffers;
	for (auto attachment : m_required_attachments)
	{
		if (attachment == FramebufferAttachmentTypes::DEPTH)
		{
			m_context->gl()->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureID(FramebufferAttachmentTypes::DEPTH, fbo_index), 0);
		}
		else
		{
			// Note: the order of the output declarations in the shader must follow the order of FramebufferAttachmentTypes, as this is the order in which the draw_buffers are added
			auto attachment_specifier = GL_COLOR_ATTACHMENT0 + static_cast<int>(attachment);
			m_context->gl()->glFramebufferTexture2D(GL_FRAMEBUFFER, attachment_specifier, GL_TEXTURE_2D, textureID(attachment, fbo_index), 0);
			draw_buffers.push_back(attachment_specifier);
		}
	}
	m_context->gl()->glDrawBuffers(draw_buffers.size(), draw_buffers.data());
	release();
}

void Framebuffer::setRequiredAttachments(const std::set<FramebufferAttachmentTypes>& required_attachments)
{
	m_required_attachments.clear();
	m_required_attachments.insert(FramebufferAttachmentTypes::COLOR); // color and depth are always required
	m_required_attachments.insert(FramebufferAttachmentTypes::DEPTH);
	m_required_attachments.insert(required_attachments.begin(), required_attachments.end());

	resize(m_width, m_height); // we just discard the old framebuffer objects and create new ones for now
}

GLuint Framebuffer::textureID(FramebufferAttachmentTypes type, int fbo_index) const
{
	return m_textures[fbo_index].at(type)->textureId();
}

void Framebuffer::bind(int fbo_index)
{
	m_current_FBO = fbo_index;
	m_context->gl()->glBindFramebuffer(GL_FRAMEBUFFER, m_FBOs[fbo_index]);
}

void Framebuffer::bind() const
{
	m_context->gl()->glBindFramebuffer(GL_FRAMEBUFFER, m_FBOs[m_current_FBO]);
}

void Framebuffer::release() const
{
	m_context->gl()->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::destroy()
{
	m_context->makeCurrent();
	destroyFBOs();
	m_context->doneCurrent();
}

void Framebuffer::blitToDefaultFramebuffer()
{
	m_context->gl()->glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
	m_context->gl()->glBlitNamedFramebuffer(m_FBOs[m_current_FBO],
											m_context->defaultFramebufferID(),
											0,
											0,
											m_width,
											m_height,
											0,
											0,
											m_width,
											m_height,
											GL_COLOR_BUFFER_BIT,
											GL_NEAREST);
}

void Framebuffer::swapCurrentFBO()
{
	release();
	m_current_FBO = 1 - m_current_FBO;
	bind();
}

void Framebuffer::destroyFBOs()
{
	if (m_textures[0].empty())
	{
		return;
	}

	m_context->gl()->glDeleteFramebuffers(2, &m_FBOs[0]);
	for (auto& texture : m_textures[0])
	{
		texture.second->destroy();
	}

	// all textures are shared, except the color textures
	if (m_textures[1].find(FramebufferAttachmentTypes::COLOR) != m_textures[1].end())
	{
		m_textures[1][FramebufferAttachmentTypes::COLOR]->destroy();
	}
	m_textures[0].clear();
	m_textures[1].clear();
	m_current_FBO = 0;
}

}