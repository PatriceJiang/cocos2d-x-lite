#pragma once

NS_CC_BEGIN

class CCMTLFrameBuffer : public GFXFramebuffer
{
public:
    CCMTLFrameBuffer(GFXDevice* device);
    ~CCMTLFrameBuffer();
    
    virtual bool Initialize(const GFXFramebufferInfo& info) override;
    virtual void Destroy() override;
};

NS_CC_END