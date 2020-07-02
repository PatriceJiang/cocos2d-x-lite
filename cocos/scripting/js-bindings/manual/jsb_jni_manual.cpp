

#include "jsb_global.h"
#include "jsb_conversions.hpp"
#include <jni.h>
#include "platform/android/jni/JniHelper.h"
#include "platform/android/jni/JniImp.h"
#include "jsb_jni_utils.h"
#define JS_JNI_DEBUG 1
#define JS_JNI_TAG_TYPE "jni_obj_type"
#define JS_JNI_JCLASS_TYPE "java_class"

using cocos2d::JniHelper;
using cocos2d::JniMethodInfo;

namespace {
    se::Class *__jsb_jni_jobject = nullptr;
}


namespace {

    std::unordered_map<std::string, se::Class *> sJClassToJSClass;

    std::string getJObjectClass(jobject obj) {
        auto *env = JniHelper::getEnv();
        jclass objectClass = env->GetObjectClass(obj);
        jmethodID getClassMethodID = env->GetMethodID(objectClass, "getClass", "()Ljava/lang/Class;");

        jobject classObject = env->CallObjectMethod(obj, getClassMethodID);
        jclass classObjectClass = env->GetObjectClass(classObject);
        jmethodID getSimpleNameMethodID = env->GetMethodID(classObjectClass, "getSimpleName", "()Ljava/lang/String;");

        jstring classNameString = (jstring) env->CallObjectMethod(classObject, getSimpleNameMethodID);

        char buff[256] = {};
        int len = env->GetStringUTFLength(classNameString);
        env->GetStringUTFRegion(classNameString, 0, len, buff);

        env->DeleteLocalRef(objectClass);
        env->DeleteLocalRef(classObject);
        env->DeleteLocalRef(classObjectClass);

        return buff;
    }

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
            _jsObj->setProperty(JS_JNI_JCLASS_TYPE, se::Value(getJObjectClass(_obj)));
#endif
            return _jsObj;
        }

        jobject unwrap() const  {
            return _obj;
        }

        jclass getClass() const {
            return JniHelper::getEnv()->GetObjectClass(_obj);
        }

    private:
        jobject _obj = nullptr;
        se::Object *_jsObj = nullptr;
    };

    jvalue seval_to_jvalule(JNIEnv *env, const jni_utils::JniType &def, const se::Value &val, bool &ok)
    {
        jvalue ret;
        ok = false;
        if(def.isBoolean()) {
            ret.z = val.toBoolean();
        }else if(def.isByte()) {
            ret.b = val.toInt8();
        }else if(def.isChar()) {
            ret.c = val.toInt16();
        }else if(def.isShort()) {
            ret.s = val.toInt16();
        }else if(def.isInt()) {
            ret.i = val.toInt32();
        }else if(def.isLong()) {
            ret.j = static_cast<jlong>(val.toNumber()); // int 64
        }else if (def.isFloat()) {
            ret.f = val.toFloat();
        }else if(def.isDouble()) {
            ret.d = val.toNumber();
        }else if(def.isObject()) {
            assert(val.isObject());
            se::Object *seObj = val.toObject();
            auto *jo = static_cast<JObjectWrapper*>(seObj->getPrivateData());
            ret.l = jo->unwrap();
        }else{
            SE_LOGE("incorrect jni type, don't know how to convert from js value");
            return ret;
        }
        ok = true;
        return ret;
    }


    std::vector<jvalue> convertFuncArgs(JNIEnv *env, const std::string &signature, const std::vector<se::Value> &args, int offset, bool &success)
    {
        bool convertOk = false;
        success = false;
        std::vector<jni_utils::JniType> argTypes = jni_utils::exactArgsFromSignature(signature, convertOk);
        if(convertOk) {
            SE_LOGE("failed to parse signature \"%s\"", signature.c_str());
            return {};
        }

        if(argTypes.size() != args.size() - offset) {
            SE_LOGE("arguments size %d does not match function signature \"%s\"", args.size(), signature.c_str());
            return {};
        }

        std::vector<jvalue> ret;
        for(size_t i =0; i < argTypes.size(); i++) {
            jvalue arg = seval_to_jvalule(env, argTypes[i], args[i + offset], convertOk);
            assert(convertOk);
            ret.push_back(arg);
        }

        success = true;
        return ret;
    }

    void installJavaClass(const std::string &javaClass, se::Class *seClass) {
        sJClassToJSClass[javaClass] = seClass;
    }

    se::Class *findJSClass(const std::string &javaClass) {
        auto itr = sJClassToJSClass.find(javaClass);
        if(itr == sJClassToJSClass.end()) {
            return nullptr;
        }
        return itr->second;
    }
}

static bool js_jni_jobject_finalize(se::State& s)
{
    auto* cobj = (JObjectWrapper*)s.nativeThisObject();
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
    auto *env = JniHelper::getEnv();
    jobject activity = JniHelper::getActivity();
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
    bool ok;
    jclass klass;

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
            klass = methodInfo.classID;
        }else{
            JniHelper::reportError(klassPath, "<init>", signature);
            return false;
        }
    } else {
        // arg0 : class
        // arg1 : signature
        // arg2 : arg 0 for constructor
        // arg3 : arg 1 for constructor
        // ...
        auto klassPath = s.args()[0].toString();
        signature = s.args()[1].toString();
        std::vector<jvalue> jvalueArray = convertFuncArgs(JniHelper::getEnv(), signature, s.args(), 2, ok);

        if( JniHelper::getMethodInfo(methodInfo, klassPath.c_str(), "<init>", signature.c_str())) {
            ret = methodInfo.env->NewObjectA(methodInfo.classID, methodInfo.methodID, jvalueArray.data());
            klass = methodInfo.classID;
        }else{
            JniHelper::reportError(klassPath, "<init>", signature);
            return false;
        }
    }
    auto *jobjWrapper = new JObjectWrapper(ret);
    s.rval().setObject(jobjWrapper->asJSObject());
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

