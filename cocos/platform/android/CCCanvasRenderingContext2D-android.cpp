#include "platform/CCCanvasRenderingContext2D.h"
#include "base/ccTypes.h"
#include "base/csscolorparser.hpp"
#include "base/ccUTF8.h"

#include "cocos/scripting/js-bindings/jswrapper/SeApi.h"
#include "platform/android/jni/JniHelper.h"

#include <regex>
#include <map>
#include <android/bitmap.h>

#ifndef JCLS_CANVASIMPL
#define JCLS_CANVASIMPL  "org/cocos2dx/lib/CanvasRenderingContext2DImpl"
#endif

using namespace cocos2d;
using namespace CSSColorParser;

bool CanvasRenderingContext2D::s_needPremultiply = false;

enum class CanvasTextAlign {
    LEFT,
    CENTER,
    RIGHT
};

enum class CanvasTextBaseline {
    TOP,
    MIDDLE,
    BOTTOM
};

static std::map<std::string, bool> s_globalCompositeOperationMap;

class CanvasRenderingContext2DImpl
{
public:
    CanvasRenderingContext2DImpl()
    {
        jobject obj = JniHelper::newObject(JCLS_CANVASIMPL);
        _obj = JniHelper::getEnv()->NewGlobalRef(obj);
        JniHelper::getEnv()->DeleteLocalRef(obj);
    }

    ~CanvasRenderingContext2DImpl()
    {
        JniHelper::getEnv()->DeleteGlobalRef(_obj);
        _obj = nullptr;
    }

    void recreateBuffer(float w, float h)
    {
        _bufferWidth = w;
        _bufferHeight = h;
        if (_bufferWidth < 1.0f || _bufferHeight < 1.0f)
            return;
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "recreateBuffer", w, h);
    }

    void beginPath()
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "beginPath");
    }

    void closePath()
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "closePath");
    }

    void moveTo(float x, float y)
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "moveTo", x, y);
    }

    void lineTo(float x, float y)
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "lineTo", x, y);
    }

    void quadraticCurveTo(float x1, float y1, float x2, float y2)
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "quadraticCurveTo", x1, y1, x2, y2);
    }

    void bezierCurveTo(float x1, float y1, float x2, float y2, float x3, float y3)
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "bezierCurveTo", x1, y1, x2, y2, x3, y3);
    }

    void arc(float x1, float y1, float radius, float startAngle, float endAngle, bool anticlockwise)
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "arc", x1, y1, radius, startAngle, endAngle, anticlockwise);
    }

    void arcTo(float x1, float y1, float x2, float y2, float radius)
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "arcTo", x1, y1, x2, y2, radius);
    }

    void stroke()
    {
        if (_bufferWidth < 1.0f || _bufferHeight < 1.0f)
            return;

        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "stroke");
    }

    void fill()
    {
        if (_bufferWidth < 1.0f || _bufferHeight < 1.0f)
            return;

        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "fill");
    }

    void saveContext()
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "saveContext");
    }

    void restoreContext()
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "restoreContext");
    }

    void rect(float x, float y, float w, float h)
    {
        if (_bufferWidth < 1.0f || _bufferHeight < 1.0f)
            return;
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "rect", x, y, w, h);
    }

    void strokeRect(float x, float y, float w, float h)
    {
        if (_bufferWidth < 1.0f || _bufferHeight < 1.0f) {
            return;
        }
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "rect", x, y, w, h);
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "stroke");
    }

    void clearRect(float x, float y, float w, float h)
    {
        if (_bufferWidth < 1.0f || _bufferHeight < 1.0f)
            return;
        if (x >= _bufferWidth || y >= _bufferHeight)
            return;
        if (x + w > _bufferWidth)
            w = _bufferWidth - x;
        if (y + h > _bufferHeight)
            h = _bufferHeight - y;
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "clearRect", x, y, w, h);
    }

    void fillRect(float x, float y, float w, float h)
    {
        if (_bufferWidth < 1.0f || _bufferHeight < 1.0f)
            return;
        if (x >= _bufferWidth || y >= _bufferHeight)
            return;
        if (x + w > _bufferWidth)
            w = _bufferWidth - x;
        if (y + h > _bufferHeight)
            h = _bufferHeight - y;
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "fillRect", x, y, w, h);
    }

    void fillText(const std::string& text, float x, float y, float maxWidth)
    {
        if (_bufferWidth < 1.0f || _bufferHeight < 1.0f)
            return;
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "fillText", text, x, y, maxWidth);
    }

    void strokeText(const std::string& text, float x, float y, float maxWidth)
    {
        if (_bufferWidth < 1.0f || _bufferHeight < 1.0f)
            return;
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "strokeText", text, x, y, maxWidth);
    }

    float measureText(const std::string& text)
    {
        if (text.empty())
            return 0.0f;
        return JniHelper::callObjectFloatMethod(_obj, JCLS_CANVASIMPL, "measureText", text);
    }

    void updateFont(const std::string& fontName, float fontSize, bool bold, bool italic, bool oblique, bool smallCaps)
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "updateFont", fontName, fontSize, bold, italic, oblique, smallCaps);
    }

    void setLineCap(const std::string& lineCap) {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "setLineCap", lineCap);
    }

    void setLineJoin(const std::string& lineJoin) {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "setLineJoin", lineJoin);
    }

    void setTextAlign(CanvasTextAlign align)
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "setTextAlign", (int)align);
    }

    void setTextBaseline(CanvasTextBaseline baseline)
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "setTextBaseline", (int)baseline);
    }

    void setFillStyle(float r, float g, float b, float a)
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "setFillStyle", r, g, b, a);
    }

    void setStrokeStyle(float r, float g, float b, float a)
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "setStrokeStyle", r, g, b, a);
    }

    void setShadowColor(float r, float g, float b, float a)
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "setShadowColor", r, g, b, a);
    }

    void setShadowBlur(float blur)
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "setShadowBlur", blur);
    }

    void setShadowOffsetX(float offsetX)
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "setShadowOffsetX", offsetX);
    }

    void setShadowOffsetY(float offsetY)
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "setShadowOffsetY", offsetY);
    }

    void setLineWidth(float lineWidth)
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "setLineWidth", lineWidth);
    }

    void _fillImageData(const Data &imageData, float imageWidth, float imageHeight, float offsetX, float offsetY) {
        if (_bufferWidth < 1.0f || _bufferHeight < 1.0f)
            return;

        jbyteArray arr = JniHelper::getEnv()->NewByteArray(imageData.getSize());
        JniHelper::getEnv()->SetByteArrayRegion(arr, 0, imageData.getSize(),
                                                (const jbyte *) imageData.getBytes());
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "_fillImageData", arr, imageWidth,
                                        imageHeight, offsetX, offsetY);
        JniHelper::getEnv()->DeleteLocalRef(arr);
    }

    void getData(CanvasRenderingContext2D::CanvasBufferGetCallback &callback, bool needPremultiply) {
        jobject bmpObj = nullptr;
        JniMethodInfo methodInfo;
        if (JniHelper::getMethodInfo(methodInfo, JCLS_CANVASIMPL, "getBitmap", "()Landroid/graphics/Bitmap;")) {
            bmpObj = methodInfo.env->CallObjectMethod(_obj, methodInfo.methodID);
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }

        JNIEnv *env = JniHelper::getEnv();
        do {
            if (nullptr == bmpObj) {
                break;
            }
            AndroidBitmapInfo bmpInfo;
            AndroidBitmap_getInfo(env, bmpObj, &bmpInfo);
            if (AndroidBitmap_getInfo(env, bmpObj, &bmpInfo) != ANDROID_BITMAP_RESULT_SUCCESS) {
                SE_LOGE("AndroidBitmap_getInfo() failed ! error");
                break;
            }
            if (bmpInfo.width < 1 || bmpInfo.height < 1) {
                break;
            }
            int size = bmpInfo.width * bmpInfo.height * 4;

            void *pixelData;
            if (AndroidBitmap_lockPixels(env, bmpObj, &pixelData) != ANDROID_BITMAP_RESULT_SUCCESS) {
                SE_LOGE("AndroidBitmap_lockPixels() failed ! error");
                break;
            }

            if (nullptr != callback) {
                callback(pixelData, size, needPremultiply);
            }
            AndroidBitmap_unlockPixels(env, bmpObj);
        } while (false);
        if (bmpObj) {
            env->DeleteLocalRef(bmpObj);
        }
    }


    void scale(float x, float y)
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "scale", x, y);
    }

    void rotate(float angle)
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "rotate", angle);
    }

    void translate(float x, float y)
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "translate", x, y);
    }

    void transform(float a, float b, float c, float d, float e, float f)
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "transform", a, b, c, d, e, f);
    }

    void setTransform(float a, float b, float c, float d, float e, float f)
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "setTransform", a, b, c, d, e, f);
    }

    void resetTransform()
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "resetTransform");
    }

    void setLineDash(std::vector<float>& arr)
    {
        cocos2d::JniMethodInfo methodInfo;
        if (cocos2d::JniHelper::getMethodInfo(methodInfo, JCLS_CANVASIMPL, "setLineDash",
                                              "([F)V")) {
            jfloatArray jArrObj = nullptr;
            int size = arr.size();
            if(size > 0) {
                float arrValue[size];
                for(int i = 0; i < size; i++) {
                    arrValue[i] = arr[i];
                }
                jArrObj = methodInfo.env->NewFloatArray(size);
                methodInfo.env->SetFloatArrayRegion(jArrObj, 0, size, (const jfloat *)arrValue);
            }
            methodInfo.env->CallVoidMethod(_obj, methodInfo.methodID, jArrObj);
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
            if(nullptr != jArrObj) {
                methodInfo.env->DeleteLocalRef(jArrObj);
            }
        }
    }

    std::vector<float>& getLineDash()
    {
        jfloatArray array = JniHelper::callObjectFloatArrayMethod(_obj, JCLS_CANVASIMPL,
                                                                  "getLineDash");
        static std::vector<float> tmp;
        tmp.clear();
        if (nullptr == array) {
            return tmp;
        }
        JNIEnv* pEnv = JniHelper::getEnv();
        int size = pEnv->GetArrayLength(array);
        if (size < 1) {
            pEnv->DeleteLocalRef(array);
            return tmp;
        }
        float *values = new float[size];
        jfloat *elems = pEnv->GetFloatArrayElements(array, 0);
        if (elems) {
            memcpy(values, elems, sizeof(float) * size);
            pEnv->ReleaseFloatArrayElements(array, elems, 0);
        };
        pEnv->DeleteLocalRef(array);
        for (int i = 0; i < size; ++i) {
            tmp.push_back(values[i]);
        }
        return tmp;
    }

    void setLineDashOffset(float offset)
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "setLineDashOffset", offset);
    }

    void setMiterLimit(float limit)
    {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "setMiterLimit", limit);
    }

    void setGlobalCompositeOperation(const std::string& operation) {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "setGlobalCompositeOperation", operation);
    }

    void setGlobalAlpha(float alpha) {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "setGlobalAlpha", alpha);
    }

    void drawImage(const Data &image, float sx, float sy, float sw, float sh,
                   float dx, float dy, float dw, float dh, float ow, float oh) {
        if (_bufferWidth < 1.0f || _bufferHeight < 1.0f)
            return;
        jbyteArray arr = JniHelper::getEnv()->NewByteArray((jsize) image.getSize());
        unsigned char *data = image.getBytes();
        int alpha = 0;
        for (uint8_t *end = data + image.getSize(); data < end; data = data + 4) {
            alpha = data[3];
            data[0] = data[0] * alpha / 255;
            data[1] = data[1] * alpha / 255;
            data[2] = data[2] * alpha / 255;
        }
        JniHelper::getEnv()->SetByteArrayRegion(arr, 0, (jsize) image.getSize(), (const jbyte *) image.getBytes());
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "drawImage", arr, sx, sy, sw, sh, dx, dy, dw, dh, ow, oh);
        JniHelper::getEnv()->DeleteLocalRef(arr);
    }

    void ellipse(float x, float y, float radiusX, float radiusY, float rotation, float startAngle, float endAngle, bool antiClockWise) {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "ellipse", x, y, radiusX, radiusY, rotation, startAngle, endAngle, antiClockWise);
    }

    void clip(std::string rule) {
        JniHelper::callObjectVoidMethod(_obj, JCLS_CANVASIMPL, "clip", rule);
    }

    void applyStyle_Linear(bool isFillStyle, float x0, float y0, float x1, float y1, std::vector<float>& posVec, std::vector<std::string>& colorVec ) {
        cocos2d::JniMethodInfo methodInfo;
        if (cocos2d::JniHelper::getMethodInfo(methodInfo, JCLS_CANVASIMPL, "applyStyle_Linear",
                                              "(ZFFFF[F[I)V")) {
            jfloatArray jArrPos = nullptr;
            jintArray jArrColor = nullptr;


            int size = posVec.size();
            if(size < 1) {
                return;
            }
            float arrPos[size];
            int arrColor[size];
            for(int i = 0; i < size; i++) {
                arrPos[i] = posVec[i];
                Color color = parse(colorVec[i]);
                int alpha = color.a * 255;
                int red = color.r;
                int green = color.g;
                int blue = color.b;
                arrColor[i] = alpha << 24 | red << 16 | green << 8 | blue;
            }
            jArrPos = methodInfo.env->NewFloatArray(size);
            methodInfo.env->SetFloatArrayRegion(jArrPos, 0, size, (const jfloat *)arrPos);

            jArrColor = methodInfo.env->NewIntArray(size);
            methodInfo.env->SetIntArrayRegion(jArrColor, 0, size, (const jint *)arrColor);

            methodInfo.env->CallVoidMethod(_obj, methodInfo.methodID, isFillStyle, x0, y0, x1, y1, jArrPos, jArrColor);
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
            if(nullptr != jArrPos) {
                methodInfo.env->DeleteLocalRef(jArrPos);
            }
            if(nullptr != jArrColor) {
                methodInfo.env->DeleteLocalRef(jArrColor);
            }
        }
    }

    void _applyStyle_Pattern(bool isFillStyle, std::string rule, const Data& image, float width, float height) {
        cocos2d::JniMethodInfo methodInfo;
        if (cocos2d::JniHelper::getMethodInfo(methodInfo, JCLS_CANVASIMPL, "applyStyle_Pattern", "(ZLjava/lang/String;[BFF)V")) {
            int size = image.getSize();
            if (size < 1) {
                return;
            }
            jbyteArray jArrData = methodInfo.env->NewByteArray(size);
            unsigned char* data = image.getBytes();
            int alpha = 0;
            for (uint8_t *end = data + size; data < end; data = data + 4) {
                alpha = data[3];
                data[0] = data[0] * alpha / 255;
                data[1] = data[1] * alpha / 255;
                data[2] = data[2] * alpha / 255;
            }

            methodInfo.env->SetByteArrayRegion(jArrData, 0, size, (const jbyte *) image.getBytes());
            jstring jRule = cocos2d::StringUtils::newStringUTFJNI(methodInfo.env, rule);

            methodInfo.env->CallVoidMethod(_obj, methodInfo.methodID, isFillStyle, jRule, jArrData, width, height);
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
            if (nullptr != jArrData) {
                methodInfo.env->DeleteLocalRef(jArrData);
            }
            methodInfo.env->DeleteLocalRef(jRule);
         }
    }

    void _applyStyle_Radial(bool isFillStyle, float x0, float y0, float r0, float x1, float y1, float r1, std::vector<float>& posVec, std::vector<std::string>& colorVec) {
        cocos2d::JniMethodInfo methodInfo;
        if (cocos2d::JniHelper::getMethodInfo(methodInfo, JCLS_CANVASIMPL, "applyStyle_Radial",
                                              "(ZFFFFFF[F[I)V")) {
            jfloatArray jArrPos = nullptr;
            jintArray jArrColor = nullptr;


            int size = posVec.size();
            if(size < 1) {
                return;
            }
            float arrPos[size];
            int arrColor[size];
            for(int i = 0; i < size; i++) {
                arrPos[i] = posVec[i];
                Color color = parse(colorVec[i]);
                int alpha = color.a * 255;
                int red = color.r;
                int green = color.g;
                int blue = color.b;
                arrColor[i] = alpha << 24 | red << 16 | green << 8 | blue;
            }
            jArrPos = methodInfo.env->NewFloatArray(size);
            methodInfo.env->SetFloatArrayRegion(jArrPos, 0, size, (const jfloat *)arrPos);

            jArrColor = methodInfo.env->NewIntArray(size);
            methodInfo.env->SetIntArrayRegion(jArrColor, 0, size, (const jint *)arrColor);

            methodInfo.env->CallVoidMethod(_obj, methodInfo.methodID, isFillStyle, x0, y0, r0, x1, y1, r1, jArrPos, jArrColor);
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
            if(nullptr != jArrPos) {
                methodInfo.env->DeleteLocalRef(jArrPos);
            }
            if(nullptr != jArrColor) {
                methodInfo.env->DeleteLocalRef(jArrColor);
            }
        }
    }

private:
    jobject _obj = nullptr;
    Data _data;
    float _bufferWidth = 0.0f;
    float _bufferHeight = 0.0f;
};

namespace {
    void fillRectWithColor(uint8_t* buf, uint32_t totalWidth, uint32_t totalHeight, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t r, uint8_t g, uint8_t b)
    {
        assert(x + width <= totalWidth);
        assert(y + height <=  totalHeight);

        uint32_t y0 = totalHeight - (y + height);
        uint32_t y1 = totalHeight - y;
        uint8_t* p;
        for (uint32_t offsetY = y0; offsetY < y1; ++offsetY)
        {
            for (uint32_t offsetX = x; offsetX < (x + width); ++offsetX)
            {
                p = buf + (totalWidth * offsetY + offsetX) * 3;
                *p++ = r;
                *p++ = g;
                *p++ = b;
            }
        }
    }
}

NS_CC_BEGIN


CanvasGradient::CanvasGradient(float x0, float y0, float x1, float y1, float r0, float r1) {
    this->x0 = x0;
    this->x1 = x1;
    this->y0 = y0;
    this->y1 = y1;
    this->radius0 = r0;
    this->radius1 = r1;
}

CanvasGradient::~CanvasGradient() {
    //
}

void CanvasGradient::addColorStop(float offset, const std::string &color) {
    pos.push_back(offset);
    colors.push_back(color);
}

CanvasPattern::CanvasPattern(int width, int height, Data &pixels, std::string &rule) {
    this->width = width;
    this->height = height;
    this->data = pixels;
    this->rule = rule;
}

CanvasPattern::~CanvasPattern() {
    //
}
// CanvasRenderingContext2D

CanvasRenderingContext2D::CanvasRenderingContext2D(float width, float height)
: __width(width)
, __height(height)
{
    // SE_LOGD("CanvasRenderingContext2D constructor: %p, width: %f, height: %f\n", this, width, height);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &_maxTextureSize);
    _impl = new CanvasRenderingContext2DImpl();
    recreateBufferIfNeeded();

    s_globalCompositeOperationMap["source-over"] = true;
    s_globalCompositeOperationMap["source-in"] = true;
    s_globalCompositeOperationMap["source-out"] = true;
    s_globalCompositeOperationMap["source-atop"] = true;
    s_globalCompositeOperationMap["destination-over"] = true;
    s_globalCompositeOperationMap["destination-in"] = true;
    s_globalCompositeOperationMap["destination-out"] = true;
    s_globalCompositeOperationMap["destination-atop"] = true;
    s_globalCompositeOperationMap["xor"] = true;
    s_globalCompositeOperationMap["screen"] = true;
    s_globalCompositeOperationMap["overlay"] = true;
    s_globalCompositeOperationMap["darken"] = true;
    s_globalCompositeOperationMap["lighten"] = true;
}

CanvasRenderingContext2D::~CanvasRenderingContext2D()
{
//     SE_LOGD("CanvasRenderingContext2D destructor: %p\n", this);
    delete _impl;
}

void CanvasRenderingContext2D::_getData(CanvasBufferGetCallback& callback) {
    _impl->getData(callback, s_needPremultiply);
}

bool CanvasRenderingContext2D::recreateBufferIfNeeded()
{
    if (_isBufferSizeDirty)
    {
        _isBufferSizeDirty = false;
//        SE_LOGD("Recreate buffer %p, w: %f, h:%f\n", this, __width, __height);
        if (__width > _maxTextureSize || __height > _maxTextureSize) {
            return false;
        }
        _impl->recreateBuffer(__width, __height);

        //reset to initail state
        _lineWidthInternal = 1.0f;
        _lineDashOffsetInternal = 0.0f;
        _miterLimitInternal = 10.0f;
        _lineJoin = "miter";
        _lineCap = "butt";
        _font = "10px sans-serif";
        _textAlign = "start";
        _textBaseline = "alphabetic";

        _fillStyle = "#000";
        _strokeStyle = "#000";

        _shadowColor = "#000";
        _shadowBlurInternal = 0.0f;
        _shadowOffsetXInternal = 0.0f;
        _shadowOffsetYInternal = 0.0f;

        _globalCompositeOperation = "source-over";
        _globalAlphaInternal = 1.0f;
    }
    return true;
}

void CanvasRenderingContext2D::rect(float x, float y, float width, float height)
{
//    SE_LOGD("CanvasRenderingContext2D::rect: %p, %f, %f, %f, %f\n", this, x, y, width, height);
    _impl->rect(x, y, width, height);
}

void CanvasRenderingContext2D::clearRect(float x, float y, float width, float height)
{
//    SE_LOGD("CanvasRenderingContext2D::clearRect: %p, %f, %f, %f, %f\n", this, x, y, width, height);
    if (recreateBufferIfNeeded()) {
        _impl->clearRect(x, y, width, height);
    } else {
        SE_LOGE("[ERROR] CanvasRenderingContext2D clearRect width:%f, height:%f is out of GL_MAX_TEXTURE_SIZE",
                __width, __height);
    }
}

void CanvasRenderingContext2D::fillRect(float x, float y, float width, float height)
{
    if (recreateBufferIfNeeded()) {
        _impl->fillRect(x, y, width, height);
    } else {
        SE_LOGE("[ERROR] CanvasRenderingContext2D fillRect width:%f height:%f is out of GL_MAX_TEXTURE_SIZE",
                __width, __height);
    }
}

void CanvasRenderingContext2D::strokeRect(float x, float y, float width, float height)
{
    if (recreateBufferIfNeeded()) {
        _impl->strokeRect(x, y, width, height);
    } else {
        SE_LOGE("[ERROR] CanvasRenderingContext2D strokeRect width:%f, height:%f is out of GL_MAX_TEXTURE_SIZE",
                __width, __height);
    }
}

void CanvasRenderingContext2D::fillText(const std::string& text, float x, float y, float maxWidth)
{
//    SE_LOGD("CanvasRenderingContext2D::fillText: %s, %f, %f, %f\n", text.c_str(), x, y, maxWidth);
    if (recreateBufferIfNeeded()) {
        _impl->fillText(text, x, y, maxWidth);
    } else {
        SE_LOGE("[ERROR] CanvasRenderingContext2D fillRect width:%f, height:%f is out of GL_MAX_TEXTURE_SIZE",
                __width, __height);
    }
}

void CanvasRenderingContext2D::strokeText(const std::string& text, float x, float y, float maxWidth)
{
//    SE_LOGD("CanvasRenderingContext2D::strokeText: %s, %f, %f, %f\n", text.c_str(), x, y, maxWidth);
    if (recreateBufferIfNeeded()) {
        _impl->strokeText(text, x, y, maxWidth);
    } else {
        SE_LOGE("[ERROR] CanvasRenderingContext2D strokeText width:%f, height:%f is out of GL_MAX_TEXTURE_SIZE",
                __width, __height);
    }
}

cocos2d::Size CanvasRenderingContext2D::measureText(const std::string& text)
{
//    SE_LOGD("CanvasRenderingContext2D::measureText: %s\n", text.c_str());
    return cocos2d::Size(_impl->measureText(text), 0);
}

void CanvasRenderingContext2D::save()
{
    _impl->saveContext();
}

void CanvasRenderingContext2D::beginPath()
{
    _impl->beginPath();
}

void CanvasRenderingContext2D::closePath()
{
    _impl->closePath();
}

void CanvasRenderingContext2D::moveTo(float x, float y)
{
    _impl->moveTo(x, y);
}

void CanvasRenderingContext2D::lineTo(float x, float y)
{
    _impl->lineTo(x, y);
}

void CanvasRenderingContext2D::quadraticCurveTo(float x1, float y1, float x2, float y2)
{
    _impl->quadraticCurveTo(x1, y1, x2, y2);
}

void CanvasRenderingContext2D::bezierCurveTo(float x1, float y1, float x2, float y2, float x3, float y3)
{
    _impl->bezierCurveTo(x1, y1, x2, y2, x3, y3);
}

void CanvasRenderingContext2D::arc(float x1, float y1, float radius, float startAngle, float endAngle, bool anticlockwise)
{
    _impl->arc(x1, y1, radius, startAngle, endAngle, anticlockwise);
}

void CanvasRenderingContext2D::arcTo(float x1, float y1, float x2, float y2, float radius)
{
    _impl->arcTo(x1, y1, x2, y2, radius);
}

void CanvasRenderingContext2D::stroke()
{
    if (recreateBufferIfNeeded()) {
        _impl->stroke();
    } else {
        SE_LOGE("[ERROR] CanvasRenderingContext2D stroke width:%f height:%f is out of GL_MAX_TEXTURE_SIZE",
                __width, __height);
    }
}

void CanvasRenderingContext2D::fill()
{
    if (recreateBufferIfNeeded()) {
        _impl->fill();
    } else {
        SE_LOGE("[ERROR] CanvasRenderingContext2D fill width:%f height:%f is out of GL_MAX_TEXTURE_SIZE",
                __width, __height);
    }
}

void CanvasRenderingContext2D::restore()
{
    _impl->restoreContext();
}

void CanvasRenderingContext2D::set__width(float width)
{
//    SE_LOGD("CanvasRenderingContext2D::set__width: %f\n", width);
    __width = width;
    _isBufferSizeDirty = true;
    recreateBufferIfNeeded();
}

void CanvasRenderingContext2D::set__height(float height)
{
//    SE_LOGD("CanvasRenderingContext2D::set__height: %f\n", height);
    __height = height;
    _isBufferSizeDirty = true;
    recreateBufferIfNeeded();
}

void CanvasRenderingContext2D::set_lineWidthInternal(float lineWidth)
{
    _lineWidthInternal = lineWidth;
    _impl->setLineWidth(lineWidth);
}

void CanvasRenderingContext2D::set_lineJoin(const std::string& lineJoin)
{
    if (lineJoin != "bevel" && lineJoin != "round" &&lineJoin != "miter")
    {
        return;
    }
    this->_lineJoin = lineJoin;
    _impl->setLineJoin(lineJoin);
}

void CanvasRenderingContext2D::set_lineCap(const std::string& lineCap)
{
    if (lineCap != "butt" && lineCap != "round" &&lineCap != "square")
    {
        return;
    }
    this->_lineCap = lineCap;
    _impl->setLineCap(lineCap);
}

/*
 * support format e.g.: "oblique bold small-caps 18px Arial"
 *                      "italic bold small-caps 25px Arial"
 *                      "italic 25px Arial"
 * */
void CanvasRenderingContext2D::set_font(const std::string& font)
{
    this->_font = font;
    std::string fontName = "sans-serif";
    std::string fontSizeStr = "30";
    std::regex re("\\s*((\\d+)([\\.]\\d+)?)px\\s+([^\\r\\n]*)");
    std::match_results<std::string::const_iterator> results;
    if (std::regex_search(font.cbegin(), font.cend(), results, re))
    {
        fontSizeStr = results[2].str();
        // support get font name from `60px American` or `60px "American abc-abc_abc"`
        // support get font name contain space,example `times new roman`
        // if regex rule that does not conform to the rules,such as Chinese,it defaults to sans-serif
        std::match_results<std::string::const_iterator> fontResults;
        std::regex fontRe("([\\w\\s-]+|\"[\\w\\s-]+\"$)");
        if(std::regex_match(results[4].str(), fontResults, fontRe))
        {
            fontName = results[4].str();
        }
    }

    float fontSize = atof(fontSizeStr.c_str());
    bool isBold = font.find("bold", 0) != std::string::npos;
    bool isItalic = font.find("italic", 0) != std::string::npos;
    bool isSmallCaps = font.find("small-caps", 0) != std::string::npos;
    bool isOblique = font.find("oblique", 0) != std::string::npos;
    //font-style: italic, oblique, normal
    //font-weight: normal, bold
    //font-variant: normal, small-caps
    _impl->updateFont(fontName, fontSize, isBold, isItalic, isOblique, isSmallCaps);
}

void CanvasRenderingContext2D::set_textAlign(const std::string& textAlign)
{
    std::string originContent = this->_textAlign;
    this->_textAlign = textAlign;
    // SE_LOGD("CanvasRenderingContext2D::set_textAlign: %s\n", textAlign.c_str());
    if (textAlign == "left")
    {
        _impl->setTextAlign(CanvasTextAlign::LEFT);
    }
    else if (textAlign == "center" || textAlign == "middle")
    {
        _impl->setTextAlign(CanvasTextAlign::CENTER);
    }
    else if (textAlign == "right")
    {
        _impl->setTextAlign(CanvasTextAlign::RIGHT);
    }
    else
    {
        this->_textAlign = originContent;
        assert(false);
    }
}

void CanvasRenderingContext2D::set_textBaseline(const std::string& textBaseline)
{
    // SE_LOGD("CanvasRenderingContext2D::set_textBaseline: %s\n", textBaseline.c_str());
    if (textBaseline == "top")
    {
        _impl->setTextBaseline(CanvasTextBaseline::TOP);
    }
    else if (textBaseline == "middle")
    {
        _impl->setTextBaseline(CanvasTextBaseline::MIDDLE);
    }
    else if (textBaseline == "bottom" || textBaseline == "alphabetic") //REFINE:, how to deal with alphabetic, currently we handle it as bottom mode.
    {
        _impl->setTextBaseline(CanvasTextBaseline::BOTTOM);
    }
    else
    {
        assert(false);
    }
}

void CanvasRenderingContext2D::set_fillStyle(const std::string& fillStyle)
{
    resetFillStyle();
    _fillStyle = fillStyle;
    CSSColorParser::Color color = CSSColorParser::parse(fillStyle);
    _impl->setFillStyle(color.r/255.0f, color.g/255.0f, color.b/255.0f, color.a);
    // SE_LOGD("CanvasRenderingContext2D::set_fillStyle: %s, (%d, %d, %d, %f)\n", fillStyle.c_str(), color.r, color.g, color.b, color.a);
}

void CanvasRenderingContext2D::set_fillStyle(CanvasGradient* gradient) {
    resetFillStyle();
    _gradientFillStyle = gradient;
    if(gradient->radius0 < 0) {
        _impl->applyStyle_Linear(true, gradient->x0, gradient->y0, gradient->x1, gradient->y1, gradient->pos, gradient->colors);
    } else {
        _impl->_applyStyle_Radial(true,
                                  gradient->x0,
                                  gradient->y0,
                                  gradient->radius0,
                                  gradient->x1,
                                  gradient->y1,
                                  gradient->radius1,
                                  gradient->pos,
                                  gradient->colors);
    }
}

void CanvasRenderingContext2D::set_fillStyle(CanvasPattern* pattern) {
    resetFillStyle();
    _patternFillStyle = pattern;
    _impl->_applyStyle_Pattern(true, pattern->rule, pattern->data, pattern->width, pattern->height);
}

void CanvasRenderingContext2D::set_strokeStyle(const std::string& strokeStyle)
{
    resetStrokeStyle();
    _strokeStyle = strokeStyle;
    CSSColorParser::Color color = CSSColorParser::parse(strokeStyle);
    _impl->setStrokeStyle(color.r/255.0f, color.g/255.0f, color.b/255.0f, color.a);
}

void CanvasRenderingContext2D::set_strokeStyle(CanvasGradient* gradient)
{
    resetStrokeStyle();
    _gradientStrokeStyle = gradient;
    if(gradient->radius0 < 0) {
        _impl->applyStyle_Linear(false, gradient->x0, gradient->y0, gradient->x1, gradient->y1, gradient->pos, gradient->colors);
    } else {
        _impl->_applyStyle_Radial(false,
                                  gradient->x0,
                                  gradient->y0,
                                  gradient->radius0,
                                  gradient->x1,
                                  gradient->y1,
                                  gradient->radius1,
                                  gradient->pos,
                                  gradient->colors);
    }
}

void CanvasRenderingContext2D::set_strokeStyle(CanvasPattern* pattern) {
    resetStrokeStyle();
    _patternStrokeStyle = pattern;
    _impl->_applyStyle_Pattern(false, pattern->rule, pattern->data, pattern->width, pattern->height);
}

void CanvasRenderingContext2D::set_globalCompositeOperation(const std::string& globalCompositeOperation)
{
    if(s_globalCompositeOperationMap.find(globalCompositeOperation) == s_globalCompositeOperationMap.end()) {
        return;
    }
    this->_globalCompositeOperation = globalCompositeOperation;
    _impl->setGlobalCompositeOperation(globalCompositeOperation);
}

void CanvasRenderingContext2D::set_globalAlphaInternal(float alpha) {
    if(alpha < 0 || alpha > 1) {
        return;
    }
    this->_globalAlphaInternal = alpha;
    _impl->setGlobalAlpha(alpha);
}

void CanvasRenderingContext2D::set_lineDashOffsetInternal(float offset)
{
    this->_lineDashOffsetInternal = offset;
    _impl->setLineDashOffset(offset);
}

void CanvasRenderingContext2D::_fillImageData(const Data& imageData, float imageWidth, float imageHeight, float offsetX, float offsetY)
{
    _impl->_fillImageData(imageData, imageWidth, imageHeight, offsetX, offsetY);
}
// transform
//REFINE:

void CanvasRenderingContext2D::translate(float x, float y)
{
    _impl->translate(x, y);
}

void CanvasRenderingContext2D::scale(float x, float y)
{
    _impl->scale(x, y);
}

void CanvasRenderingContext2D::rotate(float angle)
{
    _impl->rotate(angle);
}

void CanvasRenderingContext2D::transform(float a, float b, float c, float d, float e, float f)
{
    _impl->transform(a, b, c, d, e, f);
}

void CanvasRenderingContext2D::setTransform(float a, float b, float c, float d, float e, float f)
{
    _impl->setTransform(a, b, c, d, e, f);
}

void CanvasRenderingContext2D::resetTransform()
{
    _impl->resetTransform();
}

void CanvasRenderingContext2D::setLineDash(std::vector<float>& arr)
{
    _impl->setLineDash(arr);
}

std::vector<float>& CanvasRenderingContext2D::getLineDash()
{
    return _impl->getLineDash();
}

void CanvasRenderingContext2D::set_miterLimitInternal(float limit)
{
    this->_miterLimitInternal = limit;
    _impl->setMiterLimit(limit);
}

void CanvasRenderingContext2D::set_shadowColor(const std::string& shadowColor)
{
    CSSColorParser::Color color = CSSColorParser::parse(shadowColor);
    _shadowColor = shadowColor;
    _impl->setShadowColor(color.r/255.0f, color.g/255.0f, color.b/255.0f, color.a);
}

void CanvasRenderingContext2D::set_shadowBlurInternal(float blur)
{
    _shadowBlurInternal = blur;
    _impl->setShadowBlur(blur);
}

void CanvasRenderingContext2D::set_shadowOffsetXInternal(float offsetX)
{
    _shadowOffsetXInternal = offsetX;
    _impl->setShadowOffsetX(offsetX);
}

void CanvasRenderingContext2D::set_shadowOffsetYInternal(float offsetY)
{
    _shadowOffsetYInternal = offsetY;
    _impl->setShadowOffsetY(offsetY);
}

void CanvasRenderingContext2D::drawImage(const Data &image, float sx, float sy, float sw, float sh,
                                         float dx, float dy, float dw, float dh, float ow, float oh) {
    _impl->drawImage(image, sx, sy, sw, sh, dx, dy, dw, dh, ow, oh);
}

void CanvasRenderingContext2D::ellipse(float x, float y, float radiusX, float radiusY, float rotation, float startAngle, float endAngle, bool antiClockWise) {
    _impl->ellipse(x, y, radiusX, radiusY, rotation, startAngle, endAngle, antiClockWise);
}

void CanvasRenderingContext2D::clip(std::string rule) {
    _impl->clip(rule);
}

void CanvasRenderingContext2D::resetFillStyle() {
    _gradientFillStyle = nullptr;
    _patternFillStyle = nullptr;
    _fillStyle = "#000";
}

void CanvasRenderingContext2D::resetStrokeStyle() {
    _gradientStrokeStyle = nullptr;
    _patternStrokeStyle = nullptr;
    _strokeStyle = "#000";
}


NS_CC_END
