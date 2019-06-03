/****************************************************************************
 Copyright (c) 2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#pragma once

#include "base/ccMacros.h"
#include "base/CCData.h"
#include "math/CCGeometry.h"
#include "CCGL.h"

#include <string>
#include <vector>


#ifndef OBJC_CLASS
#ifdef __OBJC__
#define OBJC_CLASS(name) @class name
#else
#define OBJC_CLASS(name) class name
#endif
#endif // OBJC_CLASS

OBJC_CLASS(CanvasRenderingContext2DImpl);

NS_CC_BEGIN

class CC_DLL CanvasGradient {
public:
    CanvasGradient(float x0, float y0, float x1, float y1, float r0, float r1);

    ~CanvasGradient();

    void addColorStop(float offset, const std::string &color);

public:
    std::vector<float> pos;
    std::vector<std::string> colors;
    float x0, y0, x1, y1, radius0, radius1;
};

class CC_DLL CanvasPattern {
public:
    CanvasPattern(int width, int height, Data &pixels, std::string &rule);

    ~CanvasPattern();

public:
    int width, height;
    Data data;
    std::string rule;
};

class CC_DLL CanvasRenderingContext2D
{
public:
    CanvasRenderingContext2D(float width, float height);
    ~CanvasRenderingContext2D();

    // Rect
    void rect(float x, float y, float width, float height);
    void clearRect(float x, float y, float width, float height);
    void fillRect(float x, float y, float width, float height);
    void strokeRect(float x, float y, float width, float height);

    void fillText(const std::string& text, float x, float y, float maxWidth = -1.0f);
    void strokeText(const std::string& text, float x, float y, float maxWidth = -1.0f);
    Size measureText(const std::string& text);
    // Paths
    void beginPath();
    void closePath();
    void moveTo(float x, float y);
    void lineTo(float x, float y);
    void quadraticCurveTo(float x1, float y1, float x2, float y2);
    void bezierCurveTo(float x1, float y1, float x2, float y2, float x3, float y3);
    void arc(float x1, float y1, float radius, float startAngle, float endAngle, bool anticlockwise = false);
    void arcTo(float x1, float y1, float x2, float y2, float radius);
    void fill();
    void stroke();
    void ellipse(float x, float y, float radiusX, float radiusY, float rotation, float startAngle, float endAngle, bool antiClockWise = false);
    void clip(std::string rule = "nonzero");

    // state
    void save();
    void restore();

    // drawing images
    void drawImage(const Data& image, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh,
                   float ow, float oh);

    // functions for properties
    void set__width(float width);
    void set__height(float height);
    void set_lineWidthInternal(float lineWidth);
    void set_lineJoin(const std::string& lineJoin);
    void set_lineCap(const std::string& lineCap);
    void set_font(const std::string& font);
    void set_textAlign(const std::string& textAlign);
    void set_textBaseline(const std::string& textBaseline);
    void set_fillStyle(const std::string& fillStyle);
    void set_fillStyle(CanvasGradient* gradient);
    void set_fillStyle(CanvasPattern* pattern);
    void set_strokeStyle(const std::string& strokeStyle);
    void set_strokeStyle(CanvasGradient* gradient);
    void set_strokeStyle(CanvasPattern* pattern);
    void set_globalCompositeOperation(const std::string& globalCompositeOperation);
    void set_globalAlphaInternal(float alpha);
    void set_lineDashOffsetInternal(float offset);
    void set_miterLimitInternal(float limit);
    void set_shadowColor(const std::string& shadowColor);
    void set_shadowBlurInternal(float blur);
    void set_shadowOffsetXInternal(float offsetX);
    void set_shadowOffsetYInternal(float offsetY);

    // fill image data into Context2D
    void _fillImageData(const Data& imageData, float imageWidth, float imageHeight, float offsetX, float offsetY);

    // transform
    void translate(float x, float y);
    void scale(float x, float y);
    void rotate(float angle);

    void transform(float a, float b, float c, float d, float e, float f);
    void setTransform(float a, float b, float c, float d, float e, float f);
    void resetTransform();
    void setLineDash(std::vector<float>& arr);
    std::vector<float>& getLineDash();

    using CanvasBufferGetCallback = std::function<void(void* p, int len, bool needPremultiply)>;
    void _getData(CanvasBufferGetCallback& callback);

private:
    GLint _maxTextureSize;
    bool recreateBufferIfNeeded();
    void resetFillStyle();
    void resetStrokeStyle();

public:

    float __width = 0.0f;
    float __height = 0.0f;

    // Line styles
    float _lineWidthInternal = 1.0f;
    float _lineDashOffsetInternal = 0.0f;
    float _miterLimitInternal = 10.0f;
    std::string _lineJoin = "miter";
    std::string _lineCap = "butt";

    // Text styles
    std::string _font = "10px sans-serif";
    std::string _textAlign = "start";
    std::string _textBaseline = "alphabetic";

    // Fill and stroke styles
    std::string _fillStyle = "#000";
    std::string _strokeStyle = "#000";

    //shadow
    std::string _shadowColor = "#000";
    float _shadowBlurInternal = 0.0f;
    float _shadowOffsetXInternal = 0.0f;
    float _shadowOffsetYInternal = 0.0f;

    // Compositing
    std::string _globalCompositeOperation = "source-over";
    float _globalAlphaInternal = 1.0f;
    CanvasGradient* _gradientFillStyle = nullptr;
    CanvasGradient* _gradientStrokeStyle = nullptr;
    CanvasPattern* _patternFillStyle = nullptr;
    CanvasPattern* _patternStrokeStyle = nullptr;
    static bool s_needPremultiply;

private:
    CanvasRenderingContext2DImpl* _impl = nullptr;

    bool _isBufferSizeDirty = true;
};

NS_CC_END
