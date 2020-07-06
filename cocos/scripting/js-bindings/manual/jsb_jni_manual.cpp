

#include "jsb_global.h"
#include "jsb_conversions.hpp"
#include <jni.h>
#include <vector>
#include <string>
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

    se::Object *sProxyObject = nullptr;
    se::Object *sJavaObjectMethodsUnbound = nullptr;
    se::Object *sJavaObjectFieldsUnbound = nullptr;
}


namespace {

    class JValueWrapper;

    std::unordered_map<std::string, se::Class *> sJClassToJSClass;

    std::string jstringToString(jobject arg) {
        auto *env = JniHelper::getEnv();
        jstring str = (jstring) arg;
        std::vector<char> buff;
        int len = env->GetStringUTFLength(str);
        buff.resize(len + 1);
        env->GetStringUTFRegion(str, 0, len, buff.data());
        buff[len] = '\0';
        return buff.data();
    }

    std::string jthrowableToString(jthrowable e) {
        auto *env = JniHelper::getEnv();
        jobject str = JniHelper::callObjectObjectMethod((jobject)e, "java/lang/Throwable", "toString", "Ljava/lang/String;");
        auto ret = jstringToString(str);
        env->DeleteLocalRef(str);
        return ret;
    }

    jobject getJObjectClassObject(jobject obj) {
        auto *env = JniHelper::getEnv();
        jclass objectClass = env->GetObjectClass(obj);
        jmethodID getClassMethodID = env->GetMethodID(objectClass, "getClass", "()Ljava/lang/Class;");
        jobject classObject = env->CallObjectMethod(obj, getClassMethodID);
        env->DeleteLocalRef(objectClass);
        return classObject;
    }

    std::string getJObjectClass(jobject obj) {
        auto *env = JniHelper::getEnv();

        jobject classObject = getJObjectClassObject(obj);
        jobject classNameString = JniHelper::callObjectObjectMethod(classObject, "java/lang/Class", "getName", "Ljava/lang/String;");

        std::string buff = jstringToString(classNameString);

        env->DeleteLocalRef(classObject);
        env->DeleteLocalRef(classNameString);

        return buff;
    }

    std::vector<std::string> getJobjectMethods(jobject obj) {
        auto *env = JniHelper::getEnv();
        jobject classObject = getJObjectClassObject(obj);
        jobjectArray methods = (jobjectArray) JniHelper::callObjectObjectMethod(classObject, "java/lang/Class", "getMethods", "[Ljava/lang/reflect/Method;");
        std::vector<std::string> methodList;
        auto len = env->GetArrayLength(methods);
        for(auto i = 0; i < len; i++) {
            jobject method = env->GetObjectArrayElement(methods, i);
            jobject methodName = JniHelper::callObjectObjectMethod(method, "java/lang/reflect/Method", "getName", "Ljava/lang/String;");
            std::stringstream ss;
            ss << jstringToString(methodName);
            jobjectArray paramTypes = (jobjectArray)JniHelper::callObjectObjectMethod(method, "java/lang/reflect/Method", "getParameterTypes", "[Ljava/lang/Class;");
            auto len = env->GetArrayLength(paramTypes);
            ss << " # (";
            for(auto j = 0 ; j < len; j++) {
                jobject m = env->GetObjectArrayElement(paramTypes, 0);
                jobject paramName = JniHelper::callObjectObjectMethod(m, "java/lang/Class", "getName", "Ljava/lang/String;");
                ss << jstringToString(paramName);
            }
            ss << ")";
            jobject returnType = JniHelper::callObjectObjectMethod(method, "java/lang/reflect/Method", "getReturnType", "Ljava/lang/Class;");
            jobject returnTypeName = JniHelper::callObjectObjectMethod(returnType, "java/lang/Class", "getName", "Ljava/lang/String;");
            ss << jstringToString(returnTypeName);
            methodList.push_back(ss.str());
        }

        return methodList;
    }

    std::vector<std::string> getJobjectFields(jobject obj) {
        auto *env = JniHelper::getEnv();
        jobject classObject = getJObjectClassObject(obj);
        jobjectArray fields = (jobjectArray) JniHelper::callObjectObjectMethod(classObject, "java/lang/Class", "getFields", "[Ljava/lang/reflect/Field;");
        std::vector<std::string> fieldList;
        auto len = env->GetArrayLength(fields);
        for(auto i = 0; i < len; i++) {
            jobject fieldObj = env->GetObjectArrayElement(fields, i);
            jobject fieldName = JniHelper::callObjectObjectMethod(fieldObj, "java/lang/reflect/Field", "getName", "Ljava/lang/String;");
            std::string name = jstringToString(fieldName);

            jobject fieldClass = JniHelper::callObjectObjectMethod(fieldObj, "java/lang/reflect/Field", "getType", "Ljava/lang/Class;");
            jobject fieldTypeName = JniHelper::callObjectObjectMethod(fieldClass, "java/lang/Class","getName", "Ljava/lang/String;");
            std::string fieldTypeNameStr = jstringToString(fieldTypeName);
            fieldList.push_back(name+" # "+fieldTypeNameStr);
        }

        return fieldList;
    }

    class JValueWrapper {
    public:
        JValueWrapper(const  jni_utils::JniType& type_, jvalue v_): type(type_), value(v_) {}

        const jni_utils::JniType & getType() const {return type;}

        jboolean getBoolean() const {
            assert(type.isBoolean());
            return value.z;
        }

        jbyte getByte() const {
            assert(type.isByte());
            return value.b;
        }

        jchar getChar() const {
            assert(type.isChar());
            return value.c;
        }

        jshort getShort() const {
            assert(type.isShort());
            return value.s;
        }

        jint getInt() const {
            assert(type.isInt());
            return value.i;
        }

        jlong getLong() const {
            assert(type.isLong());
            return value.j;
        }

        jfloat getFloat() const {
            assert(type.isFloat());
            return value.f;
        }

        jdouble getDouble() const {
            assert(type.isDouble());
            return value.d;
        }

        jobject getObject() const {
            assert(type.isObject());
            return value.l;
        }

        bool cast(se::Value &out);

    private:
        jni_utils::JniType type;
        jvalue value;
    };

    class JObjectWrapper {
    public:
        JObjectWrapper(jobject obj){
            if(obj) {
                _javaObject = JniHelper::getEnv()->NewGlobalRef(obj);
            }
        }
        virtual ~JObjectWrapper() {
            if(_javaObject) {
                JniHelper::getEnv()->DeleteGlobalRef(_javaObject);
                _javaObject = nullptr;
            }

        }

        se::Object * asJSObject() {
            if(!_javaObject) return nullptr;
            if(_jsProxy) return _jsProxy;
            _jsObject = se::Object::createObjectWithClass(__jsb_jni_jobject);
            _jsObject->setPrivateData(this);
            _jsProxy = _jsObject->proxyTo(sProxyObject);
#if JS_JNI_DEBUG
            _jsProxy->setProperty(JS_JNI_TAG_TYPE, se::Value("jobject"));
            _jsProxy->setProperty(JS_JNI_JCLASS_TYPE, se::Value(getJObjectClass(_javaObject)));
#endif
            auto f1 = _jsProxy->getPrivateFieldCount();
            auto f2 = _jsObject->getPrivateFieldCount();

            _jsProxy->attachObject(_jsObject);

            return _jsProxy;
        }

        jobject getJavaObject() const  {
            return _javaObject;
        }

        jclass getClass() const {
            return JniHelper::getEnv()->GetObjectClass(_javaObject);
        }

        se::Object *getProxy() const {return _jsProxy;}
        se::Object *getUnderlineJSObject() const {return _jsObject;}

        bool findMethods(const std::string &name, const std::string &signature);

        JValueWrapper* findField(const std::string &name);

    private:
        jobject _javaObject = nullptr;
        se::Object *_jsProxy = nullptr;
        se::Object *_jsObject = nullptr;
    };

    JValueWrapper* JObjectWrapper::findField(const std::string &name)
    {
        auto *env = JniHelper::getEnv();
        std::string klassName = getJObjectClass(_javaObject);
        jobject klassObject = JniHelper::callObjectObjectMethod(_javaObject, klassName, "getClass", "Ljava/lang/Class;");
        jobject fieldObj = JniHelper::callObjectObjectMethod(klassObject,"java/lang/Class","getField", "Ljava/lang/reflect/Field;", name);
        if(fieldObj == nullptr || env->ExceptionCheck()) {
            env->ExceptionClear();
            return nullptr;
        }
        jobject fieldType = JniHelper::callObjectObjectMethod(fieldObj, "java/lang/reflect/Field", "getType", "Ljava/lang/Class;");
        jobject fieldTypeName = JniHelper::callObjectObjectMethod(fieldType, "java/lang/Class", "getName", "Ljava/lang/String;");
        std::string fieldNameStr = jstringToString(fieldTypeName);
        jni_utils::JniType jniFieldType = jni_utils::JniType::fromCanonicalName(fieldNameStr);
        jfieldID fieldId = env->GetFieldID(env->GetObjectClass(_javaObject), name.c_str(), jniFieldType.toString().c_str());
        if(fieldId == nullptr || env->ExceptionCheck()) {
            jthrowable e = env->ExceptionOccurred();
            auto exceptionString = jthrowableToString(e);
            SE_LOGE("Exception caught when access %s#%s\n %s", klassName.c_str(), name.c_str(), exceptionString.c_str());
            env->ExceptionClear();
            return nullptr;
        }
        jvalue ret;
        if(jniFieldType.dim == 0)
        {
            if(jniFieldType.isBoolean()) {
                ret.z = env->GetBooleanField(_javaObject, fieldId);
            }else if(jniFieldType.isByte()) {
                ret.b = env->GetByteField(_javaObject, fieldId);
            }else if(jniFieldType.isChar()){
                ret.c = env->GetCharField(_javaObject, fieldId);
            }else if(jniFieldType.isShort()) {
                ret.s = env->GetShortField(_javaObject, fieldId);
            }else if(jniFieldType.isInt()) {
                ret.i = env->GetIntField(_javaObject, fieldId);
            }else if(jniFieldType.isLong()) {
                ret.j = env->GetLongField(_javaObject, fieldId);
            }else if(jniFieldType.isFloat()) {
                ret.f = env->GetFloatField(_javaObject, fieldId);
            }else if(jniFieldType.isDouble()) {
                ret.d = env->GetDoubleField(_javaObject, fieldId);
            }else if(jniFieldType.isObject()) {
                ret.l = env->GetObjectField(_javaObject, fieldId);
            }else {
                assert(false);
            }
        }
        else if(jniFieldType.dim > 0)
        {
            ret.l = env->GetObjectField(_javaObject, fieldId);
        }

        return new JValueWrapper(jniFieldType, ret);
    }

    bool JObjectWrapper::findMethods(const std::string &name, const std::string &signature)
    {
        //TODO
        return false;
    }


    bool JValueWrapper::cast(se::Value &out) {
        auto *env = JniHelper::getEnv();
        if(type.dim == 0)
        {
            if (type.isBoolean()) {
                out.setBoolean(value.z);
            } else if (type.isByte()) {
                out.setUint8(value.b);
            } else if (type.isChar()) {
                out.setUint16(value.c);
            } else if (type.isShort()) {
                out.setInt16(value.s);
            } else if (type.isInt()) {
                out.setInt32(value.i);
            } else if (type.isLong()) {
                out.setInt32((int32_t) value.l); // use double ??
            } else if (type.isFloat()) {
                out.setFloat(value.f);
            } else if (type.isDouble()) {
                out.setFloat(value.d);
            } else if (type.isObject()) {
                if (type.getClassName() == "java/lang/String")
                {
                    out.setString(jstringToString(value.l));
                }
                else
                {
                    auto *obj = new JObjectWrapper(value.l);
                    out.setObject(obj->asJSObject());
                }
            }
        }
        else if(type.dim == 1)
        {
            auto jarr = (jarray) value.l;
            auto len = env->GetArrayLength(jarr);
            se::Object * array = se::Object::createArrayObject(len);
            jboolean isCopy = false;
            if (type.isBoolean()) {
                auto  *tmp=env->GetBooleanArrayElements((jbooleanArray)jarr, &isCopy);
                for(auto i =0;i<len; i++) {
                    array->setArrayElement(i, se::Value((bool)tmp[i]));
                }
            } else if (type.isByte()) {
                auto  *tmp=env->GetByteArrayElements((jbyteArray)jarr, &isCopy);
                for(auto i =0;i<len; i++) {
                    array->setArrayElement(i, se::Value((uint8_t)tmp[i]));
                }
            } else if (type.isChar()) {
                auto  *tmp=env->GetCharArrayElements((jcharArray)jarr, &isCopy);
                for(auto i =0;i<len; i++) {
                    array->setArrayElement(i, se::Value((uint16_t)tmp[i]));
                }
            } else if (type.isShort()) {
                auto *tmp=env->GetShortArrayElements((jshortArray)jarr, &isCopy);
                for(auto i =0;i<len; i++) {
                    array->setArrayElement(i, se::Value((int16_t)tmp[i]));
                }
            } else if (type.isInt()) {
                auto  *tmp = env->GetIntArrayElements((jintArray)jarr, &isCopy);
                for(auto i =0;i<len; i++) {
                    array->setArrayElement(i, se::Value((int)tmp[i]));
                }
            } else if (type.isLong()) {
                auto  *tmp = env->GetLongArrayElements((jlongArray)jarr, &isCopy);
                for(auto i =0;i<len; i++) {
                    array->setArrayElement(i, se::Value((int32_t)tmp[i]));
                }
            } else if (type.isFloat()) {
                auto  *tmp = env->GetFloatArrayElements((jfloatArray)jarr, &isCopy);
                for(auto i =0;i<len; i++) {
                    array->setArrayElement(i, se::Value((float)tmp[i]));
                }
            } else if (type.isDouble()) {
                auto  *tmp = env->GetDoubleArrayElements((jdoubleArray)jarr, &isCopy);
                for(auto i =0;i<len; i++) {
                    array->setArrayElement(i, se::Value((double)tmp[i]));
                }
            } else if (type.isObject()) {
                for(auto i = 0 ; i < len; i++) {
                    jobject ele = env->GetObjectArrayElement((jobjectArray)jarr, i);
                    if(type.klassName == "java/lang/String") {
                        array->setArrayElement(i, se::Value(jstringToString(ele)));
                    }else {
                        auto *obj = new JObjectWrapper(ele);
                        out.setObject(obj->asJSObject());
                    }
                }
            }
            out.setObject(array);
        } else {
            auto jarr = (jarray) value.l;
            auto len = env->GetArrayLength(jarr);
            se::Object * array = se::Object::createArrayObject(len);
            jboolean isCopy = false;

            jni_utils::JniType type2 = type;
            type2.dim -= 1;
            for(auto i = 0;i < len; i++) {
                se::Value tmpSeValue;
                jvalue jvalueTmp;
                jvalueTmp.l = env->GetObjectArrayElement((jobjectArray)jarr, i);
                JValueWrapper tmpJwrapper(type2, jvalueTmp);
                tmpJwrapper.cast(tmpSeValue);
                array->setArrayElement(i, tmpSeValue);
            }
            out.setObject(array);
        }
        return true;
    }


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
            ret.l = jo->getJavaObject();
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

SE_DECLARE_FUNC(js_jni_proxy_methods);


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
        JniHelper::setClassLoaderFrom(jobj->getJavaObject());
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

static bool js_jni_proxy_apply(se::State& s) {
    return true;
}
SE_BIND_FUNC(js_jni_proxy_apply)

static bool js_jni_proxy_get(se::State& s) {
    int argCnt = s.args().size();
    if(argCnt < 2) {
        SE_REPORT_ERROR("wrong number of arguments: %d, 2+ expected", (int)argCnt);
        return false;
    }
    assert(s.args()[0].isObject());
    assert(s.args()[1].isString());

    auto *target = s.args()[0].toObject();
    auto method = s.args()[1].toString();
    auto *receive = s.args()[2].toObject();

//    if(method.size() > 2 && method[0] == '_' && method[1] == '_') {
//        s.rval().setUndefined();
//        return true;
//    }

    if(method == "$methods") {
        se::Object *funcObj = sJavaObjectMethodsUnbound->bindThis(target);
        s.rval().setObject(funcObj);
        return true;
    } else if(method == "$fields") {
        se::Object *funcObj = sJavaObjectFieldsUnbound->bindThis(target);
        s.rval().setObject(funcObj);
        return true;
    }


    se::Value outvalue;
    if(target->getRealNamedProperty(method.c_str(), &outvalue)) {
        s.rval() = outvalue;
        return true;
    }

    auto *inner = (JObjectWrapper*)target->getPrivateData();
    if(inner) {
        JValueWrapper *field = inner->findField(method.c_str());
        if(field) {
            field->cast(outvalue);
            s.rval() = outvalue;
            return true;
        }
    }


    SE_LOGE("try to find field '%s'", method.c_str());

    return true;
}
SE_BIND_FUNC(js_jni_proxy_get)

static bool js_jni_proxy_set(se::State& s) {
    int argCnt = s.args().size();
    if(argCnt < 2) {
        SE_REPORT_ERROR("wrong number of arguments: %d, 2+ expected", (int)argCnt);
        return false;
    }
    assert(s.args()[0].isObject());
    assert(s.args()[1].isString());

    return true;
}
SE_BIND_FUNC(js_jni_proxy_set)

// query public methods of instance
static bool js_jni_proxy_methods(se::State& s) {
    auto *env = JniHelper::getEnv();
    int argCnt = s.args().size();
//    if(argCnt < 1) {
//        SE_REPORT_ERROR("wrong number of arguments: %d, 2+ expected", (int)argCnt);
//        return false;
//    }
    auto *jsThis = s.getJSThisObject();
    assert(jsThis);
    auto *jobjWrapper = (JObjectWrapper*) jsThis->getPrivateData();
    jobject jobj = jobjWrapper->getJavaObject();
    auto methodNames = getJobjectMethods(jobj);
    auto *array = se::Object::createArrayObject(methodNames.size());
    for(int i = 0;i<methodNames.size(); i++) {
        array->setArrayElement(i, se::Value(methodNames[i]));
    }
    s.rval().setObject(array);
    return true;
}
SE_BIND_FUNC(js_jni_proxy_methods)


// query fields of instance
static bool js_jni_proxy_fields(se::State& s) {
    auto *env = JniHelper::getEnv();
    int argCnt = s.args().size();
//    if(argCnt < 1) {
//        SE_REPORT_ERROR("wrong number of arguments: %d, 2+ expected", (int)argCnt);
//        return false;
//    }
    auto *jsThis = s.getJSThisObject();
    assert(jsThis);
    auto *jobjWrapper = (JObjectWrapper*) jsThis->getPrivateData();
    jobject jobj = jobjWrapper->getJavaObject();
    auto fieldNames = getJobjectFields(jobj);
    auto *array = se::Object::createArrayObject(fieldNames.size());
    for(int i = 0; i < fieldNames.size(); i++) {
        array->setArrayElement(i, se::Value(fieldNames[i]));
    }
    s.rval().setObject(array);
    return true;
}
SE_BIND_FUNC(js_jni_proxy_fields)

static void setup_proxy_object()
{
    sProxyObject = se::Object::createPlainObject();
    sProxyObject->defineFunction("apply", _SE(js_jni_proxy_apply));
    sProxyObject->defineFunction("get", _SE(js_jni_proxy_get));
    //sProxyObject->defineFunction("set", _SE(js_jni_proxy_set));
//    sProxyObject->defineFunction("_methods", _SE(js_jni_proxy_methods));
    sProxyObject->root(); // unroot somewhere

    sJavaObjectMethodsUnbound = se::Object::createFunctionObject(nullptr, _SE(js_jni_proxy_methods));
    sJavaObjectMethodsUnbound->root();

    sJavaObjectFieldsUnbound = se::Object::createFunctionObject(nullptr, _SE(js_jni_proxy_fields));
    sJavaObjectFieldsUnbound->root();
}

bool jsb_register_jni_manual(se::Object* obj)
{
    setup_proxy_object();
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
