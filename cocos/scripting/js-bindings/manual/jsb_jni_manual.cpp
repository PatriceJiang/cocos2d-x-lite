

#include "jsb_global.h"
#include "jsb_conversions.hpp"
#include <jni.h>
#include "platform/android/jni/JniHelper.h"
#include "platform/android/jni/JniImp.h"

#define JS_JNI_DEBUG 1
#define JS_JNI_TAG_TYPE "jni_obj_type"

using cocos2d::JniHelper;
using cocos2d::JniMethodInfo;

namespace {
    se::Class *__jsb_jni_jobject = nullptr;
}


namespace {
    class JObjectWrapper {
    public:
        JObjectWrapper(jobject obj){
            if(obj) {
                _obj = JniHelper::getEnv()->NewGlobalRef(obj);
            }
        }
        virtual ~JObjectWrapper() {
            if(_obj) {
                JniHelper::getEnv()->DeleteGlobalRef(_obj);
                _obj = nullptr;
            }
        }

        se::Object * asJSObject() {
            if(!_obj) return nullptr;
            if(_jsObj) return _jsObj;
            _jsObj = se::Object::createObjectWithClass(__jsb_jni_jobject);
            _jsObj->setPrivateData(this);
#if JS_JNI_DEBUG
            _jsObj->setProperty(JS_JNI_TAG_TYPE, se::Value("jobject"));
#endif
            return _jsObj;
        }

        jobject unwrap() const  {
            return _obj;
        }

    private:
        jobject _obj = nullptr;
        se::Object *_jsObj = nullptr;
    };

    enum class JniType{
        None,
        Boolean_Z,
        Char_C,
        Short_S,
        Int_I,
        Long_J,
        Float_F,
        Double_D,
        Array_,
        Object_L,
    };

    JniType parseSigType(const char *data, int *len) {
        int i = 0;
        const char f = data[i];
        *len = 1;
        switch(f) {
            case 'Z': return JniType::Boolean_Z;
            case 'C': return JniType ::Char_C;
            case 'S': return JniType ::Short_S;
            case 'I': return JniType ::Int_I;
            case 'J': return JniType ::Long_J;
            case 'F' : return JniType ::Float_F;
            case 'D' : return JniType ::Double_D;
            default: ;
        }

        if(f == '[') {

        }else if(f == 'L') {

        }
    }

    std::vector<jobject*> convertFuncArgs(const std::string &signature, const std::vector<se::Value> &args, int offset, bool *success)
    {
        int i = 0;
        const int e = signature.length();
        if(signature[i] != '(') {
            *success = false;
            return {};
        }
    }

}

static bool js_jni_jobject_finalize(se::State& s)
{
    JObjectWrapper* cobj = (JObjectWrapper*)s.nativeThisObject();
    if(cobj){
        delete cobj;
    }
    return true;
}
SE_BIND_FINALIZE_FUNC(js_jni_jobject_finalize)


static bool js_register_jni_jobject(se::Object *obj) {
    auto cls = se::Class::create("jobject", obj, nullptr, nullptr);
    cls->defineFinalizeFunction(_SE(js_jni_jobject_finalize));
    __jsb_jni_jobject = cls;
    return true;
}

static bool js_jni_helper_getActivity(se::State& s)
{
    auto *obj = new JObjectWrapper(JniHelper::getActivity());
    s.rval().setObject(obj->asJSObject());
    return true;
}
SE_BIND_FUNC(js_jni_helper_getActivity)

static bool js_jni_helper_setClassLoaderFrom(se::State& s)
{
    const int argCnt = s.args().size();
    if(argCnt == 1) {
        SE_PRECONDITION2(s.args()[0].isObject(), false, "jobject expected!");
        auto *obj = s.args()[0].toObject();
        auto *jobj = (JObjectWrapper*) obj->getPrivateData();
        JniHelper::setClassLoaderFrom(jobj->unwrap());
        return true;
    }
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", (int)argCnt, 1);
    return false;
}
SE_BIND_FUNC(js_jni_helper_setClassLoaderFrom)

static bool js_jni_helper_newObject(se::State& s)
{
    const int argCnt = s.args().size();
    std::string signature;
    jobject ret = nullptr;

    JniMethodInfo methodInfo;
    if(argCnt == 0) {
        SE_REPORT_ERROR("wrong number of arguments: %d", (int)argCnt);
        return false;
    } else if(argCnt == 1) {
        // no arguments
        auto klassPath = s.args()[0].toString();
        signature = "()V";
        if( JniHelper::getMethodInfo( methodInfo, klassPath.c_str(), "<init>", signature.c_str())) {
            ret = methodInfo.env->NewObject(methodInfo.classID, methodInfo.methodID);
        }else{
            JniHelper::reportError(klassPath, "<init>", signature);
        }
    }
    return true;
}
SE_BIND_FUNC(js_jni_helper_newObject)

static bool js_register_jni_helper(se::Object *obj) {
    se::Value helperVal;
    if(!obj->getProperty("helper", &helperVal)){
        se::HandleObject jsobj(se::Object::createPlainObject());
        helperVal.setObject(jsobj);
        obj->setProperty("helper", helperVal);
    }
    auto *helperObj = helperVal.toObject();
    helperObj->defineFunction("getActivity", _SE(js_jni_helper_getActivity));
    helperObj->defineFunction("setClassLoaderFrom", _SE(js_jni_helper_setClassLoaderFrom));
    helperObj->defineFunction("newObject", _SE(js_jni_helper_newObject));
    return true;
}

bool jsb_register_jni_manual(se::Object* obj)
{
    se::Value nsVal;
    if (!obj->getProperty("jni", &nsVal))
    {
        se::HandleObject jsobj(se::Object::createPlainObject());
        nsVal.setObject(jsobj);
        obj->setProperty("jni", nsVal);
    }
    auto* ns = nsVal.toObject();
    js_register_jni_jobject(ns);
    js_register_jni_helper(ns);

    return true;
}

