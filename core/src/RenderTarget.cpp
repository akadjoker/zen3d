#include "RenderTarget.hpp"
 #include "Texture.hpp"
#include <SDL2/SDL.h>

bool RenderTarget::create(int w, int h)
{
    if (fbo_) destroy();
    width_  = w;
    height_ = h;
    glGenFramebuffers(1, &fbo_);
    return fbo_ != 0;
}

bool RenderTarget::addColorAttachment()
{
    if (!fbo_ || colorTex_) return false;

    colorTex_ = Texture2D::CreateColor(width_, height_);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, colorTex_->GetID(), 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

bool RenderTarget::addDepthAttachment()
{
    if (!fbo_ || depthRbo_ || depthTex_) return false;

    glGenRenderbuffers(1, &depthRbo_);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRbo_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width_, height_);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, depthRbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

bool RenderTarget::addDepthTexture()
{
    if (!fbo_ || depthTex_ || depthRbo_) return false;

    depthTex_ = Texture2D::CreateDepth(width_, height_);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D, depthTex_->GetID(), 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

 

bool RenderTarget::finalize()
{
    if (!fbo_) return false;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        SDL_Log("[RenderTarget] FBO incomplete: 0x%x", status);
        destroy();
        return false;
    }
    return true;
}

void RenderTarget::destroy()
{
    if (fbo_)      { glDeleteFramebuffers(1,  &fbo_);      fbo_      = 0; }
    if (depthRbo_) { glDeleteRenderbuffers(1, &depthRbo_); depthRbo_ = 0; }
    delete colorTex_; colorTex_ = nullptr;
    delete depthTex_; depthTex_ = nullptr;
    width_ = height_ = 0;
}

void RenderTarget::bind()   const { glBindFramebuffer(GL_FRAMEBUFFER, fbo_); }
void RenderTarget::unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0);   }
