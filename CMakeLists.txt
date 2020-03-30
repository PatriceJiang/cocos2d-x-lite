


if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(WINDOWS TRUE)
    set(PLATFORM_FOLDER win32)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set(PLATFORM_FOLDER android)    
    set(ANDROID TRUE)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(APPLE TRUE)
    set(MACOSX TRUE)
    set(PLATFORM_FOLDER mac)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    set(APPLE TRUE)
    set(IOS TRUE)
    set(PLATFORM_FOLDER ios)
else()
    message(FATAL_ERROR "Unsupported platform, CMake will exit")
    return()
endif()

# generators that are capable of organizing into a hierarchy of folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# simplify generator condition, please use them everywhere
if(CMAKE_GENERATOR STREQUAL Xcode)
    set(XCODE TRUE)
elseif(CMAKE_GENERATOR MATCHES Visual)
    set(VS TRUE)
endif()


# set c++ standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

################################# options ############################################

if(ANDROID) 
    set(CC_USE_GLES3 OFF)
    set(CC_USE_GLES2 ON)
else()
    set(CC_USE_GLES2 OFF)
    set(CC_USE_GLES3 ON)
endif()

################################# external source code ################################

include(${CMAKE_CURRENT_LIST_DIR}/external/CMakeLists.txt)

################################# engine source code ##################################

set(CWD ${CMAKE_CURRENT_LIST_DIR})
set(COCOS2D_SOURCE_LIST)


list(APPEND COCOS2D_SOURCE_LIST
    ${CWD}/cocos/cocos2d.h
    ${CWD}/cocos/cocos2d.cpp
)

##### audio
list(APPEND COCOS2D_SOURCE_LIST
    ${CWD}/cocos/audio/AudioEngine.cpp
    ${CWD}/cocos/audio/include/AudioEngine.h
    ${CWD}/cocos/audio/include/Export.h
)
if(WINDOWS)
    list(APPEND COCOS2D_SOURCE_LIST
        ${CWD}/cocos/audio/win32/AudioCache.cpp
        ${CWD}/cocos/audio/win32/AudioCache.h
        ${CWD}/cocos/audio/win32/AudioDecoder.cpp
        ${CWD}/cocos/audio/win32/AudioDecoder.h
        ${CWD}/cocos/audio/win32/AudioDecoderManager.cpp
        ${CWD}/cocos/audio/win32/AudioDecoderManager.h
        ${CWD}/cocos/audio/win32/AudioDecoderMp3.cpp
        ${CWD}/cocos/audio/win32/AudioDecoderMp3.h
        ${CWD}/cocos/audio/win32/AudioDecoderOgg.cpp
        ${CWD}/cocos/audio/win32/AudioDecoderOgg.h
        ${CWD}/cocos/audio/win32/AudioEngine-win32.cpp
        ${CWD}/cocos/audio/win32/AudioEngine-win32.h
        ${CWD}/cocos/audio/win32/AudioMacros.h
        ${CWD}/cocos/audio/win32/AudioPlayer.cpp
        ${CWD}/cocos/audio/win32/AudioPlayer.h
    )
endif()


##### base
list(APPEND COCOS2D_SOURCE_LIST
    ${CWD}/cocos/base/base64.cpp
    ${CWD}/cocos/base/base64.h
    ${CWD}/cocos/base/CCAutoreleasePool.cpp
    ${CWD}/cocos/base/CCAutoreleasePool.h
    ${CWD}/cocos/base/ccCArray.cpp
    ${CWD}/cocos/base/ccCArray.h
    ${CWD}/cocos/base/ccConfig.h
    ${CWD}/cocos/base/CCData.cpp
    ${CWD}/cocos/base/CCData.h
    ${CWD}/cocos/base/CCLog.cpp
    ${CWD}/cocos/base/CCLog.h
    ${CWD}/cocos/base/ccMacros.h
    ${CWD}/cocos/base/CCMap.h
    ${CWD}/cocos/base/ccRandom.cpp
    ${CWD}/cocos/base/ccRandom.h
    ${CWD}/cocos/base/CCRef.cpp
    ${CWD}/cocos/base/CCRef.h
    ${CWD}/cocos/base/CCRefPtr.h
    #${CWD}/cocos/base/CCRenderTexture.cpp
    #${CWD}/cocos/base/CCRenderTexture.h
    ${CWD}/cocos/base/CCScheduler.cpp
    ${CWD}/cocos/base/CCScheduler.h
    ${CWD}/cocos/base/CCThreadPool.cpp
    ${CWD}/cocos/base/CCThreadPool.h
    ${CWD}/cocos/base/ccUTF8.cpp
    ${CWD}/cocos/base/ccUTF8.h
    ${CWD}/cocos/base/ccUtils.cpp
    ${CWD}/cocos/base/ccUtils.h
    ${CWD}/cocos/base/CCValue.cpp
    ${CWD}/cocos/base/CCValue.h
    ${CWD}/cocos/base/CCVector.h
    ${CWD}/cocos/base/csscolorparser.cpp
    ${CWD}/cocos/base/csscolorparser.hpp
    ${CWD}/cocos/base/etc1.cpp
    ${CWD}/cocos/base/etc1.h
    ${CWD}/cocos/base/etc2.cpp
    ${CWD}/cocos/base/etc2.h
    ${CWD}/cocos/base/pvr.cpp
    ${CWD}/cocos/base/pvr.h
    ${CWD}/cocos/base/uthash.h
    ${CWD}/cocos/base/utlist.h
    ${CWD}/cocos/base/ZipUtils.cpp
    ${CWD}/cocos/base/ZipUtils.h
)

##### math
list(APPEND COCOS2D_SOURCE_LIST
    ${CWD}/cocos/math/CCGeometry.cpp
    ${CWD}/cocos/math/CCGeometry.h
    ${CWD}/cocos/math/CCMath.h
    ${CWD}/cocos/math/CCMathBase.h
    ${CWD}/cocos/math/CCVertex.cpp
    ${CWD}/cocos/math/CCVertex.h
    ${CWD}/cocos/math/Mat3.cpp
    ${CWD}/cocos/math/Mat3.hpp
    ${CWD}/cocos/math/Mat4.cpp
    ${CWD}/cocos/math/Mat4.h
    ${CWD}/cocos/math/Mat4.inl
    ${CWD}/cocos/math/MathUtil.cpp
    ${CWD}/cocos/math/MathUtil.h
    ${CWD}/cocos/math/MathUtil.inl
    ${CWD}/cocos/math/MathUtilNeon.inl
    ${CWD}/cocos/math/MathUtilNeon64.inl
    ${CWD}/cocos/math/MathUtilSSE.inl
    ${CWD}/cocos/math/Quaternion.cpp
    ${CWD}/cocos/math/Quaternion.h
    ${CWD}/cocos/math/Quaternion.inl
    ${CWD}/cocos/math/Vec2.cpp
    ${CWD}/cocos/math/Vec2.h
    ${CWD}/cocos/math/Vec2.inl
    ${CWD}/cocos/math/Vec3.cpp
    ${CWD}/cocos/math/Vec3.h
    ${CWD}/cocos/math/Vec3.inl
    ${CWD}/cocos/math/Vec4.cpp
    ${CWD}/cocos/math/Vec4.h
    ${CWD}/cocos/math/Vec4.inl
)

##### network
list(APPEND COCOS2D_SOURCE_LIST
    ${CWD}/cocos/network/CCDownloader.cpp
    ${CWD}/cocos/network/CCDownloader.h
    ${CWD}/cocos/network/CCIDownloaderImpl.h
    ${CWD}/cocos/network/HttpClient.cpp
    ${CWD}/cocos/network/HttpClient.h
    ${CWD}/cocos/network/HttpCookie.cpp
    ${CWD}/cocos/network/HttpCookie.h
    ${CWD}/cocos/network/HttpRequest.h
    ${CWD}/cocos/network/HttpResponse.h
    ${CWD}/cocos/network/SocketIO.cpp
    ${CWD}/cocos/network/SocketIO.h
    ${CWD}/cocos/network/Uri.cpp
    ${CWD}/cocos/network/Uri.h
    ${CWD}/cocos/network/WebSocket.h
    )

if(APPLE)
    list(APPEND COCOS2D_SOURCE_LIST
        ${CWD}/cocos/network/CCDownloaderImpl-apple.h
        ${CWD}/cocos/network/CCDownloaderImpl-apple.mm
        ${CWD}/cocos/network/HttpAsynConnection-apple.h
        ${CWD}/cocos/network/HttpAsynConnection-apple.m
        ${CWD}/cocos/network/HttpClient-apple.mm
        ${CWD}/cocos/network/WebSocket-apple.mm
        ${CWD}/cocos/network/WebSocket-apple.mm
    )
endif()

if(ANDROID)
    list(APPEND COCOS2D_SOURCE_LIST
        ${CWD}/cocos/network/CCDownloader-android.cpp
        ${CWD}/cocos/network/CCDownloader-android.h
        ${CWD}/cocos/network/HttpClient-android.cpp
    )
endif()
if(NOT APPLE)
    list(APPEND COCOS2D_SOURCE_LIST
        ${CWD}/cocos/network/WebSocket-libwebsockets.cpp
    )
endif()
if(WINDOWS)
    list(APPEND COCOS2D_SOURCE_LIST
        ${CWD}/cocos/network/CCDownloader-curl.cpp
        ${CWD}/cocos/network/CCDownloader-curl.h
    )
endif()


##### platform
list(APPEND COCOS2D_SOURCE_LIST
    ${CWD}/cocos/platform/CCApplication.h
    ${CWD}/cocos/platform/CCCanvasRenderingContext2D.h
    ${CWD}/cocos/platform/CCDevice.h
    ${CWD}/cocos/platform/CCFileUtils.cpp
    ${CWD}/cocos/platform/CCFileUtils.h
    ${CWD}/cocos/platform/CCImage.cpp
    ${CWD}/cocos/platform/CCImage.h
    ${CWD}/cocos/platform/CCPlatformConfig.h
    ${CWD}/cocos/platform/CCPlatformDefine.h
    ${CWD}/cocos/platform/CCSAXParser.cpp
    ${CWD}/cocos/platform/CCSAXParser.h
    ${CWD}/cocos/platform/CCStdC.h
)
if(WINDOWS)
    list(APPEND COCOS2D_SOURCE_LIST
        ${CWD}/cocos/platform/win32/CCApplication-win32.cpp
        ${CWD}/cocos/platform/win32/CCCanvasRenderingContext2D-win32.cpp
        ${CWD}/cocos/platform/win32/CCDevice-win32.cpp
        ${CWD}/cocos/platform/win32/CCFileUtils-win32.cpp
        ${CWD}/cocos/platform/win32/CCFileUtils-win32.h
        ${CWD}/cocos/platform/win32/CCPlatformDefine-win32.h
        ${CWD}/cocos/platform/win32/CCUtils-win32.cpp
        ${CWD}/cocos/platform/win32/CCUtils-win32.h
        ${CWD}/cocos/platform/win32/View-win32.cpp
        ${CWD}/cocos/platform/win32/View-win32.h
        ${CWD}/cocos/platform/win32/compat
        ${CWD}/cocos/platform/win32/compat/stdint.h
        ${CWD}/cocos/platform/win32/inet_pton_mingw.cpp
        ${CWD}/cocos/platform/win32/inet_pton_mingw.h
    )
endif()

##### renderer
list(APPEND COCOS2D_SOURCE_LIST
    ${CWD}/cocos/renderer/core/CMakeLists.txt
    ${CWD}/cocos/renderer/core/Core.h
    ${CWD}/cocos/renderer/core/CoreDef.h
    ${CWD}/cocos/renderer/core/CoreStd.cc
    ${CWD}/cocos/renderer/core/CoreStd.h
    ${CWD}/cocos/renderer/core/gfx/GFXObject.h
    ${CWD}/cocos/renderer/core/gfx/GFXObject.cc
    ${CWD}/cocos/renderer/core/gfx/GFXBindingLayout.cc
    ${CWD}/cocos/renderer/core/gfx/GFXBindingLayout.h
    ${CWD}/cocos/renderer/core/gfx/GFXBuffer.cc
    ${CWD}/cocos/renderer/core/gfx/GFXBuffer.h
    ${CWD}/cocos/renderer/core/gfx/GFXCommand.h
    ${CWD}/cocos/renderer/core/gfx/GFXCommandAllocator.cc
    ${CWD}/cocos/renderer/core/gfx/GFXCommandAllocator.h
    ${CWD}/cocos/renderer/core/gfx/GFXCommandBuffer.cc
    ${CWD}/cocos/renderer/core/gfx/GFXCommandBuffer.h
    ${CWD}/cocos/renderer/core/gfx/GFXCommandPool.h
    ${CWD}/cocos/renderer/core/gfx/GFXContext.cc
    ${CWD}/cocos/renderer/core/gfx/GFXContext.h
    ${CWD}/cocos/renderer/core/gfx/GFXDef.cc
    ${CWD}/cocos/renderer/core/gfx/GFXDef.h
    ${CWD}/cocos/renderer/core/gfx/GFXDevice.cc
    ${CWD}/cocos/renderer/core/gfx/GFXDevice.h
    ${CWD}/cocos/renderer/core/gfx/GFXFramebuffer.cc
    ${CWD}/cocos/renderer/core/gfx/GFXFramebuffer.h
    ${CWD}/cocos/renderer/core/gfx/GFXInputAssembler.cc
    ${CWD}/cocos/renderer/core/gfx/GFXInputAssembler.h
    ${CWD}/cocos/renderer/core/gfx/GFXPipelineLayout.cc
    ${CWD}/cocos/renderer/core/gfx/GFXPipelineLayout.h
    ${CWD}/cocos/renderer/core/gfx/GFXPipelineState.cc
    ${CWD}/cocos/renderer/core/gfx/GFXPipelineState.h
    ${CWD}/cocos/renderer/core/gfx/GFXQueue.cc
    ${CWD}/cocos/renderer/core/gfx/GFXQueue.h
    ${CWD}/cocos/renderer/core/gfx/GFXRenderPass.cc
    ${CWD}/cocos/renderer/core/gfx/GFXRenderPass.h
    ${CWD}/cocos/renderer/core/gfx/GFXSampler.cc
    ${CWD}/cocos/renderer/core/gfx/GFXSampler.h
    ${CWD}/cocos/renderer/core/gfx/GFXShader.cc
    ${CWD}/cocos/renderer/core/gfx/GFXShader.h
    ${CWD}/cocos/renderer/core/gfx/GFXTexture.cc
    ${CWD}/cocos/renderer/core/gfx/GFXTexture.h
    ${CWD}/cocos/renderer/core/gfx/GFXTextureView.cc
    ${CWD}/cocos/renderer/core/gfx/GFXTextureView.h
    ${CWD}/cocos/renderer/core/gfx/GFXWindow.cc
    ${CWD}/cocos/renderer/core/gfx/GFXWindow.h
    ${CWD}/cocos/renderer/core/kernel/Assertion.cc
    ${CWD}/cocos/renderer/core/kernel/Assertion.h
    ${CWD}/cocos/renderer/core/kernel/CachedArray.h
    ${CWD}/cocos/renderer/core/kernel/Log.cc
    ${CWD}/cocos/renderer/core/kernel/Log.h
    ${CWD}/cocos/renderer/core/kernel/Object.h
    ${CWD}/cocos/renderer/core/kernel/UTFString.cc
    ${CWD}/cocos/renderer/core/kernel/UTFString.h
    ${CWD}/cocos/renderer/core/math/CCCoreMath.cc
    ${CWD}/cocos/renderer/core/math/CCCoreMath.h
    ${CWD}/cocos/renderer/core/memory/AllocatedObj.cc
    ${CWD}/cocos/renderer/core/memory/AllocatedObj.h
    ${CWD}/cocos/renderer/core/memory/JeAlloc.cc
    ${CWD}/cocos/renderer/core/memory/JeAlloc.h
    ${CWD}/cocos/renderer/core/memory/MemDef.h
    ${CWD}/cocos/renderer/core/memory/Memory.cc
    ${CWD}/cocos/renderer/core/memory/Memory.h
    ${CWD}/cocos/renderer/core/memory/MemTracker.cc
    ${CWD}/cocos/renderer/core/memory/MemTracker.h
    ${CWD}/cocos/renderer/core/memory/NedPooling.cc
    ${CWD}/cocos/renderer/core/memory/NedPooling.h
    ${CWD}/cocos/renderer/core/memory/StdAlloc.h
    ${CWD}/cocos/renderer/core/memory/StlAlloc.h
    ${CWD}/cocos/renderer/core/TypeDef.h
    ${CWD}/cocos/renderer/core/util/StringUtil.cc
    ${CWD}/cocos/renderer/core/util/StringUtil.h
)

if(CC_USE_GLES2)
    list(APPEND COCOS2D_SOURCE_LIST
        ${CWD}/cocos/renderer/gfx-gles2/GFXGLES2.h
        ${CWD}/cocos/renderer/gfx-gles2/GLES2BindingLayout.cc
        ${CWD}/cocos/renderer/gfx-gles2/GLES2BindingLayout.h
        ${CWD}/cocos/renderer/gfx-gles2/GLES2Buffer.cc
        ${CWD}/cocos/renderer/gfx-gles2/GLES2Buffer.h
        ${CWD}/cocos/renderer/gfx-gles2/GLES2CommandAllocator.cc
        ${CWD}/cocos/renderer/gfx-gles2/GLES2CommandAllocator.h
        ${CWD}/cocos/renderer/gfx-gles2/GLES2CommandBuffer.cc
        ${CWD}/cocos/renderer/gfx-gles2/GLES2CommandBuffer.h
        ${CWD}/cocos/renderer/gfx-gles2/GLES2Commands.cc
        ${CWD}/cocos/renderer/gfx-gles2/GLES2Commands.h
        ${CWD}/cocos/renderer/gfx-gles2/GLES2Context.h
        ${CWD}/cocos/renderer/gfx-gles2/GLES2Device.cc
        ${CWD}/cocos/renderer/gfx-gles2/GLES2Device.h
        ${CWD}/cocos/renderer/gfx-gles2/GLES2Framebuffer.cc
        ${CWD}/cocos/renderer/gfx-gles2/GLES2Framebuffer.h
        ${CWD}/cocos/renderer/gfx-gles2/GLES2GPUObjects.h
        ${CWD}/cocos/renderer/gfx-gles2/GLES2InputAssembler.cc
        ${CWD}/cocos/renderer/gfx-gles2/GLES2InputAssembler.h
        ${CWD}/cocos/renderer/gfx-gles2/GLES2PipelineLayout.cc
        ${CWD}/cocos/renderer/gfx-gles2/GLES2PipelineLayout.h
        ${CWD}/cocos/renderer/gfx-gles2/GLES2PipelineState.cc
        ${CWD}/cocos/renderer/gfx-gles2/GLES2PipelineState.h
        ${CWD}/cocos/renderer/gfx-gles2/GLES2Queue.cc
        ${CWD}/cocos/renderer/gfx-gles2/GLES2Queue.h
        ${CWD}/cocos/renderer/gfx-gles2/GLES2RenderPass.cc
        ${CWD}/cocos/renderer/gfx-gles2/GLES2RenderPass.h
        ${CWD}/cocos/renderer/gfx-gles2/GLES2Sampler.cc
        ${CWD}/cocos/renderer/gfx-gles2/GLES2Sampler.h
        ${CWD}/cocos/renderer/gfx-gles2/GLES2Shader.cc
        ${CWD}/cocos/renderer/gfx-gles2/GLES2Shader.h
        ${CWD}/cocos/renderer/gfx-gles2/GLES2StateCache.h
        ${CWD}/cocos/renderer/gfx-gles2/GLES2Std.cc
        ${CWD}/cocos/renderer/gfx-gles2/GLES2Std.h
        ${CWD}/cocos/renderer/gfx-gles2/GLES2Texture.cc
        ${CWD}/cocos/renderer/gfx-gles2/GLES2Texture.h
        ${CWD}/cocos/renderer/gfx-gles2/GLES2TextureView.cc
        ${CWD}/cocos/renderer/gfx-gles2/GLES2TextureView.h
        ${CWD}/cocos/renderer/gfx-gles2/gles2w.h
        ${CWD}/cocos/renderer/gfx-gles2/GLES2Window.cc
        ${CWD}/cocos/renderer/gfx-gles2/GLES2Window.h
    )

    if(APPLE) 
        list(APPEND COCOS2D_SOURCE_LIST
            ${CWD}/cocos/renderer/gfx-gles2/GLES2Context.mm
            ${CWD}/cocos/renderer/gfx-gles2/gles2w.mm
        )
    else()
        list(APPEND COCOS2D_SOURCE_LIST
            ${CWD}/cocos/renderer/gfx-gles2/GLES2Context.cc
            ${CWD}/cocos/renderer/gfx-gles2/gles2w.c
        )
    endif()

endif()

if(CC_USE_GLES3)
    list(APPEND COCOS2D_SOURCE_LIST
        ${CWD}/cocos/renderer/gfx-gles3/GFXGLES3.h
        ${CWD}/cocos/renderer/gfx-gles3/GLES3BindingLayout.cc
        ${CWD}/cocos/renderer/gfx-gles3/GLES3BindingLayout.h
        ${CWD}/cocos/renderer/gfx-gles3/GLES3Buffer.cc
        ${CWD}/cocos/renderer/gfx-gles3/GLES3Buffer.h
        ${CWD}/cocos/renderer/gfx-gles3/GLES3CommandAllocator.cc
        ${CWD}/cocos/renderer/gfx-gles3/GLES3CommandAllocator.h
        ${CWD}/cocos/renderer/gfx-gles3/GLES3CommandBuffer.cc
        ${CWD}/cocos/renderer/gfx-gles3/GLES3CommandBuffer.h
        ${CWD}/cocos/renderer/gfx-gles3/GLES3Commands.cc
        ${CWD}/cocos/renderer/gfx-gles3/GLES3Commands.h
        ${CWD}/cocos/renderer/gfx-gles3/GLES3Context.h
        ${CWD}/cocos/renderer/gfx-gles3/GLES3Device.cc
        ${CWD}/cocos/renderer/gfx-gles3/GLES3Device.h
        ${CWD}/cocos/renderer/gfx-gles3/GLES3Framebuffer.cc
        ${CWD}/cocos/renderer/gfx-gles3/GLES3Framebuffer.h
        ${CWD}/cocos/renderer/gfx-gles3/GLES3GPUObjects.h
        ${CWD}/cocos/renderer/gfx-gles3/GLES3InputAssembler.cc
        ${CWD}/cocos/renderer/gfx-gles3/GLES3InputAssembler.h
        ${CWD}/cocos/renderer/gfx-gles3/GLES3PipelineLayout.cc
        ${CWD}/cocos/renderer/gfx-gles3/GLES3PipelineLayout.h
        ${CWD}/cocos/renderer/gfx-gles3/GLES3PipelineState.cc
        ${CWD}/cocos/renderer/gfx-gles3/GLES3PipelineState.h
        ${CWD}/cocos/renderer/gfx-gles3/GLES3Queue.cc
        ${CWD}/cocos/renderer/gfx-gles3/GLES3Queue.h
        ${CWD}/cocos/renderer/gfx-gles3/GLES3RenderPass.cc
        ${CWD}/cocos/renderer/gfx-gles3/GLES3RenderPass.h
        ${CWD}/cocos/renderer/gfx-gles3/GLES3Sampler.cc
        ${CWD}/cocos/renderer/gfx-gles3/GLES3Sampler.h
        ${CWD}/cocos/renderer/gfx-gles3/GLES3Shader.cc
        ${CWD}/cocos/renderer/gfx-gles3/GLES3Shader.h
        ${CWD}/cocos/renderer/gfx-gles3/GLES3StateCache.h
        ${CWD}/cocos/renderer/gfx-gles3/GLES3Std.cc
        ${CWD}/cocos/renderer/gfx-gles3/GLES3Std.h
        ${CWD}/cocos/renderer/gfx-gles3/GLES3Texture.cc
        ${CWD}/cocos/renderer/gfx-gles3/GLES3Texture.h
        ${CWD}/cocos/renderer/gfx-gles3/GLES3TextureView.cc
        ${CWD}/cocos/renderer/gfx-gles3/GLES3TextureView.h
        ${CWD}/cocos/renderer/gfx-gles3/gles3w.h
        ${CWD}/cocos/renderer/gfx-gles3/GLES3Window.cc
        ${CWD}/cocos/renderer/gfx-gles3/GLES3Window.h
    )
    if(APPLE) 
        list(APPEND COCOS2D_SOURCE_LIST
            ${CWD}/cocos/renderer/gfx-gles3/GLES3Context.mm
            ${CWD}/cocos/renderer/gfx-gles3/gles3w.mm
        )
    else()
        list(APPEND COCOS2D_SOURCE_LIST
            ${CWD}/cocos/renderer/gfx-gles3/GLES3Context.cc
            ${CWD}/cocos/renderer/gfx-gles3/gles3w.c
        )
    endif()
endif()


##### scripting
list(APPEND COCOS2D_SOURCE_LIST
    ${CWD}/cocos/scripting/js-bindings/auto/jsb_cocos2dx_audioengine_auto.cpp
    ${CWD}/cocos/scripting/js-bindings/auto/jsb_cocos2dx_audioengine_auto.hpp
    ${CWD}/cocos/scripting/js-bindings/auto/jsb_cocos2dx_auto.cpp
    ${CWD}/cocos/scripting/js-bindings/auto/jsb_cocos2dx_auto.hpp
    ${CWD}/cocos/scripting/js-bindings/auto/jsb_cocos2dx_extension_auto.cpp
    ${CWD}/cocos/scripting/js-bindings/auto/jsb_cocos2dx_extension_auto.hpp
    ${CWD}/cocos/scripting/js-bindings/auto/jsb_cocos2dx_network_auto.cpp
    ${CWD}/cocos/scripting/js-bindings/auto/jsb_cocos2dx_network_auto.hpp
    ${CWD}/cocos/scripting/js-bindings/auto/jsb_gfx_auto.cpp
    ${CWD}/cocos/scripting/js-bindings/auto/jsb_gfx_auto.hpp
    ${CWD}/cocos/scripting/js-bindings/auto/jsb_video_auto.cpp
    ${CWD}/cocos/scripting/js-bindings/auto/jsb_video_auto.hpp
    ${CWD}/cocos/scripting/js-bindings/auto/jsb_webview_auto.cpp
    ${CWD}/cocos/scripting/js-bindings/auto/jsb_webview_auto.hpp
)
list(APPEND COCOS2D_SOURCE_LIST

    ${CWD}/cocos/scripting/js-bindings/manual/jsb_classtype.cpp
    ${CWD}/cocos/scripting/js-bindings/manual/jsb_classtype.hpp
    #${CWD}/cocos/scripting/js-bindings/manual/jsb_cocos2dx_extension_manual.cpp
    #${CWD}/cocos/scripting/js-bindings/manual/jsb_cocos2dx_extension_manual.hpp
    ${CWD}/cocos/scripting/js-bindings/manual/jsb_cocos2dx_manual.cpp
    ${CWD}/cocos/scripting/js-bindings/manual/jsb_cocos2dx_manual.hpp
    ${CWD}/cocos/scripting/js-bindings/manual/jsb_cocos2dx_network_manual.cpp
    ${CWD}/cocos/scripting/js-bindings/manual/jsb_cocos2dx_network_manual.h
    ${CWD}/cocos/scripting/js-bindings/manual/jsb_conversions.cpp
    ${CWD}/cocos/scripting/js-bindings/manual/jsb_conversions.hpp
    ${CWD}/cocos/scripting/js-bindings/manual/jsb_gfx_manual.cpp
    ${CWD}/cocos/scripting/js-bindings/manual/jsb_gfx_manual.hpp
    ${CWD}/cocos/scripting/js-bindings/manual/jsb_global.cpp
    ${CWD}/cocos/scripting/js-bindings/manual/jsb_global.h
    ${CWD}/cocos/scripting/js-bindings/manual/jsb_helper.cpp
    ${CWD}/cocos/scripting/js-bindings/manual/jsb_helper.hpp
    ${CWD}/cocos/scripting/js-bindings/manual/jsb_module_register.cpp
    ${CWD}/cocos/scripting/js-bindings/manual/jsb_module_register.hpp
    ${CWD}/cocos/scripting/js-bindings/manual/jsb_platform.h
    ${CWD}/cocos/scripting/js-bindings/manual/jsb_socketio.cpp
    ${CWD}/cocos/scripting/js-bindings/manual/jsb_socketio.hpp
    ${CWD}/cocos/scripting/js-bindings/manual/jsb_websocket.cpp
    ${CWD}/cocos/scripting/js-bindings/manual/jsb_websocket.hpp
    ${CWD}/cocos/scripting/js-bindings/manual/jsb_xmlhttprequest.cpp
    ${CWD}/cocos/scripting/js-bindings/manual/jsb_xmlhttprequest.hpp
)

if(ANDROID)
    list(APPEND COCOS2D_SOURCE_LIST
        ${CWD}/cocos/scripting/js-bindings/manual/jsb_platform_android.cpp
            ${CWD}/cocos/scripting/js-bindings/manual/JavaScriptJavaBridge.cpp
        ${CWD}/cocos/scripting/js-bindings/manual/JavaScriptJavaBridge.h
    )
elseif(APPLE)
    list(APPEND COCOS2D_SOURCE_LIST
        ${CWD}/cocos/scripting/js-bindings/manual/jsb_platfrom_apple.mm    
        ${CWD}/cocos/scripting/js-bindings/manual/JavaScriptObjCBridge.h
        ${CWD}/cocos/scripting/js-bindings/manual/JavaScriptObjCBridge.mm
    )
elseif(WIN32)
    list(APPEND COCOS2D_SOURCE_LIST
        ${CWD}/cocos/scripting/js-bindings/manual/jsb_platfrom_win32.cpp
    )
endif()

if(CC_USE_GLES2)
    list(APPEND COCOS2D_SOURCE_LIST
        ${CWD}/cocos/scripting/js-bindings/auto/jsb_gles2_auto.hpp
        ${CWD}/cocos/scripting/js-bindings/auto/jsb_gles2_auto.cpp
    )
endif()
if(CC_USE_GLES3)
    list(APPEND COCOS2D_SOURCE_LIST
        ${CWD}/cocos/scripting/js-bindings/auto/jsb_gles3_auto.hpp
        ${CWD}/cocos/scripting/js-bindings/auto/jsb_gles3_auto.cpp
    )
endif()

list(APPEND COCOS2D_SOURCE_LIST
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/Base.h
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/Class.cpp
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/Class.hpp
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/debugger/base64.h
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/debugger/env.cc
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/debugger/env.h
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/debugger/http_parser.c
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/debugger/http_parser.h
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/debugger/inspector_agent.cc
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/debugger/inspector_agent.h
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/debugger/inspector_io.cc
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/debugger/inspector_io.h
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/debugger/inspector_socket.cc
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/debugger/inspector_socket.h
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/debugger/inspector_socket_server.cc
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/debugger/inspector_socket_server.h
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/debugger/node.cc
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/debugger/node.h
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/debugger/node_debug_options.cc
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/debugger/node_debug_options.h
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/debugger/node_mutex.h
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/debugger/SHA1.cpp
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/debugger/SHA1.h
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/debugger/util-inl.h
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/debugger/util.cc
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/debugger/util.h
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/debugger/v8_inspector_protocol_json.h
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/HelperMacros.h
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/Object.cpp
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/Object.hpp
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/ObjectWrap.cpp
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/ObjectWrap.h
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/ScriptEngine.cpp
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/ScriptEngine.hpp
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/SeApi.h
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/Utils.cpp
    ${CWD}/cocos/scripting/js-bindings/jswrapper/v8/Utils.hpp
)

list(APPEND COCOS2D_SOURCE_LIST
    ${CWD}/cocos/scripting/js-bindings/jswrapper/config.cpp
    ${CWD}/cocos/scripting/js-bindings/jswrapper/config.hpp
    ${CWD}/cocos/scripting/js-bindings/jswrapper/HandleObject.cpp
    ${CWD}/cocos/scripting/js-bindings/jswrapper/HandleObject.hpp
    ${CWD}/cocos/scripting/js-bindings/jswrapper/MappingUtils.cpp
    ${CWD}/cocos/scripting/js-bindings/jswrapper/MappingUtils.hpp
    ${CWD}/cocos/scripting/js-bindings/jswrapper/Object.hpp
    ${CWD}/cocos/scripting/js-bindings/jswrapper/RefCounter.cpp
    ${CWD}/cocos/scripting/js-bindings/jswrapper/RefCounter.hpp
    ${CWD}/cocos/scripting/js-bindings/jswrapper/SeApi.h
    ${CWD}/cocos/scripting/js-bindings/jswrapper/State.cpp
    ${CWD}/cocos/scripting/js-bindings/jswrapper/State.hpp
    ${CWD}/cocos/scripting/js-bindings/jswrapper/Value.cpp
    ${CWD}/cocos/scripting/js-bindings/jswrapper/Value.hpp
)

list(APPEND COCOS2D_SOURCE_LIST
    ${CWD}/cocos/scripting/js-bindings/event/CustomEventTypes.h 
    ${CWD}/cocos/scripting/js-bindings/event/EventDispatcher.cpp
    ${CWD}/cocos/scripting/js-bindings/event/EventDispatcher.h
)

#### storage
list(APPEND COCOS2D_SOURCE_LIST
    ${CWD}/cocos/storage/local-storage/LocalStorage.cpp
    ${CWD}/cocos/storage/local-storage/LocalStorage.h
)

if(ANDROID)
    list(APPEND COCOS2D_SOURCE_LIST
        ${CWD}/cocos/storage/local-storage/LocalStorage-android.cpp
    )
endif()

#### ui
list(APPEND COCOS2D_SOURCE_LIST
    ${CWD}/cocos/ui/edit-box/EditBox.h
)

if(ANDROID OR IOS) 
    list(APPEND COCOS2D_SOURCE_LIST
        ${CWD}/cocos/ui/videoplayer/VideoPlayer.h
        ${CWD}/cocos/ui/webview/WebView-inl.h
        ${CWD}/cocos/ui/webview/WebView.h
    )
endif()

if(ANDROID)
    list(APPEND COCOS2D_SOURCE_LIST
        ${CWD}/cocos/ui/edit-box/EditBox-android.cpp
        ${CWD}/cocos/ui/videoplayer/VideoPlayer-android.cpp
        ${CWD}/cocos/ui/webview/WebViewImpl-android.cpp
        ${CWD}/cocos/ui/webview/WebViewImpl-android.h
    )
elseif(MACOSX)
    list(APPEND COCOS2D_SOURCE_LIST
        ${CWD}/cocos/ui/edit-box/EditBox-mac.mm
    )
elseif(IOS)
    list(APPEND COCOS2D_SOURCE_LIST
        ${CWD}/cocos/ui/webview/WebViewImpl-ios.h
        ${CWD}/cocos/ui/webview/WebViewImpl-ios.mm
        ${CWD}/cocos/ui/videoplayer/VideoPlayer-ios.mm
        ${CWD}/cocos/ui/edit-box/EditBox-ios.mm
    )
elseif(WIN32)
    list(APPEND COCOS2D_SOURCE_LIST
        ${CWD}/cocos/ui/edit-box/EditBox-win32.cpp
    )
endif()

##### extensions

list(APPEND COCOS2D_SOURCE_LIST
    ${CWD}/extensions/assets-manager/AssetsManagerEx.cpp
    ${CWD}/extensions/assets-manager/AssetsManagerEx.h
    ${CWD}/extensions/assets-manager/CCAsyncTaskPool.cpp
    ${CWD}/extensions/assets-manager/CCAsyncTaskPool.h
    ${CWD}/extensions/assets-manager/CCEventAssetsManagerEx.cpp
    ${CWD}/extensions/assets-manager/CCEventAssetsManagerEx.h
    ${CWD}/extensions/assets-manager/Manifest.cpp
    ${CWD}/extensions/assets-manager/Manifest.h
    ${CWD}/extensions/cocos-ext.h
    ${CWD}/extensions/ExtensionExport.h
    ${CWD}/extensions/ExtensionMacros.h
)
##### external
list(APPEND COCOS2D_SOURCE_LIST
    ${CWD}/external/sources/ConvertUTF/ConvertUTF.c
    ${CWD}/external/sources/ConvertUTF/ConvertUTF.h
    ${CWD}/external/sources/ConvertUTF/ConvertUTFWrapper.cpp
)
list(APPEND COCOS2D_SOURCE_LIST
    ${CWD}/external/sources/tinyxml2/tinyxml2.cpp
    ${CWD}/external/sources/tinyxml2/tinyxml2.h
)

list(APPEND COCOS2D_SOURCE_LIST
    ${CWD}/external/sources/unzip/crypt.h
    ${CWD}/external/sources/unzip/ioapi.cpp
    ${CWD}/external/sources/unzip/ioapi.h
    ${CWD}/external/sources/unzip/ioapi_mem.cpp
    ${CWD}/external/sources/unzip/ioapi_mem.h
    ${CWD}/external/sources/unzip/unzip.cpp
    ${CWD}/external/sources/unzip/unzip.h
)

list(APPEND COCOS2D_SOURCE_LIST
    ${CWD}/external/sources/tommyds/tommy.c
    ${CWD}/external/sources/tommyds/tommy.h
    ${CWD}/external/sources/tommyds/tommyalloc.c
    ${CWD}/external/sources/tommyds/tommyalloc.h
    ${CWD}/external/sources/tommyds/tommyarray.c
    ${CWD}/external/sources/tommyds/tommyarray.h
    ${CWD}/external/sources/tommyds/tommyarrayblk.c
    ${CWD}/external/sources/tommyds/tommyarrayblk.h
    ${CWD}/external/sources/tommyds/tommyarrayblkof.c
    ${CWD}/external/sources/tommyds/tommyarrayblkof.h
    ${CWD}/external/sources/tommyds/tommyarrayof.c
    ${CWD}/external/sources/tommyds/tommyarrayof.h
    ${CWD}/external/sources/tommyds/tommychain.h
    ${CWD}/external/sources/tommyds/tommyhash.c
    ${CWD}/external/sources/tommyds/tommyhash.h
    ${CWD}/external/sources/tommyds/tommyhashdyn.c
    ${CWD}/external/sources/tommyds/tommyhashdyn.h
    ${CWD}/external/sources/tommyds/tommyhashlin.c
    ${CWD}/external/sources/tommyds/tommyhashlin.h
    ${CWD}/external/sources/tommyds/tommyhashtbl.c
    ${CWD}/external/sources/tommyds/tommyhashtbl.h
    ${CWD}/external/sources/tommyds/tommylist.c
    ${CWD}/external/sources/tommyds/tommylist.h
    ${CWD}/external/sources/tommyds/tommytree.c
    ${CWD}/external/sources/tommyds/tommytree.h
    ${CWD}/external/sources/tommyds/tommytrie.c
    ${CWD}/external/sources/tommyds/tommytrie.h
    ${CWD}/external/sources/tommyds/tommytrieinp.c
    ${CWD}/external/sources/tommyds/tommytrieinp.h
    ${CWD}/external/sources/tommyds/tommytypes.h
)
list(APPEND COCOS2D_SOURCE_LIST
    ${CWD}/external/sources/xxtea/xxtea.cpp
    ${CWD}/external/sources/xxtea/xxtea.h
)

add_library(cocos2d ${COCOS2D_SOURCE_LIST})

target_include_directories(cocos2d PUBLIC 
    ${CC_EXTERNAL_INCLUDES}
    ${CWD}
    ${CWD}/cocos
    ${CWD}/cocos/renderer
    ${CWD}/cocos/platform
    ${CWD}/cocos/renderer/core
    ${CWD}/external/sources/khronos
)

if(WIN32)
    target_link_libraries(cocos2d PUBLIC
        ws2_32 userenv psapi winmm Version Iphlpapi opengl32
        ${CC_EXTERNAL_LIBS}
    )

    target_compile_definitions(cocos2d PUBLIC
        CC_STATIC 
        $<$<CONFIG:Debug>:COCOS2D_DEBUG=1>
        WIN32
        _WIN32
        _WINDOWS
        UNICODE
        _UNICODE
        _CRT_SECURE_NO_WARNINGS
        _SCL_SECURE_NO_WARNINGS
        _USRLIBSIMSTATIC
        SE_ENABLE_INSPECTOR
    )

endif()

source_group(TREE ${CWD} PREFIX "Source Files" FILES ${COCOS2D_SOURCE_LIST})


if(MSVC)
    file(GLOB_RECURSE WIN32_DLLS 
        ${CWD}/external/win32/libs/*.dll 
        ${CWD}/external/win32/libs/Debug/*.dll
    )
    target_compile_options(cocos2d PUBLIC /MP)
endif()