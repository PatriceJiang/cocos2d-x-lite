

#include "jsb_conversions.hpp"
#include "jsb_global.h"
#include "jsb_jni_utils.h"
#include "platform/android/jni/JniHelper.h"
#include "platform/android/jni/JniImp.h"
#include <jni.h>
#include <string>
#include <vector>
#include <stack>
#include <algorithm>
#include <regex>

#define JS_JNI_DEBUG 1
#define JS_JNI_TAG_TYPE "jni_obj_type"
#define JS_JNI_JCLASS_TYPE "java_class"
#define JS_JNI_TAG_PATH "path"

using cocos2d::JniHelper;
using cocos2d::JniMethodInfo;

namespace {
    se::Class *__jsb_jni_jobject = nullptr;
    se::Class *__jsb_jni_jmethod_invoke_instance = nullptr;

    se::Object *sProxyObject = nullptr;
    se::Object *sPathProxObject = nullptr;
    se::Object *sJavaObjectMethodsUnbound = nullptr;
    se::Object *sJavaObjectFieldsUnbound = nullptr;
    se::Object *sJavaObjectMethodApplyUnbound = nullptr;


    se::Object *sJavaStaticMethodApplyUnbound = nullptr;
} // namespace

SE_DECLARE_FUNC(js_jni_proxy_java_path_base);

namespace {

    class JValueWrapper;

    class JObjectWrapper;

    class JPathWrapper;

    bool
    getJFieldByType1D(JNIEnv *env, jobject jthis, const jni_utils::JniType &type, jfieldID fieldId,
                      jvalue &ret);

    jvalue seval_to_jvalule(JNIEnv *env, const jni_utils::JniType &def,
                            const se::Value &val, bool &ok);

    bool jobject_to_seval(JNIEnv *env, const jni_utils::JniType &type, jvalue obj, se::Value &out);

    std::vector<jvalue> convertFuncArgs(JNIEnv *env, const std::string &signature,
                                        const std::vector<se::Value> &args,
                                        int offset, bool &success);

    bool getJobjectStaticFieldByType(JNIEnv *env, jclass kls, jfieldID field,
                                     const jni_utils::JniType &type, se::Value &out);

    bool setJobjectStaticFieldByType(JNIEnv *env, jclass kls, jfieldID field,
                                     const jni_utils::JniType &type, const se::Value &out);


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
        jobject str = JniHelper::callObjectObjectMethod(
                (jobject) e, "java/lang/Throwable", "toString", "Ljava/lang/String;");
        auto ret = jstringToString(str);
        env->DeleteLocalRef(str);
        return ret;
    }

    jobject getJObjectClassObject(jobject obj) {
        auto *env = JniHelper::getEnv();
        jclass objectClass = env->GetObjectClass(obj);
        jmethodID getClassMethodID =
                env->GetMethodID(objectClass, "getClass", "()Ljava/lang/Class;");
        jobject classObject = env->CallObjectMethod(obj, getClassMethodID);
        env->DeleteLocalRef(objectClass);
        return classObject;
    }

    std::string getJObjectClass(jobject obj) {
        auto *env = JniHelper::getEnv();

        jobject classObject = getJObjectClassObject(obj);
        jobject classNameString = JniHelper::callObjectObjectMethod(
                classObject, "java/lang/Class", "getName", "Ljava/lang/String;");

        std::string buff = jstringToString(classNameString);

        env->DeleteLocalRef(classObject);
        env->DeleteLocalRef(classNameString);

        return buff;
    }

    std::string getJMethodName(jobject method) {
        jobject methodName = JniHelper::callObjectObjectMethod(
                method, "java/lang/reflect/Method", "getName", "Ljava/lang/String;");
        return jstringToString(methodName);
    }

    std::string getJMethodReturnType(jobject method) {
        jobject returnType =
                JniHelper::callObjectObjectMethod(method, "java/lang/reflect/Method",
                                                  "getReturnType", "Ljava/lang/Class;");
        jobject returnTypeName = JniHelper::callObjectObjectMethod(
                returnType, "java/lang/Class", "getName", "Ljava/lang/String;");
        auto jniRetType = jni_utils::JniType::fromString(jstringToString(returnTypeName));
        return jniRetType.toString();
    }

    std::string getJConstructorSignature(JNIEnv *env, jobject constructor) {
        std::stringstream ss;
        jobjectArray paramTypes = (jobjectArray) JniHelper::callObjectObjectMethod(constructor,
                                                                                   "java/lang/reflect/Constructor",
                                                                                   "getParameterTypes",
                                                                                   "[Ljava/lang/Class;");
        auto len = env->GetArrayLength(paramTypes);
        ss << "(";
        for (auto j = 0; j < len; j++) {
            jobject m = env->GetObjectArrayElement(paramTypes, 0);
            jobject paramName = JniHelper::callObjectObjectMethod(
                    m, "java/lang/Class", "getName", "Ljava/lang/String;");
            ss << jni_utils::JniType::reparse(jstringToString(paramName));
        }
        ss << ")";
        ss << "V";
        return ss.str();
    }

    std::string getJMethodSignature(jobject method) {
        std::stringstream ss;
        auto *env = JniHelper::getEnv();
        jobjectArray paramTypes = (jobjectArray) JniHelper::callObjectObjectMethod(method,
                                                                                   "java/lang/reflect/Method",
                                                                                   "getParameterTypes",
                                                                                   "[Ljava/lang/Class;");
        auto len = env->GetArrayLength(paramTypes);
        ss << "(";
        for (auto j = 0; j < len; j++) {
            jobject m = env->GetObjectArrayElement(paramTypes, 0);
            jobject paramName = JniHelper::callObjectObjectMethod(
                    m, "java/lang/Class", "getName", "Ljava/lang/String;");
            ss << jni_utils::JniType::reparse(jstringToString(paramName));
        }
        ss << ")";
        ss << getJMethodReturnType(method);
        return ss.str();
    }

    std::vector<std::string> getJobjectMethods(jobject obj) {
        auto *env = JniHelper::getEnv();
        jobject classObject = getJObjectClassObject(obj);
        jobjectArray methods = (jobjectArray) JniHelper::callObjectObjectMethod(
                classObject, "java/lang/Class", "getMethods",
                "[Ljava/lang/reflect/Method;");
        std::vector<std::string> methodList;
        auto len = env->GetArrayLength(methods);
        for (auto i = 0; i < len; i++) {
            jobject method = env->GetObjectArrayElement(methods, i);
            std::stringstream ss;
            ss << getJMethodName(method);
            ss << " # ";
            ss << getJMethodSignature(method);
            methodList.push_back(ss.str());
        }

        return methodList;
    }

    bool setJobjectFieldByType(JNIEnv *env, const jni_utils::JniType &type, jobject jthis,
                               jfieldID field, const jvalue value) {
        if (type.dim == 0) {
            if (type.isBoolean()) {
                env->SetBooleanField(jthis, field, value.z);
            } else if (type.isChar()) {
                env->SetCharField(jthis, field, value.c);
            } else if (type.isShort()) {
                env->SetShortField(jthis, field, value.s);
            } else if (type.isByte()) {
                env->SetByteField(jthis, field, value.b);
            } else if (type.isInt()) {
                env->SetIntField(jthis, field, value.i);
            } else if (type.isLong()) {
                env->SetIntField(jthis, field, value.j);
            } else if (type.isFloat()) {
                env->SetFloatField(jthis, field, value.f);
            } else if (type.isDouble()) {
                env->SetDoubleField(jthis, field, value.d);
            } else if (type.isObject()) {
                env->SetObjectField(jthis, field, value.l);
            } else {
                assert(false);
            }
        } else {
            // array
            env->SetObjectField(jthis, field, value.l);
        }

        return true;
    }

    bool setJobjectField(jobject obj, const std::string &fieldName, const se::Value &value,
                         bool &hasField) {
        auto *env = JniHelper::getEnv();
        bool ok = false;
        jobject classObject = getJObjectClassObject(obj);
        jobject field = JniHelper::callObjectObjectMethod(
                classObject, "java/lang/Class", "getField",
                "Ljava/lang/reflect/Field;", fieldName);
        if (!field || env->ExceptionCheck()) {
            env->ExceptionClear();
            hasField = false;
            return true;
        }
        hasField = true;

        jobject fieldClassObject = JniHelper::callObjectObjectMethod(
                field, "java/lang/reflect/Field", "getType",
                "Ljava/lang/Class;");
        jobject fieldTypeJNIName = JniHelper::callObjectObjectMethod(
                fieldClassObject, "java/lang/Class", "getName",
                "Ljava/lang/String;");

        jni_utils::JniType fieldType = jni_utils::JniType::fromString(
                jstringToString(fieldTypeJNIName));
        jvalue ret = seval_to_jvalule(env, fieldType, value, ok);
        if (ok) {
            jfieldID fieldId = env->GetFieldID(env->GetObjectClass(obj), fieldName.c_str(),
                                               fieldType.toString().c_str());
            if (!fieldId || env->ExceptionCheck()) {
                env->ExceptionClear();
                return false;
            }
            ok &= setJobjectFieldByType(env, fieldType, obj, fieldId, ret);
        }
        return ok;
    }

    std::vector<std::string> getJobjectFields(jobject obj) {
        auto *env = JniHelper::getEnv();
        jobject classObject = getJObjectClassObject(obj);
        jobjectArray fields = (jobjectArray) JniHelper::callObjectObjectMethod(
                classObject, "java/lang/Class", "getFields",
                "[Ljava/lang/reflect/Field;");
        std::vector<std::string> fieldList;
        auto len = env->GetArrayLength(fields);
        for (auto i = 0; i < len; i++) {
            jobject fieldObj = env->GetObjectArrayElement(fields, i);
            jobject fieldName = JniHelper::callObjectObjectMethod(
                    fieldObj, "java/lang/reflect/Field", "getName", "Ljava/lang/String;");
            std::string name = jstringToString(fieldName);

            jobject fieldClass = JniHelper::callObjectObjectMethod(
                    fieldObj, "java/lang/reflect/Field", "getType", "Ljava/lang/Class;");
            jobject fieldTypeName = JniHelper::callObjectObjectMethod(
                    fieldClass, "java/lang/Class", "getName", "Ljava/lang/String;");
            std::string fieldTypeNameStr = jstringToString(fieldTypeName);
            fieldList.push_back(name + " # " + fieldTypeNameStr);
        }

        return fieldList;
    }

    jobject
    invokeConstructor(JNIEnv *env, const std::string &path, jclass klass, jobject constructor,
                      se::Object *args) {
        jobjectArray arguments = (jobjectArray) JniHelper::callObjectObjectMethod(constructor,
                                                                                  "java/lang/reflect/Constructor",
                                                                                  "getParameterTypes",
                                                                                  "[Ljava/lang/Class;");
        std::vector<se::Value> argVector;
        {
            uint32_t argLen = 0;
            args->getArrayLength(&argLen);
            argVector.resize(argLen);
            for (int i = 0; i < argLen; i++) {
                args->getArrayElement(i, &argVector[i]);
            }
        }
        auto signature = getJConstructorSignature(env, constructor);
        bool ok = false;

        jmethodID initMethod = env->GetMethodID(klass, "<init>", signature.c_str());
        if (!initMethod || env->ExceptionCheck()) {
            env->ExceptionClear();
            return nullptr;
        }

        std::vector<jvalue> jargs = convertFuncArgs(env, signature, argVector, 0, ok);
        if (!ok) {
            return nullptr;
        }

        jobject obj = env->NewObjectA(klass, initMethod, jargs.data());
        if (!obj || env->ExceptionCheck()) {
            env->ExceptionClear();
            return nullptr;
        }

        return obj;
    }


    jobject constructByClassPath(const std::string &path, se::Object *args) {

        JNIEnv *env = JniHelper::getEnv();
        std::stack<jclass> classStack;
        jclass tmpClass = nullptr;
        bool ok = false;

        std::string classPath = std::regex_replace(path, std::regex("\\."), "/");

        // test class path
        tmpClass = env->FindClass(classPath.c_str());
        if (tmpClass == nullptr || env->ExceptionCheck()) env->ExceptionClear();

        jobjectArray constructors = (jobjectArray) JniHelper::callObjectObjectMethod(tmpClass,
                                                                                     "java/lang/Class",
                                                                                     "getConstructors",
                                                                                     "[Ljava/lang/reflect/Constructor;");
        if (constructors == nullptr)
            return nullptr;
        int constructLen = env->GetArrayLength(constructors);
        for (int i = 0; i < constructLen; i++) {
            jobject constructor = env->GetObjectArrayElement(constructors, i);
            jobject ret = invokeConstructor(env, classPath, tmpClass, constructor, args);
            if (ret) {
                return ret;
            }
        }

        return nullptr;
    }

    jfieldID getJClassStaticField(JNIEnv *env, jclass klass, const std::string &fieldName,
                                  jni_utils::JniType &fieldType) {
        jobject fieldObj = JniHelper::callObjectObjectMethod(klass, "java/lang/Class", "getField",
                                                             "Ljava/lang/reflect/Field;",
                                                             fieldName);
        if (!fieldObj || env->ExceptionCheck()) {
            env->ExceptionClear();
            return nullptr;
        }
        jclass modifierClass = env->FindClass("java/lang/reflect/Modifier");
        jfieldID modifierStaticField = env->GetStaticFieldID(modifierClass, "STATIC", "I");
        int staticFlag = env->GetStaticIntField(modifierClass, modifierStaticField);
        int modifiers = JniHelper::callObjectIntMethod(fieldObj, "java/lang/reflect/Field",
                                                       "getModifiers");
        if ((modifiers & staticFlag) == 0) {
            return nullptr;
        }

        if (env->ExceptionCheck()) {
            env->ExceptionClear();
            return nullptr;
        }

        jobject fieldClassObj = JniHelper::callObjectObjectMethod(fieldObj,
                                                                  "java/lang/reflect/Field",
                                                                  "getType", "Ljava/lang/Class;");
        jobject fieldTypeName = JniHelper::callObjectObjectMethod(fieldClassObj, "java/lang/Class",
                                                                  "getName", "Ljava/lang/String;");
        fieldType = jni_utils::JniType::fromString(jstringToString(fieldTypeName));
        jfieldID fid = env->GetStaticFieldID(klass, fieldName.c_str(),
                                             fieldType.toString().c_str());
        if (!fid || env->ExceptionCheck()) {
            env->ExceptionClear();
            return nullptr;
        }


        return fid;
    }

    bool hasStaticField(const std::string &longPath) {
        JNIEnv *env = JniHelper::getEnv();

        std::string path = std::regex_replace(longPath, std::regex("\\."), "/");
        std::string fieldName;
        std::string className;
        jni_utils::JniType fieldType;
        {
            auto idx = path.rfind("/");
            if (idx == std::string::npos) {
                return false;
            }
            fieldName = path.substr(idx + 1);
            className = path.substr(0, idx);
        }

        jclass kls = env->FindClass(className.c_str());
        if (kls == nullptr || env->ExceptionCheck()) {
            env->ExceptionClear();
            return false;
        }
        jfieldID f = getJClassStaticField(env, kls, fieldName, fieldType);
        return f != nullptr;
    }


    bool getJobjectStaticField(const std::string &longPath, se::Value &ret) {
        JNIEnv *env = JniHelper::getEnv();

        std::string path = std::regex_replace(longPath, std::regex("\\."), "/");
        std::string fieldName;
        std::string className;
        jni_utils::JniType fieldType;
        {
            auto idx = path.rfind("/");
            if (idx == std::string::npos) {
                return false;
            }
            fieldName = path.substr(idx + 1);
            className = path.substr(0, idx);
        }

        jclass kls = env->FindClass(className.c_str());
        if (kls == nullptr || env->ExceptionCheck()) {
            env->ExceptionClear();
            return false;
        }
        jfieldID f = getJClassStaticField(env, kls, fieldName, fieldType);
        if (f == nullptr) {
            return false;
        }

        return getJobjectStaticFieldByType(env, kls, f, fieldType, ret);
    }

    bool setJobjectStaticField(const std::string &longPath, const se::Value &ret) {
        JNIEnv *env = JniHelper::getEnv();

        std::string path = std::regex_replace(longPath, std::regex("\\."), "/");
        std::string fieldName;
        std::string className;
        jni_utils::JniType fieldType;
        {
            auto idx = path.rfind("/");
            if (idx == std::string::npos) {
                return false;
            }
            fieldName = path.substr(idx + 1);
            className = path.substr(0, idx);
        }

        jclass kls = env->FindClass(className.c_str());
        if (kls == nullptr || env->ExceptionCheck()) {
            env->ExceptionClear();
            return false;
        }
        jfieldID f = getJClassStaticField(env, kls, fieldName, fieldType);
        if (f == nullptr) {
            std::string msg = "field '" + fieldName + "' is not found in '" + className + "'";
            se::ScriptEngine::getInstance()->throwException(msg.c_str());
            return false;
        }

        return setJobjectStaticFieldByType(env, kls, f, fieldType, ret);
    }


    class JValueWrapper {
    public:
        JValueWrapper(const jni_utils::JniType &type_, jvalue v_)
                : type(type_), value(v_) {}

        const jni_utils::JniType &getType() const { return type; }

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

    struct JMethod {
        jmethodID method{};
        std::string signature;
    };


    std::vector<JMethod>
    getJStaticMethodsByName(JNIEnv *env, jclass klass, const std::string &methodName) {
        std::vector<JMethod> ret;
        jobject methods = JniHelper::callObjectObjectMethod(klass, "java/lang/Class", "getMethods",
                                                            "[Ljava/lang/reflect/Method;");
        int len = env->GetArrayLength((jarray) methods);
        for (int i = 0; i < len; i++) {
            jobject method = env->GetObjectArrayElement((jobjectArray) methods, i);
            jobject methodNameObj = JniHelper::callObjectObjectMethod(method,
                                                                      "java/lang/reflect/Method",
                                                                      "getName",
                                                                      "Ljava/lang/String;");
            if (jstringToString(methodNameObj) == methodName) {
                JMethod m;
                m.signature = getJMethodSignature(method);
                m.method = env->GetStaticMethodID(klass, methodName.c_str(), m.signature.c_str());
                ret.push_back(m);
            }
        }
        return ret;
    }

    class InvokeMethods {
    public:
        InvokeMethods(std::string methodName, const std::vector<JMethod> &_methods,
                      JObjectWrapper *_self) :
                methodName(std::move(methodName)), methods(_methods), self(_self) {
        }

        se::Object *asJSObject() {
            if (_jsobj) return _jsobj;
            _jsobj = se::Object::createObjectWithClass(__jsb_jni_jmethod_invoke_instance);
            _jsobj->setPrivateData(this);
            return _jsobj;
        }

        std::vector<JMethod> methods;
        JObjectWrapper *self = nullptr;
        std::string methodName;
    private:
        se::Object *_jsobj = nullptr;
    };

    class JObjectWrapper {
    public:
        explicit JObjectWrapper(jobject obj) {
            if (obj) {
                _javaObject = JniHelper::getEnv()->NewGlobalRef(obj);
            }
        }

        virtual ~JObjectWrapper() {
            if (_javaObject) {
                JniHelper::getEnv()->DeleteGlobalRef(_javaObject);
                _javaObject = nullptr;
            }
        }

        se::Object *asJSObject() {
            if (!_javaObject)
                return nullptr;
            if (_jsProxy)
                return _jsProxy;
            _jsTarget = se::Object::createObjectWithClass(__jsb_jni_jobject);
            _jsTarget->setPrivateData(this);
            _jsProxy = _jsTarget->proxyTo(sProxyObject);
#if JS_JNI_DEBUG
            _jsProxy->setProperty(JS_JNI_TAG_TYPE, se::Value("jobject"));
            _jsProxy->setProperty(JS_JNI_JCLASS_TYPE,
                                  se::Value(getJObjectClass(_javaObject)));
#endif
            _jsProxy->attachObject(_jsTarget);

            return _jsProxy;
        }

        jobject getJavaObject() const { return _javaObject; }

        jclass getClass() const {
            return JniHelper::getEnv()->GetObjectClass(_javaObject);
        }

        se::Object *getProxy() const { return _jsProxy; }

        se::Object *getUnderlineJSObject() const { return _jsTarget; }

        bool findMethods(const std::string &name, const std::string &signature,
                         std::vector<JMethod> &method);

        JValueWrapper *getFieldValue(const std::string &name);

    private:
        jobject _javaObject = nullptr;
        se::Object *_jsProxy = nullptr;
        se::Object *_jsTarget = nullptr;
    };

    JValueWrapper *JObjectWrapper::getFieldValue(const std::string &name) {
        auto *env = JniHelper::getEnv();
        std::string klassName = getJObjectClass(_javaObject);
        jobject klassObject = JniHelper::callObjectObjectMethod(
                _javaObject, klassName, "getClass", "Ljava/lang/Class;");
        jobject fieldObj = JniHelper::callObjectObjectMethod(
                klassObject, "java/lang/Class", "getField", "Ljava/lang/reflect/Field;",
                name);
        if (fieldObj == nullptr || env->ExceptionCheck()) {
            env->ExceptionClear();
            return nullptr;
        }
        jobject fieldType = JniHelper::callObjectObjectMethod(
                fieldObj, "java/lang/reflect/Field", "getType", "Ljava/lang/Class;");
        jobject fieldTypeName = JniHelper::callObjectObjectMethod(
                fieldType, "java/lang/Class", "getName", "Ljava/lang/String;");
        std::string fieldNameStr = jstringToString(fieldTypeName);
        jni_utils::JniType jniFieldType =
                jni_utils::JniType::fromString(fieldNameStr);
        jfieldID fieldId =
                env->GetFieldID(env->GetObjectClass(_javaObject), name.c_str(),
                                jniFieldType.toString().c_str());
        if (fieldId == nullptr || env->ExceptionCheck()) {
            jthrowable e = env->ExceptionOccurred();
            auto exceptionString = jthrowableToString(e);
            SE_LOGE("Exception caught when access %s#%s\n %s", klassName.c_str(),
                    name.c_str(), exceptionString.c_str());
            env->ExceptionClear();
            return nullptr;
        }
        jvalue ret;
        if (!getJFieldByType1D(env, _javaObject, jniFieldType, fieldId, ret)) {
            return nullptr;
        }
        return new JValueWrapper(jniFieldType, ret);
    }

    bool JObjectWrapper::findMethods(const std::string &name,
                                     const std::string &signature, std::vector<JMethod> &list) {
        auto *env = JniHelper::getEnv();
        std::string klassName = getJObjectClass(_javaObject);
        jclass jobjClass = env->GetObjectClass(_javaObject);
        JMethod oMethod;
        if (!signature.empty()) {
            jmethodID mid = env->GetMethodID(jobjClass, klassName.c_str(), signature.c_str());
            if (!mid || env->ExceptionCheck()) {
                env->ExceptionClear();
                return false;
            }
            oMethod.method = mid;
            oMethod.signature = signature;
            list.push_back(oMethod);
        } else {
            jobject classObj = JniHelper::callObjectObjectMethod(_javaObject, klassName, "getClass",
                                                                 "Ljava/lang/Class;");
            jobjectArray methodsArray = (jobjectArray) JniHelper::callObjectObjectMethod(classObj,
                                                                                         "java/lang/Class",
                                                                                         "getMethods",
                                                                                         "[Ljava/lang/reflect/Method;");
            if (!methodsArray || env->ExceptionCheck()) {
                env->ExceptionClear();
                return false;
            }
            auto len = env->GetArrayLength(methodsArray);
            for (int i = 0; i < len; i++) {
                jobject methodObj = env->GetObjectArrayElement(methodsArray, i);
                std::string methodName = getJMethodName(methodObj);
                if (methodName != name) {
                    continue;
                }
                auto sig = getJMethodSignature(methodObj);
                jmethodID mid = env->GetMethodID(jobjClass, name.c_str(), sig.c_str());
                if (mid == nullptr || env->ExceptionCheck()) {
                    env->ExceptionClear();
                    return false;
                }
                assert(mid);
                oMethod.method = mid;
                oMethod.signature = sig;
                list.push_back(oMethod);
            }
        }

        return true;
    }

    bool JValueWrapper::cast(se::Value &out) {
        auto *env = JniHelper::getEnv();
        if (type.dim == 0) {
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
                if (type.getClassName() == "java/lang/String") {
                    out.setString(jstringToString(value.l));
                } else {
                    auto *obj = new JObjectWrapper(value.l);
                    out.setObject(obj->asJSObject());
                }
            }
        } else if (type.dim == 1) {
            auto jarr = (jarray) value.l;
            auto len = env->GetArrayLength(jarr);
            se::Object *array = se::Object::createArrayObject(len);
            jboolean isCopy = false;
            if (type.isBoolean()) {
                auto *tmp = env->GetBooleanArrayElements((jbooleanArray) jarr, &isCopy);
                for (auto i = 0; i < len; i++) {
                    array->setArrayElement(i, se::Value((bool) tmp[i]));
                }
            } else if (type.isByte()) {
                auto *tmp = env->GetByteArrayElements((jbyteArray) jarr, &isCopy);
                for (auto i = 0; i < len; i++) {
                    array->setArrayElement(i, se::Value((uint8_t) tmp[i]));
                }
            } else if (type.isChar()) {
                auto *tmp = env->GetCharArrayElements((jcharArray) jarr, &isCopy);
                for (auto i = 0; i < len; i++) {
                    array->setArrayElement(i, se::Value((uint16_t) tmp[i]));
                }
            } else if (type.isShort()) {
                auto *tmp = env->GetShortArrayElements((jshortArray) jarr, &isCopy);
                for (auto i = 0; i < len; i++) {
                    array->setArrayElement(i, se::Value((int16_t) tmp[i]));
                }
            } else if (type.isInt()) {
                auto *tmp = env->GetIntArrayElements((jintArray) jarr, &isCopy);
                for (auto i = 0; i < len; i++) {
                    array->setArrayElement(i, se::Value((int) tmp[i]));
                }
            } else if (type.isLong()) {
                auto *tmp = env->GetLongArrayElements((jlongArray) jarr, &isCopy);
                for (auto i = 0; i < len; i++) {
                    array->setArrayElement(i, se::Value((int32_t) tmp[i]));
                }
            } else if (type.isFloat()) {
                auto *tmp = env->GetFloatArrayElements((jfloatArray) jarr, &isCopy);
                for (auto i = 0; i < len; i++) {
                    array->setArrayElement(i, se::Value((float) tmp[i]));
                }
            } else if (type.isDouble()) {
                auto *tmp = env->GetDoubleArrayElements((jdoubleArray) jarr, &isCopy);
                for (auto i = 0; i < len; i++) {
                    array->setArrayElement(i, se::Value((double) tmp[i]));
                }
            } else if (type.isObject()) {
                for (auto i = 0; i < len; i++) {
                    jobject ele = env->GetObjectArrayElement((jobjectArray) jarr, i);
                    if (type.klassName == "java/lang/String") {
                        array->setArrayElement(i, se::Value(jstringToString(ele)));
                    } else {
                        auto *obj = new JObjectWrapper(ele);
                        array->setArrayElement(i, se::Value(obj->asJSObject()));
                    }
                }
            }
            out.setObject(array);
        } else {
            auto jarr = (jarray) value.l;
            auto len = env->GetArrayLength(jarr);
            se::Object *array = se::Object::createArrayObject(len);
            jboolean isCopy = false;

            jni_utils::JniType type2 = type;
            type2.dim -= 1;
            for (auto i = 0; i < len; i++) {
                se::Value tmpSeValue;
                jvalue jvalueTmp;
                jvalueTmp.l = env->GetObjectArrayElement((jobjectArray) jarr, i);
                JValueWrapper tmpJwrapper(type2, jvalueTmp);
                tmpJwrapper.cast(tmpSeValue);
                array->setArrayElement(i, tmpSeValue);
            }
            out.setObject(array);
        }
        return true;
    }

    class JPathWrapper {
    public:
        JPathWrapper() = default;

        JPathWrapper(const std::string &path) {
            _path = path;
        }

        se::Object *asJSObject() {
            if (_jsProxy) { return _jsProxy; }
            _jsTarget = se::Object::createFunctionObject(nullptr, _SE(js_jni_proxy_java_path_base));
            _jsTarget->setPrivateData(this);
            _jsTarget->_setFinalizeCallback([](void *data) {
                if (data) {
                    delete (JPathWrapper *) data;
                }
            });
            _jsProxy = _jsTarget->proxyTo(sPathProxObject);
            _jsProxy->setProperty(JS_JNI_TAG_PATH, se::Value(_path));
            #if JS_JNI_DEBUG
            _jsProxy->setProperty(JS_JNI_TAG_TYPE, se::Value("jpath"));
            #endif


            _jsProxy->attachObject(_jsTarget);
            return _jsProxy;
        }

        se::Object *getProxy() {
            if (!_jsTarget) asJSObject();
            return _jsProxy;
        }

        se::Object *getUnderlineJSObject() {
            if (!_jsTarget) asJSObject();
            return _jsTarget;
        }

    private:
        jclass _currentClass = {};
        se::Object *_jsTarget = nullptr;
        se::Object *_jsProxy = nullptr;
        std::string _path;
    };

    bool
    getJFieldByType1D(JNIEnv *env, jobject jthis, const jni_utils::JniType &type, jfieldID fieldId,
                      jvalue &ret) {
        if (type.dim == 0) {
            if (type.isBoolean()) {
                ret.z = env->GetBooleanField(jthis, fieldId);
            } else if (type.isByte()) {
                ret.b = env->GetByteField(jthis, fieldId);
            } else if (type.isChar()) {
                ret.c = env->GetCharField(jthis, fieldId);
            } else if (type.isShort()) {
                ret.s = env->GetShortField(jthis, fieldId);
            } else if (type.isInt()) {
                ret.i = env->GetIntField(jthis, fieldId);
            } else if (type.isLong()) {
                ret.j = env->GetLongField(jthis, fieldId);
            } else if (type.isFloat()) {
                ret.f = env->GetFloatField(jthis, fieldId);
            } else if (type.isDouble()) {
                ret.d = env->GetDoubleField(jthis, fieldId);
            } else if (type.isObject()) {
                ret.l = env->GetObjectField(jthis, fieldId);
            } else {
                assert(false);
                return false;
            }
        } else {
            ret.l = env->GetObjectField(jthis, fieldId);
        }
        return true;
    }


    bool callJMethodByReturnType(const jni_utils::JniType &rType, jobject jthis, jmethodID method,
                                 const std::vector<jvalue> &jvalueArray, se::Value &out) {
        auto *env = JniHelper::getEnv();
        jvalue jRet = {};
        if (rType.isVoid()) {
            env->CallVoidMethodA(jthis, method, jvalueArray.data());
        } else if (rType.isBoolean()) {
            jRet.z = env->CallBooleanMethodA(jthis, method, jvalueArray.data());
        } else if (rType.isChar()) {
            jRet.c = env->CallCharMethodA(jthis, method, jvalueArray.data());
        } else if (rType.isShort()) {
            jRet.s = env->CallShortMethodA(jthis, method, jvalueArray.data());
        } else if (rType.isByte()) {
            jRet.b = env->CallByteMethodA(jthis, method, jvalueArray.data());
        } else if (rType.isInt()) {
            jRet.i = env->CallIntMethodA(jthis, method, jvalueArray.data());
        } else if (rType.isLong()) {
            jRet.j = env->CallIntMethodA(jthis, method, jvalueArray.data());
        } else if (rType.isFloat()) {
            jRet.f = env->CallFloatMethodA(jthis, method, jvalueArray.data());
        } else if (rType.isDouble()) {
            jRet.d = env->CallDoubleMethodA(jthis, method, jvalueArray.data());
        } else if (rType.isObject()) {
            jRet.l = env->CallObjectMethodA(jthis, method, jvalueArray.data());
        } else {
            assert(false);
            return false;
        }
        JValueWrapper wrap(rType, jRet);
        wrap.cast(out);
        return true;
    }

    bool callStaticMethodByType(const jni_utils::JniType &rType, jclass jthis, jmethodID method,
                                const std::vector<jvalue> &jvalueArray, se::Value &out) {
        auto *env = JniHelper::getEnv();
        jvalue jRet = {};
        if (rType.isVoid()) {
            env->CallStaticVoidMethodA(jthis, method, jvalueArray.data());
        } else if (rType.isBoolean()) {
            jRet.z = env->CallStaticBooleanMethodA(jthis, method, jvalueArray.data());
        } else if (rType.isChar()) {
            jRet.c = env->CallStaticCharMethodA(jthis, method, jvalueArray.data());
        } else if (rType.isShort()) {
            jRet.s = env->CallStaticShortMethodA(jthis, method, jvalueArray.data());
        } else if (rType.isByte()) {
            jRet.b = env->CallStaticByteMethodA(jthis, method, jvalueArray.data());
        } else if (rType.isInt()) {
            jRet.i = env->CallStaticIntMethodA(jthis, method, jvalueArray.data());
        } else if (rType.isLong()) {
            jRet.j = env->CallStaticIntMethodA(jthis, method, jvalueArray.data());
        } else if (rType.isFloat()) {
            jRet.f = env->CallStaticFloatMethodA(jthis, method, jvalueArray.data());
        } else if (rType.isDouble()) {
            jRet.d = env->CallStaticDoubleMethodA(jthis, method, jvalueArray.data());
        } else if (rType.isObject()) {
            jRet.l = env->CallStaticObjectMethodA(jthis, method, jvalueArray.data());
        } else {
            assert(false);
            return false;
        }
        JValueWrapper wrap(rType, jRet);
        wrap.cast(out);
        return true;
    }

    jvalue seval_to_jvalule(JNIEnv *env, const jni_utils::JniType &def,
                            const se::Value &val, bool &ok) {
        jvalue ret = {};
        ok = false;
        if (def.dim == 0) {
            if (def.isBoolean() && val.isBoolean()) {
                ret.z = val.toBoolean();
            } else if (def.isByte() && val.isNumber()) {
                ret.b = val.toInt8();
            } else if (def.isChar() && val.isNumber()) {
                ret.c = val.toInt16();
            } else if (def.isShort() && val.isNumber()) {
                ret.s = val.toInt16();
            } else if (def.isInt() && val.isNumber()) {
                ret.i = val.toInt32();
            } else if (def.isLong() && val.isNumber()) {
                ret.j = static_cast<jlong>(val.toNumber()); // int 64
            } else if (def.isFloat() && val.isNumber()) {
                ret.f = val.toFloat();
            } else if (def.isDouble() && val.isNumber()) {
                ret.d = val.toNumber();
            } else if (def.isObject()) {

                if (def.getClassName() == "java/lang/String") {
                    std::string jsstring = val.toStringForce();
                    ret.l = env->NewStringUTF(jsstring.c_str());
                } else if (val.isObject()) {
                    se::Object *seObj = val.toObject();
                    auto *jo = static_cast<JObjectWrapper *>(seObj->getPrivateData());
                    if (!jo) {
                        SE_LOGE("incorrect jni type, don't know how to convert pure js object %s to java value %s",
                                val.toStringForce().c_str(), def.toString().c_str());
                    }
                    assert(jo);
                    ret.l = jo->getJavaObject();
                } else {
                    ok = false;
                    SE_LOGE("incorrect jni type, don't know how to convert %s to java value %s",
                            val.toStringForce().c_str(), def.toString().c_str());
                }

            } else {
                SE_LOGE("incorrect jni type, don't know how to convert %s from js value %s:%s",
                        def.toString().c_str(), val.getTypeName().c_str(),
                        val.toStringForce().c_str());
                return ret;
            }
            ok = true;
            return ret;
        } else if (def.dim == 1) {

            if (!val.isObject() || !val.toObject()->isArray()) {
                SE_LOGE("incorrect jni type, don't know how to convert %s from js value %s:%s",
                        def.toString().c_str(), val.getTypeName().c_str(),
                        val.toStringForce().c_str());
                return ret;
            }


            jni_utils::JniType decayDef = def;
            decayDef.dim -= 1;

            se::Object *jsArray = val.toObject();
            uint32_t len = 0;
            jsArray->getArrayLength(&len);
            se::Value jsTmp;
            if (def.isBoolean()) {
                auto dst = (jbooleanArray) env->NewBooleanArray(len);
                std::vector<jboolean> tmpArray;
                tmpArray.resize(len);
                for (auto i = 0; i < len; i++) {
                    jsArray->getArrayElement(i, &jsTmp);
                    tmpArray[i] = jsTmp.toBoolean();
                }
                env->SetBooleanArrayRegion(dst, 0, len, tmpArray.data());
                ret.l = dst;
            } else if (def.isChar()) {
                auto dst = (jcharArray) env->NewCharArray(len);
                std::vector<jchar> tmpArray;
                tmpArray.resize(len);
                for (auto i = 0; i < len; i++) {
                    jsArray->getArrayElement(i, &jsTmp);
                    tmpArray[i] = jsTmp.toUint16();
                }
                env->SetCharArrayRegion(dst, 0, len, tmpArray.data());
                ret.l = dst;
            } else if (def.isShort()) {
                auto dst = (jshortArray) env->NewShortArray(len);
                std::vector<jshort> tmpArray;
                tmpArray.resize(len);
                for (auto i = 0; i < len; i++) {
                    jsArray->getArrayElement(i, &jsTmp);
                    tmpArray[i] = jsTmp.toInt16();
                }
                env->SetShortArrayRegion(dst, 0, len, tmpArray.data());
                ret.l = dst;
            } else if (def.isByte()) {
                auto dst = (jbyteArray) env->NewByteArray(len);
                std::vector<jbyte> tmpArray;
                tmpArray.resize(len);
                for (auto i = 0; i < len; i++) {
                    jsArray->getArrayElement(i, &jsTmp);
                    tmpArray[i] = jsTmp.toUint8();
                }
                env->SetByteArrayRegion(dst, 0, len, tmpArray.data());
                ret.l = dst;
            } else if (def.isInt()) {
                auto dst = (jintArray) env->NewIntArray(len);
                std::vector<jint> tmpArray;
                tmpArray.resize(len);
                for (auto i = 0; i < len; i++) {
                    jsArray->getArrayElement(i, &jsTmp);
                    tmpArray[i] = jsTmp.toInt32();
                }
                env->SetIntArrayRegion(dst, 0, len, tmpArray.data());
                ret.l = dst;
            } else if (def.isLong()) {
                auto dst = (jlongArray) env->NewLongArray(len);
                std::vector<jlong> tmpArray;
                tmpArray.resize(len);
                for (auto i = 0; i < len; i++) {
                    jsArray->getArrayElement(i, &jsTmp);
                    tmpArray[i] = (jlong) jsTmp.toNumber();
                }
                env->SetLongArrayRegion(dst, 0, len, tmpArray.data());
                ret.l = dst;
            } else if (def.isFloat()) {
                auto dst = (jfloatArray) env->NewFloatArray(len);
                std::vector<jfloat> tmpArray;
                tmpArray.resize(len);
                for (auto i = 0; i < len; i++) {
                    jsArray->getArrayElement(i, &jsTmp);
                    tmpArray[i] = jsTmp.toFloat();
                }
                env->SetFloatArrayRegion(dst, 0, len, tmpArray.data());
                ret.l = dst;
            } else if (def.isDouble()) {
                auto dst = (jdoubleArray) env->NewDoubleArray(len);
                std::vector<jdouble> tmpArray;
                tmpArray.resize(len);
                for (auto i = 0; i < len; i++) {
                    jsArray->getArrayElement(i, &jsTmp);
                    tmpArray[i] = jsTmp.toNumber();
                }
                env->SetDoubleArrayRegion(dst, 0, len, tmpArray.data());
                ret.l = dst;
            } else if (def.isObject()) {
                jclass kls = env->FindClass(def.getClassName().c_str());
                if (!kls || env->ExceptionCheck()) {
                    env->ExceptionClear();
                    ok = false;
                    SE_LOGE("class '%s' is not found!", def.getClassName().c_str());
                    return ret;
                }
                auto dst = (jobjectArray) env->NewObjectArray(len, kls, nullptr);
                for (auto i = 0; i < len; i++) {
                    jsArray->getArrayElement(i, &jsTmp);
                    jvalue jtmp = seval_to_jvalule(env, decayDef, jsTmp, ok);
                    if (!ok) {
                        ok = false;
                        return ret;
                    }
                    env->SetObjectArrayElement(dst, i, jtmp.l);
                }
                ret.l = dst;
            } else {
                SE_LOGE("incorrect jni type, don't know how to convert %s from js value %s:%s",
                        def.toString().c_str(), val.getTypeName().c_str(),
                        val.toStringForce().c_str());
                return ret;
            }

            ok = true;
            return ret;

        } else {
            jni_utils::JniType decayDef = def;
            decayDef.dim -= 1;
            if (!val.isObject() || !val.toObject()->isArray()) {
                SE_LOGE("incorrect jni type, don't know how to convert %s from js value %s:%s",
                        def.toString().c_str(), val.getTypeName().c_str(),
                        val.toStringForce().c_str());
                return ret;
            }

            uint32_t len;
            se::Object *jsArray = val.toObject();
            jsArray->getArrayLength(&len);
            jclass kls = env->FindClass("java/lang/Object");
            if (!kls || env->ExceptionCheck()) {
                env->ExceptionClear();
                ok = false;
                SE_LOGE("class '%s' is not found!", def.getClassName().c_str());
                return ret;
            }
            jobjectArray jArr = (jobjectArray) env->NewObjectArray(len, kls, nullptr);
            se::Value jsTmp;
            for (int i = 0; i < len; i++) {
                jsArray->getArrayElement(i, &jsTmp);
                jvalue x = seval_to_jvalule(env, decayDef, jsTmp, ok);
                if (!ok) {
                    ok = false;
                    return ret;
                }
                env->SetObjectArrayElement(jArr, i, x.l);
            }
            ret.l = jArr;
            ok = true;
            return ret;
        }
        ok = false;
        return ret;
    }

    std::vector<jvalue> convertFuncArgs(JNIEnv *env, const std::string &signature,
                                        const std::vector<se::Value> &args,
                                        int offset, bool &success) {
        bool convertOk = false;
        success = false;
        std::vector<jni_utils::JniType> argTypes =
                jni_utils::exactArgsFromSignature(signature, convertOk);
        if (!convertOk) {
            SE_LOGE("failed to parse signature \"%s\"", signature.c_str());
            return {};
        }

        if (argTypes.size() != args.size() - offset) {
            SE_LOGE("arguments size %d does not match function signature \"%s\"",
                    args.size(), signature.c_str());
            return {};
        }

        std::vector<jvalue> ret;
        for (size_t i = 0; i < argTypes.size(); i++) {
            jvalue arg =
                    seval_to_jvalule(env, argTypes[i], args[i + offset], convertOk);
            if (!convertOk || env->ExceptionCheck()) {
                env->ExceptionClear();
                success = false;
                return {};
            }
            assert(convertOk);
            ret.push_back(arg);
        }

        success = true;
        return ret;
    }

    bool jobject_to_seval(JNIEnv *env, const jni_utils::JniType &type, jvalue v, se::Value &out) {
        if (type.dim == 0) {
            if (type.isBoolean()) {
                out.setBoolean(v.z);
            } else if (type.isChar()) {
                out.setUint16(v.c);
            } else if (type.isShort()) {
                out.setInt16(v.s);
            } else if (type.isByte()) {
                out.setUint8(v.b);
            } else if (type.isInt()) {
                out.setInt32(v.i);
            } else if (type.isLong()) {
                out.setLong(v.j);
            } else if (type.isFloat()) {
                out.setFloat(v.f);
            } else if (type.isDouble()) {
                out.setNumber(v.d);
            } else if (type.isObject()) {
                JObjectWrapper *w = new JObjectWrapper(v.l);
                out.setObject(w->asJSObject());
            } else {
                assert(false);
                return false;
            }
        } else if (type.dim == 1) {
            auto arr = (jarray) (v.l);
            auto len = env->GetArrayLength(arr);
            se::Object *jsArr = se::Object::createArrayObject(len);
            jboolean copyOut = false;
            if (type.isBoolean()) {
                auto *data = env->GetBooleanArrayElements((jbooleanArray) arr, &copyOut);
                for (int i = 0; i < len; i++) {
                    jsArr->setArrayElement(i, se::Value((bool) data[i]));
                }
            } else if (type.isChar()) {
                auto *data = env->GetCharArrayElements((jcharArray) arr, &copyOut);
                for (int i = 0; i < len; i++) {
                    jsArr->setArrayElement(i, se::Value((uint16_t) data[i]));
                }
            } else if (type.isShort()) {
                auto *data = env->GetShortArrayElements((jshortArray) arr, &copyOut);
                for (int i = 0; i < len; i++) {
                    jsArr->setArrayElement(i, se::Value((int16_t) data[i]));
                }
            } else if (type.isByte()) {
                auto *data = env->GetByteArrayElements((jbyteArray) arr, &copyOut);
                for (int i = 0; i < len; i++) {
                    jsArr->setArrayElement(i, se::Value((int8_t) data[i]));
                }
            } else if (type.isInt()) {
                auto *data = env->GetIntArrayElements((jintArray) arr, &copyOut);
                for (int i = 0; i < len; i++) {
                    jsArr->setArrayElement(i, se::Value((jint) data[i]));
                }
            } else if (type.isLong()) {
                auto *data = env->GetLongArrayElements((jlongArray) arr, &copyOut);
                for (int i = 0; i < len; i++) {
                    jsArr->setArrayElement(i, se::Value((int32_t) data[i]));
                }
            } else if (type.isFloat()) {
                auto *data = env->GetFloatArrayElements((jfloatArray) arr, &copyOut);
                for (int i = 0; i < len; i++) {
                    jsArr->setArrayElement(i, se::Value((float) data[i]));
                }
            } else if (type.isDouble()) {
                auto *data = env->GetDoubleArrayElements((jdoubleArray) arr, &copyOut);
                for (int i = 0; i < len; i++) {
                    jsArr->setArrayElement(i, se::Value((jdouble) data[i]));
                }
            } else if (type.isObject()) {
                for (int i = 0; i < len; i++) {
                    jobject t = env->GetObjectArrayElement((jobjectArray) arr, i);
                    JObjectWrapper *jw = new JObjectWrapper(t);
                    jsArr->setArrayElement(i, se::Value(jw->asJSObject()));
                }
            } else {
                assert(false);
                return false;
            }
            out.setObject(jsArr);
        } else {
            auto sub = type.rankDec();
            auto arr = (jarray) (v.l);
            auto len = env->GetArrayLength(arr);
            se::Object *jsArr = se::Object::createArrayObject(len);
            for (auto i = 0; i < len; i++) {
                auto ele = env->GetObjectArrayElement((jobjectArray) arr, i);
                jvalue t;
                t.l = ele;
                se::Value jtmp;
                if (!jobject_to_seval(env, sub, t, jtmp)) {
                    return false;
                }
                jsArr->setArrayElement(i, jtmp);
            }
            out.setObject(jsArr);
        }
        return true;
    }


    bool getJobjectStaticFieldByType(JNIEnv *env, jclass kls, jfieldID field,
                                     const jni_utils::JniType &type, se::Value &out) {
        if (type.dim == 0) {
            if (type.isBoolean()) {
                auto v = env->GetStaticBooleanField(kls, field);
                out.setBoolean(v);
            } else if (type.isChar()) {
                auto v = env->GetStaticCharField(kls, field);
                out.setUint16(v);
            } else if (type.isShort()) {
                auto v = env->GetStaticShortField(kls, field);
                out.setInt16(v);
            } else if (type.isByte()) {
                auto v = env->GetStaticByteField(kls, field);
                out.setUint8(v);
            } else if (type.isInt()) {
                auto v = env->GetStaticIntField(kls, field);
                out.setInt32(v);
            } else if (type.isLong()) {
                auto v = env->GetStaticLongField(kls, field);
                out.setLong(v);
            } else if (type.isFloat()) {
                auto v = env->GetStaticFloatField(kls, field);
                out.setFloat(v);
            } else if (type.isDouble()) {
                auto v = env->GetStaticDoubleField(kls, field);
                out.setNumber(v);
            } else if (type.isObject()) {
                auto v = env->GetStaticObjectField(kls, field);
                if (env->IsInstanceOf(v, env->FindClass("java/lang/String"))) {
                    out.setString(jstringToString(v));
                } else {
                    auto *w = new JObjectWrapper(v);
                    out.setObject(w->asJSObject());
                }
            } else {
                assert(false);
            }
        } else {
            jobject arr = env->GetStaticObjectField(kls, field);
            jvalue t;
            t.l = arr;
            return jobject_to_seval(env, type, t, out);
        }
        return true;
    }

    bool setJobjectStaticFieldByType(JNIEnv *env, jclass kls, jfieldID field,
                                     const jni_utils::JniType &type, const se::Value &in) {
        if (type.dim == 0) {
            if (type.isBoolean() && in.isBoolean()) {
                env->SetStaticBooleanField(kls, field, (jboolean) in.toBoolean());
            } else if (type.isChar() && in.isNumber()) {
                env->SetStaticCharField(kls, field, (jchar) in.toUint16());
            } else if (type.isShort() && in.isNumber()) {
                env->SetStaticShortField(kls, field, (jshort) in.toInt16());
            } else if (type.isByte() && in.isNumber()) {
                env->SetStaticByteField(kls, field, (jbyte) in.toInt8());
            } else if (type.isInt() && in.isNumber()) {
                env->SetStaticIntField(kls, field, (jint) in.toInt32());
            } else if (type.isLong() && in.isNumber()) {
                env->SetStaticLongField(kls, field, (jlong) in.toNumber());
            } else if (type.isFloat() && in.isNumber()) {
                env->SetStaticFloatField(kls, field, (jfloat) in.toFloat());
            } else if (type.isDouble() && in.isNumber()) {
                env->SetStaticDoubleField(kls, field, (jdouble) in.toNumber());
            } else if (type.isObject()) {
                if (in.isString()) {
                    jobject str = env->NewStringUTF(in.toString().c_str());
                    env->SetStaticObjectField(kls, field, str);
                    env->DeleteLocalRef(str);
                } else {
                    if (!in.isObject()) {
                        return false;
                    }
                    se::Object *jsObj = in.toObject();
                    JObjectWrapper *jo = (JObjectWrapper *) jsObj->getPrivateData();
                    if (jo == nullptr) {
                        return false;
                    }
                    env->SetStaticObjectField(kls, field, jo->getJavaObject());
                }
            } else {
                assert(false);
            }
        } else {
            if (!in.isObject() || in.toObject()->isArray()) {
                return false;
            }
            bool ok = false;
            jvalue jobj = seval_to_jvalule(env, type, in, ok);
            if (!ok) {
                return false;
            }
            env->SetStaticObjectField(kls, field, jobj.l);
            if (env->ExceptionCheck()) {
                env->ExceptionClear();
                return false;
            }
        }
        return true;
    }

    bool hasStaticMethod(const std::string &longPath) {
        JNIEnv *env = JniHelper::getEnv();
        bool ok = false;
        std::string path = std::regex_replace(longPath, std::regex("\\."), "/");
        std::string fieldName;
        std::string className;
        jni_utils::JniType fieldType;
        {
            auto idx = path.rfind("/");
            if (idx == std::string::npos) {
                return false;
            }
            fieldName = path.substr(idx + 1);
            className = path.substr(0, idx);
        }

        jclass kls = env->FindClass(className.c_str());
        if (kls == nullptr || env->ExceptionCheck()) {
            env->ExceptionClear();
            return false;
        }
        std::vector<JMethod> methods = getJStaticMethodsByName(env, kls, fieldName);

        return !methods.empty();
    }


    bool callStaticJMethod(const std::string &longPath, const std::vector<se::Value> &args,
                           se::Value &out) {
        JNIEnv *env = JniHelper::getEnv();
        bool ok = false;
        std::string path = std::regex_replace(longPath, std::regex("\\."), "/");
        std::string fieldName;
        std::string className;
        jni_utils::JniType fieldType;
        {
            auto idx = path.rfind("/");
            if (idx == std::string::npos) {
                return false;
            }
            fieldName = path.substr(idx + 1);
            className = path.substr(0, idx);
        }

        jclass kls = env->FindClass(className.c_str());
        if (kls == nullptr || env->ExceptionCheck()) {
            env->ExceptionClear();
            return false;
        }
        std::vector<JMethod> methods = getJStaticMethodsByName(env, kls, fieldName);

        for (JMethod &m : methods) {
            std::vector<jvalue> newArgs = convertFuncArgs(env, m.signature, args, 0, ok);
            if (ok) {
                auto rType = jni_utils::JniType::fromString(
                        m.signature.substr(m.signature.rfind(")") + 1));
                ok = callStaticMethodByType(rType, kls, m.method, newArgs, out);
                if (ok) {
                    return true;
                }
            }
        }
        return false;
    }

    void installJavaClass(const std::string &javaClass, se::Class *seClass) {
        sJClassToJSClass[javaClass] = seClass;
    }

    se::Class *findJSClass(const std::string &javaClass) {
        auto itr = sJClassToJSClass.find(javaClass);
        if (itr == sJClassToJSClass.end()) {
            return nullptr;
        }
        return itr->second;
    }
} // namespace

SE_DECLARE_FUNC(js_jni_proxy_methods);

static bool js_jni_jobject_finalize(se::State &s) {
    auto *cobj = (JObjectWrapper *) s.nativeThisObject();
    delete cobj;
    return true;
}

SE_BIND_FINALIZE_FUNC(js_jni_jobject_finalize)

static bool js_register_jni_jobject(se::Object *obj) {
    auto cls = se::Class::create("jobject", obj, nullptr, nullptr);
    cls->defineFinalizeFunction(_SE(js_jni_jobject_finalize));
    __jsb_jni_jobject = cls;
    return true;
}

static bool js_jni_jmethod_invoke_instance_finalize(se::State &s) {
    auto *cobj = (InvokeMethods *) s.nativeThisObject();
    delete cobj;
    return true;
}

SE_BIND_FINALIZE_FUNC(js_jni_jmethod_invoke_instance_finalize)

static bool js_register_jni_jmethod_invoke_instance(se::Object *obj) {
    auto cls = se::Class::create("jmethod_invoke_instance", obj, nullptr, nullptr);
    cls->defineFinalizeFunction(_SE(js_jni_jmethod_invoke_instance_finalize));
    __jsb_jni_jmethod_invoke_instance = cls;
    return true;
}


static bool js_jni_helper_getActivity(se::State &s) {
    auto *env = JniHelper::getEnv();
    jobject activity = JniHelper::getActivity();
    auto *obj = new JObjectWrapper(JniHelper::getActivity());
    s.rval().setObject(obj->asJSObject());
    return true;
}

SE_BIND_FUNC(js_jni_helper_getActivity)


static bool js_jni_helper_setClassLoaderFrom(se::State &s) {
    const int argCnt = s.args().size();
    if (argCnt == 1) {
        SE_PRECONDITION2(s.args()[0].isObject(), false, "jobject expected!");
        auto *obj = s.args()[0].toObject();
        auto *jobj = (JObjectWrapper *) obj->getPrivateData();
        JniHelper::setClassLoaderFrom(jobj->getJavaObject());
        return true;
    }
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d",
                    (int) argCnt, 1);
    return false;
}

SE_BIND_FUNC(js_jni_helper_setClassLoaderFrom)

// arguments:
// - class
// - field name
// - value
static bool js_jni_helper_setStaticField(se::State &s) {
    const int argCnt = s.args().size();
    jobject ret = nullptr;
    bool ok;
    JniMethodInfo methodInfo;
    if (argCnt < 3) {
        SE_REPORT_ERROR("wrong number of arguments: %d, 3 expected", (int) argCnt);
        return false;
    }
    assert(s.args()[0].isString());
    assert(s.args()[1].isString());

    std::string classPath = std::regex_replace(s.args()[0].toString(), std::regex("\\."), "/");
    std::string fieldName = s.args()[1].toString();

    if (setJobjectStaticField(classPath + "/" + fieldName, s.args()[2])) {
        return true;
    }
    se::ScriptEngine::getInstance()->throwException("failed to set " + classPath + "." + fieldName);
    return false;
}

SE_BIND_FUNC(js_jni_helper_setStaticField)

static bool js_jni_helper_newObject(se::State &s) {
    const int argCnt = s.args().size();
    std::string signature;
    jobject ret = nullptr;
    bool ok;

    JniMethodInfo methodInfo;
    if (argCnt == 0) {
        SE_REPORT_ERROR("wrong number of arguments: %d", (int) argCnt);
        return false;
    } else if (argCnt == 1) {
        // no arguments
        auto klassPath = s.args()[0].toString();
        signature = "()V";
        if (JniHelper::getMethodInfo(methodInfo, klassPath.c_str(), "<init>",
                                     signature.c_str())) {
            ret = methodInfo.env->NewObject(methodInfo.classID, methodInfo.methodID);
        } else {
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
        std::vector<jvalue> jvalueArray =
                convertFuncArgs(JniHelper::getEnv(), signature, s.args(), 2, ok);

        if (JniHelper::getMethodInfo(methodInfo, klassPath.c_str(), "<init>",
                                     signature.c_str())) {
            ret = methodInfo.env->NewObjectA(methodInfo.classID, methodInfo.methodID,
                                             jvalueArray.data());
        } else {
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
    if (!obj->getProperty("helper", &helperVal)) {
        se::HandleObject jsobj(se::Object::createPlainObject());
        helperVal.setObject(jsobj);
        obj->setProperty("helper", helperVal);
    }
    auto *helperObj = helperVal.toObject();
    helperObj->defineFunction("getActivity", _SE(js_jni_helper_getActivity));
    helperObj->defineFunction("setClassLoaderFrom",
                              _SE(js_jni_helper_setClassLoaderFrom));
    helperObj->defineFunction("newObject", _SE(js_jni_helper_newObject));
    helperObj->defineFunction("setStaticField", _SE(js_jni_helper_setStaticField));
    return true;
}

static bool js_jni_proxy_apply(se::State &s) { return true; }

SE_BIND_FUNC(js_jni_proxy_apply)

static bool js_jni_proxy_get(se::State &s) {
    int argCnt = s.args().size();
    if (argCnt < 2) {
        SE_REPORT_ERROR("wrong number of arguments: %d, 2+ expected", (int) argCnt);
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

    if (method == "$methods") {
        se::Object *funcObj = sJavaObjectMethodsUnbound->bindThis(target);
        s.rval().setObject(funcObj);
        return true;
    } else if (method == "$fields") {
        se::Object *funcObj = sJavaObjectFieldsUnbound->bindThis(target);
        s.rval().setObject(funcObj);
        return true;
    }

    se::Value outvalue;

    auto *inner = (JObjectWrapper *) target->getPrivateData();
    if (inner) {

        if (inner->getUnderlineJSObject()->getRealNamedProperty(method.c_str(), &outvalue)) {
            s.rval() = outvalue;
            return true;
        }

        {
            JValueWrapper *field = inner->getFieldValue(method);
            if (field) {
                field->cast(outvalue);
                s.rval() = outvalue;
                return true;
            }
        }
        {
            std::vector<JMethod> methods;
            if (inner->findMethods(method, "", methods) && !methods.empty()) {
                auto *ctx = new InvokeMethods(method, methods, inner);
                s.rval().setObject(sJavaObjectMethodApplyUnbound->bindThis(ctx->asJSObject()));
                return true;
            }
        }

    }

    SE_LOGE("try to find field '%s'", method.c_str());

    return true;
}

SE_BIND_FUNC(js_jni_proxy_get)

static bool js_jni_proxy_set(se::State &s) {
    int argCnt = s.args().size();
    if (argCnt < 3) {
        SE_REPORT_ERROR("wrong number of arguments: %d, 3+ expected", (int) argCnt);
        return false;
    }
    assert(s.args()[0].isObject());
    assert(s.args()[1].isString());

    auto *target = s.args()[0].toObject();
    auto prop = s.args()[1].toString();
    s.rval().setBoolean(true); //always return true

    auto *inner = (JObjectWrapper *) target->getPrivateData();
    if (inner) {
        bool fieldFound = false;
        auto ok = setJobjectField(inner->getJavaObject(), prop, s.args()[2], fieldFound);
        if (fieldFound) {
            if (!ok) {
                se::ScriptEngine::getInstance()->throwException(
                        "failed to set field '" + prop + "'");
                return false;
            }
            return true;
        }
        inner->getUnderlineJSObject()->setProperty(prop.c_str(), s.args()[2]);
    }
    return true;
}

SE_BIND_FUNC(js_jni_proxy_set)

// query public methods of instance
static bool js_jni_proxy_methods(se::State &s) {
    auto *env = JniHelper::getEnv();
    int argCnt = s.args().size();
    //    if(argCnt < 1) {
    //        SE_REPORT_ERROR("wrong number of arguments: %d, 2+ expected",
    //        (int)argCnt); return false;
    //    }
    auto *jsThis = s.getJSThisObject();
    assert(jsThis);
    auto *jobjWrapper = (JObjectWrapper *) jsThis->getPrivateData();
    jobject jobj = jobjWrapper->getJavaObject();
    auto methodNames = getJobjectMethods(jobj);
    auto *array = se::Object::createArrayObject(methodNames.size());
    for (int i = 0; i < methodNames.size(); i++) {
        array->setArrayElement(i, se::Value(methodNames[i]));
    }
    s.rval().setObject(array);
    return true;
}

SE_BIND_FUNC(js_jni_proxy_methods)

// query fields of instance
static bool js_jni_proxy_fields(se::State &s) {
    auto *env = JniHelper::getEnv();
    int argCnt = s.args().size();
    //    if(argCnt < 1) {
    //        SE_REPORT_ERROR("wrong number of arguments: %d, 2+ expected",
    //        (int)argCnt); return false;
    //    }
    auto *jsThis = s.getJSThisObject();
    assert(jsThis);
    auto *jobjWrapper = (JObjectWrapper *) jsThis->getPrivateData();
    jobject jobj = jobjWrapper->getJavaObject();
    auto fieldNames = getJobjectFields(jobj);
    auto *array = se::Object::createArrayObject(fieldNames.size());
    for (int i = 0; i < fieldNames.size(); i++) {
        array->setArrayElement(i, se::Value(fieldNames[i]));
    }
    s.rval().setObject(array);
    return true;
}

SE_BIND_FUNC(js_jni_proxy_fields)


// query fields of instance

static bool js_jni_proxy_static_method_apply(se::State &s) {
    auto *env = JniHelper::getEnv();
    auto *self = s.getJSThisObject();
    auto argCnt = s.args().size();
    bool ok = false;

    auto *ctx = (JPathWrapper *) self->getPrivateData();

    if (ctx) {
        se::Object *underline = ctx->getUnderlineJSObject();
        se::Value path;
        underline->getProperty(JS_JNI_TAG_PATH, &path);
        std::string pathStr = path.toString();

        if (callStaticJMethod(pathStr, s.args(), s.rval())) {
            return true;
        }

        std::string message = "call '" + pathStr + "' failed!";
        se::ScriptEngine::getInstance()->throwException(message);
    }

    return false;
}

SE_BIND_FUNC(js_jni_proxy_static_method_apply)

static bool js_jni_proxy_object_method_apply(se::State &s) {
    auto *env = JniHelper::getEnv();
    auto *self = s.getJSThisObject();
    auto argCnt = s.args().size();
    bool ok = false;

    auto *ctx = (InvokeMethods *) self->getPrivateData();
    auto jthis = ctx->self->getJavaObject();
    auto throwException = "method '" + ctx->methodName + "' is not found, or signature mismatch!";
    if (ctx->methods.size() == 1) {
        auto &m = ctx->methods[0];
        std::vector<jvalue> jvalueArray = convertFuncArgs(env, m.signature, s.args(), 0, ok);
        if (!ok) {
            se::ScriptEngine::getInstance()->throwException(throwException);
            return false;
        }
        std::string returnType = m.signature.substr(m.signature.find(')') + 1);
        jni_utils::JniType rType = jni_utils::JniType::fromString(returnType);
        ok = callJMethodByReturnType(rType, jthis, m.method, jvalueArray, s.rval());
        return ok;
    } else {
        //TODO: match signature by type test, should not try all methods
        for (const JMethod &m : ctx->methods) {
            auto argsFromSignature = jni_utils::exactArgsFromSignature(m.signature, ok);
            if (argsFromSignature.size() == argCnt) {
                std::vector<jvalue> jvalueArray = convertFuncArgs(env, m.signature, s.args(), 0,
                                                                  ok);
                if (!ok) continue;
                std::string returnType = m.signature.substr(m.signature.find(')') + 1);
                jni_utils::JniType rType = jni_utils::JniType::fromString(returnType);
                ok = callJMethodByReturnType(rType, jthis, m.method, jvalueArray, s.rval());
                if (!ok || env->ExceptionCheck()) {
                    env->ExceptionClear();
                    continue;
                }
                return true;
            }
        }

    }
    se::ScriptEngine::getInstance()->throwException(throwException);
    return false;
}

SE_BIND_FUNC(js_jni_proxy_object_method_apply)


// base function of jpath
static bool js_jni_proxy_java_path_base(se::State &s) {
    return true;
}

SE_BIND_FUNC(js_jni_proxy_java_path_base)

// construct class / or inner class
static bool js_jni_proxy_java_path_construct(se::State &s) {
    auto *env = JniHelper::getEnv();
    auto argCnt = s.args().size();
    if (argCnt < 2) {
        SE_REPORT_ERROR("wrong number of arguments: %d, 2+ expected", (int) argCnt);
        return false;
    }
    assert(s.args()[0].isObject());
    assert(s.args()[1].isObject());

    auto *target = s.args()[0].toObject();
    auto *args = s.args()[1].toObject(); // Array

    bool ok = false;

    auto *ctx = (JPathWrapper *) target->getPrivateData();
    if (ctx) {
        se::Value path;
        se::Object *underline = ctx->getUnderlineJSObject();
        underline->getProperty(JS_JNI_TAG_PATH, &path);
        std::string pathStr = path.toString();
        jobject ret = constructByClassPath(pathStr, args);
        if (ret) {
            auto *jobjWrapper = new JObjectWrapper(ret);
            s.rval().setObject(jobjWrapper->asJSObject());
            return true;
        }
        se::ScriptEngine::getInstance()->throwException(
                "constructor of " + pathStr + " not found or parameters mismatch!");
    }
    auto *sobj = se::Object::createPlainObject();
    s.rval().setObject(sobj);
    return true;
}

SE_BIND_FUNC(js_jni_proxy_java_path_construct)

//// query fields of instance
static bool js_jni_proxy_java_path_set(se::State &s) {
    auto *env = JniHelper::getEnv();
    auto argCnt = s.args().size();
    if (argCnt < 3) {
        SE_REPORT_ERROR("wrong number of arguments: %d, 3+ expected", (int) argCnt);
        return false;
    }
    assert(s.args()[0].isObject());
    assert(s.args()[1].isString());

    auto *target = s.args()[0].toObject();
    auto field = s.args()[1].toString();

    bool ok = false;

    s.rval().setBoolean(true);

    auto *ctx = (JPathWrapper *) target->getPrivateData();
    if (ctx) {
        se::Value path;
        se::Object *underline = ctx->getUnderlineJSObject();
        underline->getProperty(JS_JNI_TAG_PATH, &path);
        if(path.isString()) {
            std::string pathStr = path.toString();
            std::string newpath = pathStr.empty() ? field : pathStr + "." + field;

            se::Value fieldValue;
            if (hasStaticField(newpath)) {
                if(!setJobjectStaticField(newpath, s.args()[2])) {
                    se::ScriptEngine::getInstance()->throwException("can not set static property");
                    return false;
                }
                return true;
            }
        }

        underline->setProperty(field.c_str(), s.args()[2]);
        return true;
    }

    return false;
}

SE_BIND_FUNC(js_jni_proxy_java_path_set)

// query fields of instance
static bool js_jni_proxy_java_path_get(se::State &s) {
    auto *env = JniHelper::getEnv();
    auto argCnt = s.args().size();
    if (argCnt < 2) {
        SE_REPORT_ERROR("wrong number of arguments: %d, 2+ expected", (int) argCnt);
        return false;
    }
    assert(s.args()[0].isObject());
    assert(s.args()[1].isString());

    auto *target = s.args()[0].toObject();
    auto field = s.args()[1].toString();

    bool ok = false;

    auto *ctx = (JPathWrapper *) target->getPrivateData();
    if (ctx) {
        se::Value path;
        se::Object *underline = ctx->getUnderlineJSObject();

        if (underline->getRealNamedProperty(field.c_str(), &path)) {
            s.rval() = path;
            return true;
        }
        if (field[0] != '_') {
            // field = field.substr(1);
            underline->getProperty(JS_JNI_TAG_PATH, &path);
            std::string pathStr = path.toString();
            std::string newpath = pathStr.empty() ? field : pathStr + "." + field;

            se::Value fieldValue;
            if (getJobjectStaticField(newpath, fieldValue)) {
                s.rval() = fieldValue;
                return true;
            }

            auto *newctx = new JPathWrapper(newpath);
            if (hasStaticMethod(newpath)) {
                se::Object *func = sJavaStaticMethodApplyUnbound->bindThis(
                        newctx->getUnderlineJSObject());
                s.rval().setObject(func);
                return true;
            }

            s.rval().setObject(newctx->asJSObject());
            return true;
        }
    }
    return false;
}

SE_BIND_FUNC(js_jni_proxy_java_path_get)


static void setup_proxy_object() {
    sProxyObject = se::Object::createPlainObject();
    sProxyObject->defineFunction("apply", _SE(js_jni_proxy_apply));
    sProxyObject->defineFunction("get", _SE(js_jni_proxy_get));
    sProxyObject->defineFunction("set", _SE(js_jni_proxy_set));
    //    sProxyObject->defineFunction("_methods", _SE(js_jni_proxy_methods));
    sProxyObject->root(); // unroot somewhere

    sJavaObjectMethodsUnbound =
            se::Object::createFunctionObject(nullptr, _SE(js_jni_proxy_methods));
    sJavaObjectMethodsUnbound->root();

    sJavaObjectFieldsUnbound =
            se::Object::createFunctionObject(nullptr, _SE(js_jni_proxy_fields));
    sJavaObjectFieldsUnbound->root();

    sPathProxObject = se::Object::createPlainObject();
//    sPathProxObject->defineFunction("apply", _SE(js_jni_proxy_java_path_apply));
    sPathProxObject->defineFunction("construct", _SE(js_jni_proxy_java_path_construct));
    sPathProxObject->defineFunction("get", _SE(js_jni_proxy_java_path_get));
    sPathProxObject->defineFunction("set", _SE(js_jni_proxy_java_path_set));
    sPathProxObject->root();

    sJavaObjectMethodApplyUnbound = se::Object::createFunctionObject(nullptr,
                                                                     _SE(js_jni_proxy_object_method_apply));
    sJavaObjectMethodApplyUnbound->root();

    sJavaStaticMethodApplyUnbound = se::Object::createFunctionObject(nullptr,
                                                                     _SE(js_jni_proxy_static_method_apply));
    sJavaStaticMethodApplyUnbound->root();
}

bool jsb_register_jni_manual(se::Object *obj) {
    setup_proxy_object();
    se::Value nsVal;
    if (!obj->getProperty("jni", &nsVal)) {
        se::HandleObject jsobj(se::Object::createPlainObject());
        nsVal.setObject(jsobj);
        obj->setProperty("jni", nsVal);
    }
    auto *ns = nsVal.toObject();
    js_register_jni_jobject(ns);
    js_register_jni_helper(ns);
    js_register_jni_jmethod_invoke_instance(ns);

    {
        // java object in global object
        auto *p = new JPathWrapper();
        obj->setProperty("java", se::Value(p->asJSObject()));
    }
    return true;
}
