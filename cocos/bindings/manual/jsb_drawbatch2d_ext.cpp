#include "cocos/bindings/manual/jsb_drawbatch2d_ext.h"

#include "cocos/bindings/auto/jsb_gfx_auto.h"
#include "cocos/bindings/auto/jsb_pipeline_auto.h"
#include "cocos/bindings/jswrapper/SeApi.h"
#include "cocos/bindings/manual/jsb_conversions.h"
#include "cocos/bindings/manual/jsb_global.h"
#include "gfx-base/GFXPipelineState.h"
#include "renderer/gfx-base/GFXDescriptorSet.h"
#include "renderer/gfx-base/GFXInputAssembler.h"
#include "renderer/pipeline/Define.h"
#include "renderer/pipeline/PipelineStateManager.h"
#include "renderer/pipeline/RenderPipeline.h"
#include "v8/ScriptEngine.h"

namespace {

void fastSetFlag(void *buffer) {
    struct Heap {
        cc::scene::DrawBatch2D *self;
        uint32_t                flags;
    };
    Heap *heap           = reinterpret_cast<Heap *>(buffer);
    heap->self->visFlags = heap->flags;
}

void fastSetDescriptorSet(void *buffer) {
    struct Heap {
        cc::scene::DrawBatch2D *self;
        cc::gfx::DescriptorSet *ds;
    };
    Heap *heap                = reinterpret_cast<Heap *>(buffer);
    heap->self->descriptorSet = heap->ds;
}

void fastSetInputAssembler(void *buffer) {
    struct Heap {
        cc::scene::DrawBatch2D * self;
        cc::gfx::InputAssembler *inputAssembler;
    };
    Heap *heap                 = reinterpret_cast<Heap *>(buffer);
    heap->self->inputAssembler = heap->inputAssembler;
}

void fastSetPasses(void *buffer) {
    struct Heap {
        cc::scene::DrawBatch2D *self;
        uint32_t                passSize;
        cc::scene::Pass* passes[0];
    };
    Heap *heap   = reinterpret_cast<Heap *>(buffer);
    auto &passes = heap->self->passes;
    passes.resize(heap->passSize);
    for (auto i = 0; i < heap->passSize; i++) {
        passes[i] = heap->passes[i];
    }
}

void fastSetShaders(void *buffer) {
    struct Heap {
        cc::scene::DrawBatch2D *self;
        uint32_t                shaderSize;
        cc::gfx::Shader* shaders[0];
    };
    Heap *heap    = reinterpret_cast<Heap *>(buffer);
    auto &shaders = heap->self->shaders;
    shaders.resize(heap->shaderSize);
    for (auto i = 0; i < heap->shaderSize; i++) {
        shaders[i] = heap->shaders[i];
    }
}

template <typename F>
uint32_t convertPtr(F *in) {
    return static_cast<uint32_t>(reinterpret_cast<intptr_t>(in));
}
uint8_t *mqPtr{nullptr};
constexpr size_t bufferSize = 10 * 1024 * 1024;
} // namespace

void flushFastMQ() {
    if (mqPtr == nullptr) return;
    uint32_t *u32Ptr   = reinterpret_cast<uint32_t *>(mqPtr);
    auto      offset   = *u32Ptr;
    auto      commands = *(u32Ptr + 1);
    assert(offset < bufferSize);
    if (commands == 0) return;

    typedef void (*FastFunction)(void *);

    uint8_t *p = mqPtr + 8;
    for (uint32_t i = 0; i < commands; i++) {
        uint32_t *    base = reinterpret_cast<uint32_t *>(p);
        uint32_t      len  = *base;
        FastFunction *fn   = reinterpret_cast<FastFunction *>(base + 1);
        (*fn)(p + 8);
        p += len;
    }
    // reset
    *u32Ptr       = 8;
    *(u32Ptr + 1) = 0;
}

bool register_all_drawbatch2d_ext_manual(se::Object *obj) {
    // allocate global message queue

    se::AutoHandleScope scope;
    auto             globalThis = se::ScriptEngine::getInstance()->getGlobalObject();
    se::Object *     msgQueue   = se::Object::createArrayBufferObject(nullptr, bufferSize);
    {
        uint8_t * ptr    = nullptr;
        uint32_t *intptr = nullptr;
        msgQueue->getArrayBufferData(&ptr, nullptr);
        intptr  = reinterpret_cast<uint32_t *>(ptr);
        *intptr = 8; // offset
        *(intptr + 1) = 0; // msg count
        mqPtr   = ptr;
    }
    globalThis->setProperty("fastMQ", se::Value(msgQueue));

    // register function table, serialize to queue
    {

        se::AutoHandleScope scope2;
        se::Value dbJSValue;
        se::Object* nsobj{ nullptr };

        se::Value nsValue;
        obj->getProperty("ns", &nsValue);
        nsobj = nsValue.toObject();

        nsobj->getProperty("DrawBatch2D", &dbJSValue);
        se::Object *dbJSObj = dbJSValue.toObject();
        se::Object *fnTable = se::Object::createPlainObject();
        fnTable->setProperty("visFlags", se::Value(convertPtr(fastSetFlag)));
        fnTable->setProperty("descriptorSet", se::Value(convertPtr(fastSetDescriptorSet)));
        fnTable->setProperty("inputAssembler", se::Value(convertPtr(fastSetInputAssembler)));
        fnTable->setProperty("passes", se::Value(convertPtr(fastSetPasses)));
        fnTable->setProperty("shaders", se::Value(convertPtr(fastSetShaders)));
        dbJSObj->setProperty("fnTable", se::Value(fnTable));
    }

    // define getter/setter
    {
        auto script = R"(
let _dataView_ = new DataView(fastMQ);
let _ft = ns.DrawBatch2D.fnTable;
Object.defineProperty(ns.DrawBatch2D.prototype, "visFlags", {
    set: function(v) { 
        let offset = _dataView_.getUint32(0, true);
        let commands = _dataView_.getUint32(4, true);
        let thisPtr = this.__native_ptr__; // BigInt

        let pos = 4; // reserved for fn length
        _dataView_.setUint32(offset + pos, _ft.visFlags, true); // fn
        pos += 4;
        _dataView_.setUint32(offset + pos, thisPtr, true); // this
        pos += 4;
        _dataView_.setUint32(offset + pos, v, true); // arg
        pos += 4;
        _dataView_.setUint32(offset + 0, pos, true);   // fn length
        _dataView_.setUint32(0, offset + pos, true); // update offset
        _dataView_.setUint32(4, commands + 1, true); // update cnt
    },             
    enumerable : true,
    configurable : true
});
Object.defineProperty(ns.DrawBatch2D.prototype, "descriptorSet", {
    set: function(v) {  
        if(!v) return;
        let offset = _dataView_.getUint32(0, true);
        let commands = _dataView_.getUint32(4, true);

        let thisPtr = this.__native_ptr__; // BigInt
        let pos = 4; // reserved for fn length
        _dataView_.setUint32(offset + pos, _ft.descriptorSet, true); // fn
        pos += 4;
        _dataView_.setUint32(offset + pos, thisPtr, true); // this
        pos += 4;
        _dataView_.setUint32(offset + pos, v.__native_ptr__, true); // arg
        pos += 4;
        _dataView_.setUint32(offset + 0, pos, true);   // fn length
        _dataView_.setUint32(0, offset + pos, true); // update offset
        _dataView_.setUint32(4, commands + 1, true); // update cnt
    },             
    enumerable : true,
    configurable : true
});

Object.defineProperty(ns.DrawBatch2D.prototype, "inputAssembler", {
    set: function(v) { 
        if(!v) return;
        let offset = _dataView_.getUint32(0, true);
        let commands = _dataView_.getUint32(4, true);

        let thisPtr = this.__native_ptr__; // BigInt
        let pos = 4; // reserved for fn length
        _dataView_.setUint32(offset + pos, _ft.inputAssembler, true); // fn
        pos += 4;
        _dataView_.setUint32(offset + pos, thisPtr, true); // this
        pos += 4;
        _dataView_.setUint32(offset + pos, v.__native_ptr__, true); // arg
        pos += 4;
        _dataView_.setUint32(offset + 0, pos, true);   // fn length
        _dataView_.setUint32(0, offset + pos, true); // update offset
        _dataView_.setUint32(4, commands + 1, true); // update cnt
    },             
    enumerable : true,
    configurable : true
});

Object.defineProperty(ns.DrawBatch2D.prototype, "passes", {
    set: function(passes) {   
        if(!passes) return;
        let offset = _dataView_.getUint32(0, true);
        let commands = _dataView_.getUint32(4, true);

        let thisPtr = this.__native_ptr__; // BigInt
        let pos = 4; // reserved for fn length
        _dataView_.setUint32(offset + pos, _ft.passes, true); // fn
        pos += 4;
        _dataView_.setUint32(offset + pos, thisPtr, true); // this
        pos += 4;
        
        _dataView_.setUint32(offset + pos, passes.length, true); // arg
        pos += 4;
        for(let p of passes) {
            _dataView_.setUint32(offset + pos, p.__native_ptr__, true); // arg
            pos += 4;
        }

        _dataView_.setUint32(offset + 0, pos, true);   // fn length
        _dataView_.setUint32(0, offset + pos, true); // update offset
        _dataView_.setUint32(4, commands + 1, true); // update cnt
    },             
    enumerable : true,
    configurable : true
});

Object.defineProperty(ns.DrawBatch2D.prototype, "shaders", {
    set: function(shaders) {  
        if(!shaders) return;
        let offset = _dataView_.getUint32(0, true);
        let commands = _dataView_.getUint32(4, true);

        let thisPtr = this.__native_ptr__; // BigInt
        let pos = 4; // reserved for fn length
        _dataView_.setUint32(offset + pos, _ft.shaders, true); // fn
        pos += 4;
        _dataView_.setUint32(offset + pos, thisPtr, true); // this
        pos += 4;
        
        _dataView_.setUint32(offset + pos, shaders.length, true); // arg
        pos += 4;
        for(let p of shaders) {
            _dataView_.setUint32(offset + pos, p.__native_ptr__, true); // arg
            pos += 4;
        }

        _dataView_.setUint32(offset + 0, pos, true);   // fn length
        _dataView_.setUint32(0, offset + pos, true); // update offset
        _dataView_.setUint32(4, commands + 1, true); // update cnt
    },             
    enumerable : true,
    configurable : true
});
        )";
        se::ScriptEngine::getInstance()->evalString(script);
    }
    return true;
}