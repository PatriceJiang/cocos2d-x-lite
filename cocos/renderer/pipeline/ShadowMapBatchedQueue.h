/****************************************************************************
 Copyright (c) 2020-2021 Xiamen Yaji Software Co., Ltd.

 http://www.cocos.com

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated engine source code (the "Software"), a limited,
 worldwide, royalty-free, non-assignable, revocable and non-exclusive license
 to use Cocos Creator solely to develop games on your target platforms. You shall
 not use Cocos Creator software for developing other software or tools that's
 used for developing games. You are not granted to publish, distribute,
 sublicense, and/or sell copies of Cocos Creator.

 The software or tools in this License Agreement are licensed, not sold.
 Xiamen Yaji Software Co., Ltd. reserves all rights not expressly granted to you.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
****************************************************************************/

#pragma once

#include "Define.h"

namespace cc {
namespace pipeline {
struct SubModelView;
struct PassView;
struct RenderObject;
class RenderInstancedQueue;
class RenderBatchedQueue;
class ForwardPipeline;
struct Shadows;
struct Light;
struct ModelView;

//const uint phaseID(PassPhase::getPhaseID("shadow-caster"));

class CC_DLL ShadowMapBatchedQueue : public Object {
public:
    explicit ShadowMapBatchedQueue(ForwardPipeline *);
    ~ShadowMapBatchedQueue() override = default;
    void destroy();

    void clear();
    void gatherLightPasses(const Light *, gfx::CommandBuffer *);
    void add(const ModelView *, gfx::CommandBuffer *);
    void recordCommandBuffer(gfx::Device *, gfx::RenderPass *, gfx::CommandBuffer *) const;

private:
    int getShadowPassIndex(const ModelView *model) const;

    ForwardPipeline *_pipeline = nullptr;
    vector<const SubModelView *> _subModels;
    vector<const PassView *> _passes;
    vector<gfx::Shader *> _shaders;
    RenderInstancedQueue *_instancedQueue = nullptr;
    RenderBatchedQueue *_batchedQueue = nullptr;
    gfx::Buffer *_buffer = nullptr;
    uint _phaseID = 0;
};

} // namespace pipeline
} // namespace cc
