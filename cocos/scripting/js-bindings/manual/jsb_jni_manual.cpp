

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


    bool findClassByPath(const std::string &path, se::Object *args) {

        JNIEnv *env = JniHelper::getEnv();
        std::stack<jclass> classStack;
        jclass prevClass = nullptr;
        jclass tmpClass = nullptr;

        std::string classPath = std::regex_replace(path, std::regex("\\."), "/");

        // test class path
        tmpClass = env->FindClass(classPath.c_str());
        if (tmpClass == nullptr || env->ExceptionCheck()) env->ExceptionClear();

        jobjectArray constructors = (jobjectArray) JniHelper::callObjectObjectMethod(tmpClass, "java/lang/Class", "getConstructors", "[Ljava/lang/relfect/Constructor;");
        int constructLen = env->GetArrayLength(constructors);
        for(int i = 0;i < constructLen; i++) {
            //TODO find by signature
        }

        return false;
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
        getJFieldByType1D(env, _javaObject, jniFieldType, fieldId, ret);
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
            _jsTarget->_setFinalizeCallback([](void *data){
               if(data) {
                   delete (JPathWrapper*) data;
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

        se::Object *getProxy() const { return _jsProxy; }
        se::Object *getUnderlineJSObject() const { return _jsTarget; }

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

    jvalue seval_to_jvalule(JNIEnv *env, const jni_utils::JniType &def,
                            const se::Value &val, bool &ok) {
        jvalue ret = {};
        ok = false;
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
                ret.l = jo->getJavaObject();
            } else {
                ok = false;
                SE_LOGE("incorrect jni type, don't know how to convert %s to java value %s",
                        val.toStringForce().c_str(), def.toString().c_str());
            }

        } else {
            SE_LOGE("incorrect jni type, don't know how to convert %s from js value %s:%s",
                    def.toString().c_str(), val.getTypeName().c_str(), val.toStringForce().c_str());
            return ret;
        }
        ok = true;
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


// query fields of instance
static bool js_jni_create_java(se::State &s) {
    auto *env = JniHelper::getEnv();
    int argCnt = s.args().size();
    if (argCnt != 0) {
        SE_REPORT_ERROR("wrong number of arguments: %d, 0 expected",
                        (int) argCnt);
        return false;
    }

    auto *p = new JPathWrapper();
    s.rval().setObject(p->asJSObject());
    return true;
}

SE_BIND_FUNC(js_jni_create_java)

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
        JPath p;
        findClassByPath(pathStr, p, args);
    }
    return false;
}

SE_BIND_FUNC(js_jni_proxy_java_path_construct)

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
            auto *newctx = new JPathWrapper(newpath);
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
//    sPathProxObject->defineFunction("apply", _SE(js_jni_proxy_apply));
    sPathProxObject->defineFunction("construct", _SE(js_jni_proxy_java_path_construct));
    sPathProxObject->defineFunction("get", _SE(js_jni_proxy_java_path_get));
//    sPathProxObject->defineFunction("set", _SE(js_jni_proxy_set));
    sPathProxObject->root();

    sJavaObjectMethodApplyUnbound = se::Object::createFunctionObject(nullptr,
                                                                     _SE(js_jni_proxy_object_method_apply));
    sJavaObjectMethodApplyUnbound->root();
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

    se::Object *functionJava = se::Object::createFunctionObject(nullptr, _SE(js_jni_create_java));
    ns->setProperty("java", se::Value(functionJava));

    return true;
}
