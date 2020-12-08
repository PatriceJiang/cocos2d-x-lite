#include "cocos/bindings/auto/jsb_mtl_auto.h"
#include "cocos/bindings/manual/jsb_conversions.h"
#include "cocos/bindings/manual/jsb_global.h"
#include "renderer/gfx-metal/GFXMTL.h"

#ifndef JSB_ALLOC
#define JSB_ALLOC(kls, ...) new (std::nothrow) kls(__VA_ARGS__)
#endif

#ifndef JSB_FREE
#define JSB_FREE(ptr) delete ptr
#endif
se::Object* __jsb_cc_gfx_CCMTLDevice_proto = nullptr;
se::Class* __jsb_cc_gfx_CCMTLDevice_class = nullptr;

static bool js_mtl_CCMTLDevice_getMaximumBufferBindingIndex(se::State& s)
{
    cc::gfx::CCMTLDevice* cobj = SE_THIS_OBJECT<cc::gfx::CCMTLDevice>(s);
    SE_PRECONDITION2(cobj, false, "js_mtl_CCMTLDevice_getMaximumBufferBindingIndex : Invalid Native Object");
    const auto& args = s.args();
    size_t argc = args.size();
    CC_UNUSED bool ok = true;
    if (argc == 0) {
        unsigned int result = cobj->getMaximumBufferBindingIndex();
        ok &= nativevalue_to_se(result, s.rval(), nullptr /*ctx*/);
        SE_PRECONDITION2(ok, false, "js_mtl_CCMTLDevice_getMaximumBufferBindingIndex : Error processing arguments");
        return true;
    }
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", (int)argc, 0);
    return false;
}
SE_BIND_FUNC(js_mtl_CCMTLDevice_getMaximumBufferBindingIndex)

static bool js_mtl_CCMTLDevice_getMTLDevice(se::State& s)
{
    cc::gfx::CCMTLDevice* cobj = SE_THIS_OBJECT<cc::gfx::CCMTLDevice>(s);
    SE_PRECONDITION2(cobj, false, "js_mtl_CCMTLDevice_getMTLDevice : Invalid Native Object");
    const auto& args = s.args();
    size_t argc = args.size();
    CC_UNUSED bool ok = true;
    if (argc == 0) {
        void* result = cobj->getMTLDevice();
        ok &= nativevalue_to_se(result, s.rval(), nullptr /*ctx*/);
        SE_PRECONDITION2(ok, false, "js_mtl_CCMTLDevice_getMTLDevice : Error processing arguments");
        return true;
    }
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", (int)argc, 0);
    return false;
}
SE_BIND_FUNC(js_mtl_CCMTLDevice_getMTLDevice)

static bool js_mtl_CCMTLDevice_getMTKView(se::State& s)
{
    cc::gfx::CCMTLDevice* cobj = SE_THIS_OBJECT<cc::gfx::CCMTLDevice>(s);
    SE_PRECONDITION2(cobj, false, "js_mtl_CCMTLDevice_getMTKView : Invalid Native Object");
    const auto& args = s.args();
    size_t argc = args.size();
    CC_UNUSED bool ok = true;
    if (argc == 0) {
        void* result = cobj->getMTKView();
        ok &= nativevalue_to_se(result, s.rval(), nullptr /*ctx*/);
        SE_PRECONDITION2(ok, false, "js_mtl_CCMTLDevice_getMTKView : Error processing arguments");
        return true;
    }
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", (int)argc, 0);
    return false;
}
SE_BIND_FUNC(js_mtl_CCMTLDevice_getMTKView)

static bool js_mtl_CCMTLDevice_getMaximumSamplerUnits(se::State& s)
{
    cc::gfx::CCMTLDevice* cobj = SE_THIS_OBJECT<cc::gfx::CCMTLDevice>(s);
    SE_PRECONDITION2(cobj, false, "js_mtl_CCMTLDevice_getMaximumSamplerUnits : Invalid Native Object");
    const auto& args = s.args();
    size_t argc = args.size();
    CC_UNUSED bool ok = true;
    if (argc == 0) {
        unsigned int result = cobj->getMaximumSamplerUnits();
        ok &= nativevalue_to_se(result, s.rval(), nullptr /*ctx*/);
        SE_PRECONDITION2(ok, false, "js_mtl_CCMTLDevice_getMaximumSamplerUnits : Error processing arguments");
        return true;
    }
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", (int)argc, 0);
    return false;
}
SE_BIND_FUNC(js_mtl_CCMTLDevice_getMaximumSamplerUnits)

static bool js_mtl_CCMTLDevice_getMaximumColorRenderTargets(se::State& s)
{
    cc::gfx::CCMTLDevice* cobj = SE_THIS_OBJECT<cc::gfx::CCMTLDevice>(s);
    SE_PRECONDITION2(cobj, false, "js_mtl_CCMTLDevice_getMaximumColorRenderTargets : Invalid Native Object");
    const auto& args = s.args();
    size_t argc = args.size();
    CC_UNUSED bool ok = true;
    if (argc == 0) {
        unsigned int result = cobj->getMaximumColorRenderTargets();
        ok &= nativevalue_to_se(result, s.rval(), nullptr /*ctx*/);
        SE_PRECONDITION2(ok, false, "js_mtl_CCMTLDevice_getMaximumColorRenderTargets : Error processing arguments");
        return true;
    }
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", (int)argc, 0);
    return false;
}
SE_BIND_FUNC(js_mtl_CCMTLDevice_getMaximumColorRenderTargets)

static bool js_mtl_CCMTLDevice_isIndirectCommandBufferSupported(se::State& s)
{
    cc::gfx::CCMTLDevice* cobj = SE_THIS_OBJECT<cc::gfx::CCMTLDevice>(s);
    SE_PRECONDITION2(cobj, false, "js_mtl_CCMTLDevice_isIndirectCommandBufferSupported : Invalid Native Object");
    const auto& args = s.args();
    size_t argc = args.size();
    CC_UNUSED bool ok = true;
    if (argc == 0) {
        bool result = cobj->isIndirectCommandBufferSupported();
        ok &= nativevalue_to_se(result, s.rval(), nullptr /*ctx*/);
        SE_PRECONDITION2(ok, false, "js_mtl_CCMTLDevice_isIndirectCommandBufferSupported : Error processing arguments");
        return true;
    }
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", (int)argc, 0);
    return false;
}
SE_BIND_FUNC(js_mtl_CCMTLDevice_isIndirectCommandBufferSupported)

static bool js_mtl_CCMTLDevice_isSamplerDescriptorCompareFunctionSupported(se::State& s)
{
    cc::gfx::CCMTLDevice* cobj = SE_THIS_OBJECT<cc::gfx::CCMTLDevice>(s);
    SE_PRECONDITION2(cobj, false, "js_mtl_CCMTLDevice_isSamplerDescriptorCompareFunctionSupported : Invalid Native Object");
    const auto& args = s.args();
    size_t argc = args.size();
    CC_UNUSED bool ok = true;
    if (argc == 0) {
        bool result = cobj->isSamplerDescriptorCompareFunctionSupported();
        ok &= nativevalue_to_se(result, s.rval(), nullptr /*ctx*/);
        SE_PRECONDITION2(ok, false, "js_mtl_CCMTLDevice_isSamplerDescriptorCompareFunctionSupported : Error processing arguments");
        return true;
    }
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", (int)argc, 0);
    return false;
}
SE_BIND_FUNC(js_mtl_CCMTLDevice_isSamplerDescriptorCompareFunctionSupported)

static bool js_mtl_CCMTLDevice_isIndirectDrawSupported(se::State& s)
{
    cc::gfx::CCMTLDevice* cobj = SE_THIS_OBJECT<cc::gfx::CCMTLDevice>(s);
    SE_PRECONDITION2(cobj, false, "js_mtl_CCMTLDevice_isIndirectDrawSupported : Invalid Native Object");
    const auto& args = s.args();
    size_t argc = args.size();
    CC_UNUSED bool ok = true;
    if (argc == 0) {
        bool result = cobj->isIndirectDrawSupported();
        ok &= nativevalue_to_se(result, s.rval(), nullptr /*ctx*/);
        SE_PRECONDITION2(ok, false, "js_mtl_CCMTLDevice_isIndirectDrawSupported : Error processing arguments");
        return true;
    }
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", (int)argc, 0);
    return false;
}
SE_BIND_FUNC(js_mtl_CCMTLDevice_isIndirectDrawSupported)

static bool js_mtl_CCMTLDevice_getMTLCommandQueue(se::State& s)
{
    cc::gfx::CCMTLDevice* cobj = SE_THIS_OBJECT<cc::gfx::CCMTLDevice>(s);
    SE_PRECONDITION2(cobj, false, "js_mtl_CCMTLDevice_getMTLCommandQueue : Invalid Native Object");
    const auto& args = s.args();
    size_t argc = args.size();
    CC_UNUSED bool ok = true;
    if (argc == 0) {
        void* result = cobj->getMTLCommandQueue();
        ok &= nativevalue_to_se(result, s.rval(), nullptr /*ctx*/);
        SE_PRECONDITION2(ok, false, "js_mtl_CCMTLDevice_getMTLCommandQueue : Error processing arguments");
        return true;
    }
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", (int)argc, 0);
    return false;
}
SE_BIND_FUNC(js_mtl_CCMTLDevice_getMTLCommandQueue)

SE_DECLARE_FINALIZE_FUNC(js_cc_gfx_CCMTLDevice_finalize)

static bool js_mtl_CCMTLDevice_constructor(se::State& s) // constructor.c
{
    cc::gfx::CCMTLDevice* cobj = JSB_ALLOC(cc::gfx::CCMTLDevice);
    s.thisObject()->setPrivateData(cobj);
    se::NonRefNativePtrCreatedByCtorMap::emplace(cobj);
    return true;
}
SE_BIND_CTOR(js_mtl_CCMTLDevice_constructor, __jsb_cc_gfx_CCMTLDevice_class, js_cc_gfx_CCMTLDevice_finalize)



extern se::Object* __jsb_cc_gfx_Device_proto;

static bool js_cc_gfx_CCMTLDevice_finalize(se::State& s)
{
    auto iter = se::NonRefNativePtrCreatedByCtorMap::find(SE_THIS_OBJECT<cc::gfx::CCMTLDevice>(s));
    if (iter != se::NonRefNativePtrCreatedByCtorMap::end())
    {
        se::NonRefNativePtrCreatedByCtorMap::erase(iter);
        cc::gfx::CCMTLDevice* cobj = SE_THIS_OBJECT<cc::gfx::CCMTLDevice>(s);
        JSB_FREE(cobj);
    }
    return true;
}
SE_BIND_FINALIZE_FUNC(js_cc_gfx_CCMTLDevice_finalize)

bool js_register_mtl_CCMTLDevice(se::Object* obj)
{
    auto cls = se::Class::create("CCMTLDevice", obj, __jsb_cc_gfx_Device_proto, _SE(js_mtl_CCMTLDevice_constructor));

    cls->defineFunction("getMaximumBufferBindingIndex", _SE(js_mtl_CCMTLDevice_getMaximumBufferBindingIndex));
    cls->defineFunction("getMTLDevice", _SE(js_mtl_CCMTLDevice_getMTLDevice));
    cls->defineFunction("getMTKView", _SE(js_mtl_CCMTLDevice_getMTKView));
    cls->defineFunction("getMaximumSamplerUnits", _SE(js_mtl_CCMTLDevice_getMaximumSamplerUnits));
    cls->defineFunction("getMaximumColorRenderTargets", _SE(js_mtl_CCMTLDevice_getMaximumColorRenderTargets));
    cls->defineFunction("isIndirectCommandBufferSupported", _SE(js_mtl_CCMTLDevice_isIndirectCommandBufferSupported));
    cls->defineFunction("isSamplerDescriptorCompareFunctionSupported", _SE(js_mtl_CCMTLDevice_isSamplerDescriptorCompareFunctionSupported));
    cls->defineFunction("isIndirectDrawSupported", _SE(js_mtl_CCMTLDevice_isIndirectDrawSupported));
    cls->defineFunction("getMTLCommandQueue", _SE(js_mtl_CCMTLDevice_getMTLCommandQueue));
    cls->defineFinalizeFunction(_SE(js_cc_gfx_CCMTLDevice_finalize));
    cls->install();
    JSBClassType::registerClass<cc::gfx::CCMTLDevice>(cls);

    __jsb_cc_gfx_CCMTLDevice_proto = cls->getProto();
    __jsb_cc_gfx_CCMTLDevice_class = cls;

    se::ScriptEngine::getInstance()->clearException();
    return true;
}

bool register_all_mtl(se::Object* obj)
{
    // Get the ns
    se::Value nsVal;
    if (!obj->getProperty("gfx", &nsVal))
    {
        se::HandleObject jsobj(se::Object::createPlainObject());
        nsVal.setObject(jsobj);
        obj->setProperty("gfx", nsVal);
    }
    se::Object* ns = nsVal.toObject();

    js_register_mtl_CCMTLDevice(ns);
    return true;
}

