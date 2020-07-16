


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

if(USE_SE_JSC)
    set(USE_SE_V8 OFF)
    set(USE_V8_DEBUGGER OFF)
endif()
################################# external source code ################################


include(${CMAKE_CURRENT_LIST_DIR}/external/CMakeLists.txt)

################################# engine source code ##################################

set(CWD ${CMAKE_CURRENT_LIST_DIR})
set(COCOS2D_SOURCE_LIST)

macro(cocos_source_files)
    set(list_var "${ARGN}")
    foreach(src IN LISTS list_var)
        set(fp "${CWD}/${src}")
        if(EXISTS ${fp})
            list(APPEND COCOS2D_SOURCE_LIST "${CWD}/${src}")
        else()
            message(FATAL_ERROR "Cocos souce file not exists: ${src} in ${CWD}")
        endif()
    endforeach()
endmacro()


cocos_source_files(
    cocos/cocos2d.h
    cocos/cocos2d.cpp
)

##### audio
cocos_source_files(
    cocos/audio/AudioEngine.cpp
    cocos/audio/include/AudioEngine.h
    cocos/audio/include/Export.h
)
if(WINDOWS)
    cocos_source_files(
        cocos/audio/win32/AudioCache.cpp
        cocos/audio/win32/AudioCache.h
        cocos/audio/win32/AudioDecoder.cpp
        cocos/audio/win32/AudioDecoder.h
        cocos/audio/win32/AudioDecoderManager.cpp
        cocos/audio/win32/AudioDecoderManager.h
        cocos/audio/win32/AudioDecoderMp3.cpp
        cocos/audio/win32/AudioDecoderMp3.h
        cocos/audio/win32/AudioDecoderOgg.cpp
        cocos/audio/win32/AudioDecoderOgg.h
        cocos/audio/win32/AudioEngine-win32.cpp
        cocos/audio/win32/AudioEngine-win32.h
        cocos/audio/win32/AudioMacros.h
        cocos/audio/win32/AudioPlayer.cpp
        cocos/audio/win32/AudioPlayer.h
    )
elseif(ANDROID)
    cocos_source_files(
        cocos/audio/android/AssetFd.cpp
        cocos/audio/android/AssetFd.h
        cocos/audio/android/audio.h
        cocos/audio/android/AudioBufferProvider.h
        cocos/audio/android/AudioDecoder.cpp
        cocos/audio/android/AudioDecoder.h
        cocos/audio/android/AudioDecoderMp3.cpp
        cocos/audio/android/AudioDecoderMp3.h
        cocos/audio/android/AudioDecoderOgg.cpp
        cocos/audio/android/AudioDecoderOgg.h
        cocos/audio/android/AudioDecoderProvider.cpp
        cocos/audio/android/AudioDecoderProvider.h
        cocos/audio/android/AudioDecoderSLES.cpp
        cocos/audio/android/AudioDecoderSLES.h
        cocos/audio/android/AudioDecoderWav.cpp
        cocos/audio/android/AudioDecoderWav.h
        cocos/audio/android/AudioEngine-inl.cpp
        cocos/audio/android/AudioEngine-inl.h
        cocos/audio/android/AudioMixer.cpp
        cocos/audio/android/AudioMixer.h
        cocos/audio/android/AudioMixerController.cpp
        cocos/audio/android/AudioMixerController.h
        cocos/audio/android/AudioMixerOps.h
        cocos/audio/android/AudioPlayerProvider.cpp
        cocos/audio/android/AudioPlayerProvider.h
        cocos/audio/android/AudioResampler.cpp
        cocos/audio/android/AudioResampler.h
        cocos/audio/android/AudioResamplerCubic.cpp
        cocos/audio/android/AudioResamplerCubic.h
        cocos/audio/android/AudioResamplerPublic.h
        cocos/audio/android/audio_utils/format.c
        cocos/audio/android/audio_utils/include/audio_utils/format.h
        cocos/audio/android/audio_utils/include/audio_utils/minifloat.h
        cocos/audio/android/audio_utils/include/audio_utils/primitives.h
        cocos/audio/android/audio_utils/minifloat.cpp
        cocos/audio/android/audio_utils/primitives.c
        cocos/audio/android/audio_utils/private/private.h
        cocos/audio/android/cutils/bitops.h
        cocos/audio/android/cutils/log.h
        cocos/audio/android/IAudioPlayer.h
        cocos/audio/android/ICallerThreadUtils.h
        cocos/audio/android/IVolumeProvider.h
        cocos/audio/android/mp3reader.cpp
        cocos/audio/android/mp3reader.h
        cocos/audio/android/OpenSLHelper.h
        cocos/audio/android/PcmAudioPlayer.cpp
        cocos/audio/android/PcmAudioPlayer.h
        cocos/audio/android/PcmAudioService.cpp
        cocos/audio/android/PcmAudioService.h
        cocos/audio/android/PcmBufferProvider.cpp
        cocos/audio/android/PcmBufferProvider.h
        cocos/audio/android/PcmData.cpp
        cocos/audio/android/PcmData.h
        cocos/audio/android/tinysndfile.cpp
        cocos/audio/android/tinysndfile.h
        cocos/audio/android/Track.cpp
        cocos/audio/android/Track.h
        cocos/audio/android/UrlAudioPlayer.cpp
        cocos/audio/android/UrlAudioPlayer.h
        cocos/audio/android/utils/Compat.h
        cocos/audio/android/utils/Errors.h
        cocos/audio/android/utils/Utils.cpp
        cocos/audio/android/utils/Utils.h
    )
elseif(APPLE)
    cocos_source_files(
        cocos/audio/apple/AudioDecoder.h
        cocos/audio/apple/AudioPlayer.mm
        cocos/audio/apple/AudioDecoder.mm
        cocos/audio/apple/AudioPlayer.h
        cocos/audio/apple/AudioEngine-inl.mm
        cocos/audio/apple/AudioMacros.h
        cocos/audio/apple/AudioCache.mm
        cocos/audio/apple/AudioCache.h
        cocos/audio/apple/AudioEngine-inl.h
    )
endif()


##### base
cocos_source_files(
    cocos/base/base64.cpp
    cocos/base/base64.h
    cocos/base/CCAutoreleasePool.cpp
    cocos/base/CCAutoreleasePool.h
    cocos/base/ccCArray.cpp
    cocos/base/ccCArray.h
    cocos/base/ccConfig.h
    cocos/base/CCConfiguration.cpp
    cocos/base/CCConfiguration.h
    cocos/base/CCData.cpp
    cocos/base/CCData.h
    cocos/base/CCGLUtils.h
    cocos/base/CCGLUtils.cpp
    cocos/base/CCLog.cpp
    cocos/base/CCLog.h
    cocos/base/ccMacros.h
    cocos/base/CCMap.h
    cocos/base/ccRandom.cpp
    cocos/base/ccRandom.h
    cocos/base/CCRef.cpp
    cocos/base/CCRef.h
    cocos/base/CCRefPtr.h
    cocos/base/CCRenderTexture.cpp
    cocos/base/CCRenderTexture.h
    cocos/base/CCScheduler.cpp
    cocos/base/CCScheduler.h
    cocos/base/CCThreadPool.cpp
    cocos/base/CCThreadPool.h
    cocos/base/ccTypes.h
    cocos/base/ccTypes.cpp
    cocos/base/ccUTF8.cpp
    cocos/base/ccUTF8.h
    cocos/base/ccUtils.cpp
    cocos/base/ccUtils.h
    cocos/base/CCValue.cpp
    cocos/base/CCValue.h
    cocos/base/CCVector.h
    cocos/base/csscolorparser.cpp
    cocos/base/csscolorparser.hpp
    cocos/base/etc1.cpp
    cocos/base/etc1.h
    cocos/base/etc2.cpp
    cocos/base/etc2.h
    cocos/base/pvr.cpp
    cocos/base/pvr.h
    cocos/base/TGAlib.h
    cocos/base/TGAlib.cpp
    cocos/base/uthash.h
    cocos/base/utlist.h
    cocos/base/ZipUtils.cpp
    cocos/base/ZipUtils.h
)

##### math
cocos_source_files(
    cocos/math/CCGeometry.cpp
    cocos/math/CCGeometry.h
    cocos/math/CCMath.h
    cocos/math/CCMathBase.h
    cocos/math/CCVertex.cpp
    cocos/math/CCVertex.h
    cocos/math/Mat3.cpp
    cocos/math/Mat3.hpp
    cocos/math/Mat4.cpp
    cocos/math/Mat4.h
    cocos/math/Mat4.inl
    cocos/math/MathUtil.cpp
    cocos/math/MathUtil.h
    cocos/math/MathUtil.inl
    cocos/math/MathUtilNeon.inl
    cocos/math/MathUtilNeon64.inl
    cocos/math/MathUtilSSE.inl
    cocos/math/Quaternion.cpp
    cocos/math/Quaternion.h
    cocos/math/Quaternion.inl
    cocos/math/Vec2.cpp
    cocos/math/Vec2.h
    cocos/math/Vec2.inl
    cocos/math/Vec3.cpp
    cocos/math/Vec3.h
    cocos/math/Vec3.inl
    cocos/math/Vec4.cpp
    cocos/math/Vec4.h
    cocos/math/Vec4.inl
)

##### network
cocos_source_files(
    cocos/network/CCDownloader.cpp
    cocos/network/CCDownloader.h
    cocos/network/CCIDownloaderImpl.h
    cocos/network/HttpClient.h
    cocos/network/HttpCookie.cpp
    cocos/network/HttpCookie.h
    cocos/network/HttpRequest.h
    cocos/network/HttpResponse.h
    cocos/network/SocketIO.cpp
    cocos/network/SocketIO.h
    cocos/network/Uri.cpp
    cocos/network/Uri.h
    cocos/network/WebSocket.h
    )

if(APPLE)
    cocos_source_files(
        cocos/network/CCDownloaderImpl-apple.h
        cocos/network/CCDownloaderImpl-apple.mm
        cocos/network/HttpAsynConnection-apple.h
        cocos/network/HttpAsynConnection-apple.m
        cocos/network/HttpClient-apple.mm
        cocos/network/WebSocket-apple.mm
        cocos/network/WebSocket-apple.mm
    )
    set_source_files_properties(${CWD}/cocos/network/WebSocket-apple.mm PROPERTIES
        COMPILE_FLAGS -fobjc-arc
    )
endif()

if(ANDROID)
    cocos_source_files(
        cocos/network/CCDownloader-android.cpp
        cocos/network/CCDownloader-android.h
        cocos/network/HttpClient-android.cpp
    )
endif()

if(NOT APPLE)
    cocos_source_files(
        cocos/network/WebSocket-libwebsockets.cpp
    )
endif()

if(WINDOWS)
    cocos_source_files(
        cocos/network/CCDownloader-curl.cpp
        cocos/network/CCDownloader-curl.h
    )
endif()

if(WINDOWS OR MACOSX)
    cocos_source_files(
        cocos/network/HttpClient.cpp
    )
endif()


##### platform
cocos_source_files(
    cocos/platform/CCApplication.h
    cocos/platform/CCCanvasRenderingContext2D.h
    cocos/platform/CCDevice.h
    cocos/platform/CCFileUtils.cpp
    cocos/platform/CCFileUtils.h
    cocos/platform/CCImage.cpp
    cocos/platform/CCImage.h
    cocos/platform/CCPlatformConfig.h
    cocos/platform/CCPlatformDefine.h
    cocos/platform/CCSAXParser.cpp
    cocos/platform/CCSAXParser.h
    cocos/platform/CCStdC.h
)

if(WINDOWS OR MACOSX)
    cocos_source_files(
        cocos/platform/desktop/CCGLView-desktop.cpp
        cocos/platform/desktop/CCGLView-desktop.h
    )
endif()

if(WINDOWS)
    cocos_source_files(
        cocos/platform/win32/CCApplication-win32.cpp
        cocos/platform/win32/CCCanvasRenderingContext2D-win32.cpp
        cocos/platform/win32/CCDevice-win32.cpp
        cocos/platform/win32/CCFileUtils-win32.cpp
        cocos/platform/win32/CCFileUtils-win32.h
        cocos/platform/win32/CCPlatformDefine-win32.h
        cocos/platform/win32/CCGL-win32.h
        cocos/platform/win32/CCUtils-win32.cpp
        cocos/platform/win32/CCUtils-win32.h
        cocos/platform/win32/compat/stdint.h
        cocos/platform/win32/inet_pton_mingw.cpp
        cocos/platform/win32/inet_pton_mingw.h
    )
elseif(ANDROID)
    cocos_source_files(
        cocos/platform/android/CCApplication-android.cpp
        cocos/platform/android/CCCanvasRenderingContext2D-android.cpp
        cocos/platform/android/CCDevice-android.cpp
        cocos/platform/android/CCFileUtils-android.cpp
        cocos/platform/android/CCFileUtils-android.h
        cocos/platform/android/CCPlatformDefine-android.h
        cocos/platform/android/CCGL-android.h
        cocos/platform/android/jni/JniHelper.cpp
        cocos/platform/android/jni/JniHelper.h
        cocos/platform/android/jni/JniImp.cpp
        cocos/platform/android/jni/JniImp.h
    )
elseif(APPLE)
    cocos_source_files(
        cocos/platform/apple/CCFileUtils-apple.h
        cocos/platform/apple/CCDevice-apple.mm
        cocos/platform/apple/CCFileUtils-apple.mm
        cocos/platform/apple/CCCanvasRenderingContext2D-apple.mm
        cocos/platform/apple/CCDevice-apple.h   
    )
    #bad struct
    cocos_source_files(
        cocos/platform/ios/CCReachability.h
        cocos/platform/ios/CCReachability.cpp
    )
    if(MACOSX)
        cocos_source_files(
            cocos/platform/mac/CCPlatformDefine-mac.h
            cocos/platform/mac/CCDevice-mac.mm
            cocos/platform/mac/CCGL-mac.h
            cocos/platform/mac/CCApplication-mac.mm
        )
    elseif(IOS)
        cocos_source_files(
            cocos/platform/ios/CCGL-ios.h
            cocos/platform/ios/CCEAGLView-ios.h
            cocos/platform/ios/CCEAGLView-ios.mm
            cocos/platform/ios/CCApplication-ios.mm
            cocos/platform/ios/CCDevice-ios.mm
            cocos/platform/ios/CCPlatformDefine-ios.h
            cocos/platform/ios/CCImage-ios.mm
            cocos/platform/ios/OpenGL_Internal-ios.h
        )
    endif()
endif()

##### renderer
cocos_source_files(
    cocos/renderer/gfx/DeviceGraphics.cpp
    cocos/renderer/gfx/DeviceGraphics.h
    cocos/renderer/gfx/FrameBuffer.cpp
    cocos/renderer/gfx/FrameBuffer.h
    cocos/renderer/gfx/GFX.cpp
    cocos/renderer/gfx/GFX.h
    cocos/renderer/gfx/GFXUtils.cpp
    cocos/renderer/gfx/GFXUtils.h
    cocos/renderer/gfx/GraphicsHandle.cpp
    cocos/renderer/gfx/GraphicsHandle.h
    cocos/renderer/gfx/IndexBuffer.cpp
    cocos/renderer/gfx/IndexBuffer.h
    cocos/renderer/gfx/Program.cpp
    cocos/renderer/gfx/Program.h
    cocos/renderer/gfx/RenderBuffer.cpp
    cocos/renderer/gfx/RenderBuffer.h
    cocos/renderer/gfx/RenderTarget.cpp
    cocos/renderer/gfx/RenderTarget.h
    cocos/renderer/gfx/State.cpp
    cocos/renderer/gfx/State.h
    cocos/renderer/gfx/Texture.cpp
    cocos/renderer/gfx/Texture.h
    cocos/renderer/gfx/Texture2D.cpp
    cocos/renderer/gfx/Texture2D.h
    cocos/renderer/gfx/VertexBuffer.cpp
    cocos/renderer/gfx/VertexBuffer.h
    cocos/renderer/gfx/VertexFormat.cpp
    cocos/renderer/gfx/VertexFormat.h
    cocos/renderer/Macro.h
    cocos/renderer/memop/RecyclePool.hpp
    cocos/renderer/renderer/BaseRenderer.cpp
    cocos/renderer/renderer/BaseRenderer.h
    cocos/renderer/renderer/Camera.cpp
    cocos/renderer/renderer/Camera.h
    cocos/renderer/renderer/Config.cpp
    cocos/renderer/renderer/Config.h
    cocos/renderer/renderer/Effect.cpp
    cocos/renderer/renderer/Effect.h
    cocos/renderer/renderer/EffectBase.cpp
    cocos/renderer/renderer/EffectBase.h
    cocos/renderer/renderer/EffectVariant.cpp
    cocos/renderer/renderer/EffectVariant.hpp
    cocos/renderer/renderer/ForwardRenderer.cpp
    cocos/renderer/renderer/ForwardRenderer.h
    cocos/renderer/renderer/INode.h
    cocos/renderer/renderer/InputAssembler.cpp
    cocos/renderer/renderer/InputAssembler.h
    cocos/renderer/renderer/Light.cpp
    cocos/renderer/renderer/Light.h
    cocos/renderer/renderer/Model.cpp
    cocos/renderer/renderer/Model.h
    cocos/renderer/renderer/Pass.cpp
    cocos/renderer/renderer/Pass.h
    cocos/renderer/renderer/ProgramLib.cpp
    cocos/renderer/renderer/ProgramLib.h
    cocos/renderer/renderer/Renderer.h
    cocos/renderer/renderer/RendererUtils.cpp
    cocos/renderer/renderer/RendererUtils.h
    cocos/renderer/renderer/Scene.cpp
    cocos/renderer/renderer/Scene.h
    cocos/renderer/renderer/Technique.cpp
    cocos/renderer/renderer/Technique.h
    cocos/renderer/renderer/View.cpp
    cocos/renderer/renderer/View.h
    cocos/renderer/scene/assembler/Assembler.cpp
    cocos/renderer/scene/assembler/Assembler.hpp
    cocos/renderer/scene/assembler/AssemblerBase.cpp
    cocos/renderer/scene/assembler/AssemblerBase.hpp
    cocos/renderer/scene/assembler/AssemblerSprite.cpp
    cocos/renderer/scene/assembler/AssemblerSprite.hpp
    cocos/renderer/scene/assembler/CustomAssembler.cpp
    cocos/renderer/scene/assembler/CustomAssembler.hpp
    cocos/renderer/scene/assembler/MaskAssembler.cpp
    cocos/renderer/scene/assembler/MaskAssembler.hpp
    cocos/renderer/scene/assembler/MeshAssembler.cpp
    cocos/renderer/scene/assembler/MeshAssembler.hpp
    cocos/renderer/scene/assembler/Particle3DAssembler.cpp
    cocos/renderer/scene/assembler/Particle3DAssembler.hpp
    cocos/renderer/scene/assembler/RenderData.cpp
    cocos/renderer/scene/assembler/RenderData.hpp
    cocos/renderer/scene/assembler/RenderDataList.cpp
    cocos/renderer/scene/assembler/RenderDataList.hpp
    cocos/renderer/scene/assembler/SimpleSprite2D.cpp
    cocos/renderer/scene/assembler/SimpleSprite2D.hpp
    cocos/renderer/scene/assembler/SimpleSprite3D.cpp
    cocos/renderer/scene/assembler/SimpleSprite3D.hpp
    cocos/renderer/scene/assembler/SlicedSprite2D.cpp
    cocos/renderer/scene/assembler/SlicedSprite2D.hpp
    cocos/renderer/scene/assembler/SlicedSprite3D.cpp
    cocos/renderer/scene/assembler/SlicedSprite3D.hpp
    cocos/renderer/scene/assembler/TiledMapAssembler.cpp
    cocos/renderer/scene/assembler/TiledMapAssembler.hpp
    cocos/renderer/scene/MemPool.cpp
    cocos/renderer/scene/MemPool.hpp
    cocos/renderer/scene/MeshBuffer.cpp
    cocos/renderer/scene/MeshBuffer.hpp
    cocos/renderer/scene/ModelBatcher.cpp
    cocos/renderer/scene/ModelBatcher.hpp
    cocos/renderer/scene/NodeMemPool.cpp
    cocos/renderer/scene/NodeMemPool.hpp
    cocos/renderer/scene/NodeProxy.cpp
    cocos/renderer/scene/NodeProxy.hpp
    cocos/renderer/scene/ParallelTask.cpp
    cocos/renderer/scene/ParallelTask.hpp
    cocos/renderer/scene/RenderFlow.cpp
    cocos/renderer/scene/RenderFlow.hpp
    cocos/renderer/scene/scene-bindings.h
    cocos/renderer/scene/StencilManager.cpp
    cocos/renderer/scene/StencilManager.hpp
    cocos/renderer/Types.cpp
    cocos/renderer/Types.h
)

#### label
cocos_source_files(
    cocos/2d/CCFontAtlas.cpp
    cocos/2d/CCFontAtlas.h
    cocos/2d/CCFontFreetype.cpp
    cocos/2d/CCFontFreetype.h
    cocos/2d/CCLabelLayout.cpp
    cocos/2d/CCLabelLayout.h
    cocos/2d/CCTTFLabelAtlasCache.cpp
    cocos/2d/CCTTFLabelAtlasCache.h
    cocos/2d/CCTTFLabelRenderer.cpp
    cocos/2d/CCTTFLabelRenderer.h
    cocos/2d/CCTTFTypes.cpp
    cocos/2d/CCTTFTypes.h
)


##### script bindings
######## auto
cocos_source_files(
    cocos/scripting/js-bindings/auto/jsb_cocos2dx_audioengine_auto.cpp
    cocos/scripting/js-bindings/auto/jsb_cocos2dx_audioengine_auto.hpp
    cocos/scripting/js-bindings/auto/jsb_cocos2dx_auto.cpp
    cocos/scripting/js-bindings/auto/jsb_cocos2dx_auto.hpp
    cocos/scripting/js-bindings/auto/jsb_cocos2dx_dragonbones_auto.cpp
    cocos/scripting/js-bindings/auto/jsb_cocos2dx_dragonbones_auto.hpp
    cocos/scripting/js-bindings/auto/jsb_cocos2dx_editor_support_auto.cpp
    cocos/scripting/js-bindings/auto/jsb_cocos2dx_editor_support_auto.hpp
    cocos/scripting/js-bindings/auto/jsb_cocos2dx_extension_auto.cpp
    cocos/scripting/js-bindings/auto/jsb_cocos2dx_extension_auto.hpp
    cocos/scripting/js-bindings/auto/jsb_cocos2dx_network_auto.cpp
    cocos/scripting/js-bindings/auto/jsb_cocos2dx_network_auto.hpp
    cocos/scripting/js-bindings/auto/jsb_cocos2dx_particle_auto.cpp
    cocos/scripting/js-bindings/auto/jsb_cocos2dx_particle_auto.hpp
    cocos/scripting/js-bindings/auto/jsb_cocos2dx_spine_auto.cpp
    cocos/scripting/js-bindings/auto/jsb_cocos2dx_spine_auto.hpp
    cocos/scripting/js-bindings/auto/jsb_gfx_auto.cpp
    cocos/scripting/js-bindings/auto/jsb_gfx_auto.hpp
    cocos/scripting/js-bindings/auto/jsb_renderer_auto.cpp
    cocos/scripting/js-bindings/auto/jsb_renderer_auto.hpp
    cocos/scripting/js-bindings/auto/jsb_video_auto.cpp
    cocos/scripting/js-bindings/auto/jsb_video_auto.hpp
    cocos/scripting/js-bindings/auto/jsb_webview_auto.cpp
    cocos/scripting/js-bindings/auto/jsb_webview_auto.hpp
)
######## manual
cocos_source_files(
    cocos/scripting/js-bindings/manual/jsb_classtype.cpp
    cocos/scripting/js-bindings/manual/jsb_classtype.hpp
    # cocos/scripting/js-bindings/manual/jsb_cocos2dx_extension_manual.cpp
    # cocos/scripting/js-bindings/manual/jsb_cocos2dx_extension_manual.hpp
    cocos/scripting/js-bindings/manual/jsb_cocos2dx_manual.cpp
    cocos/scripting/js-bindings/manual/jsb_cocos2dx_manual.hpp
    cocos/scripting/js-bindings/manual/jsb_cocos2dx_network_manual.cpp
    cocos/scripting/js-bindings/manual/jsb_cocos2dx_network_manual.h
    cocos/scripting/js-bindings/manual/jsb_conversions.cpp
    cocos/scripting/js-bindings/manual/jsb_conversions.hpp
    cocos/scripting/js-bindings/manual/jsb_dragonbones_manual.cpp
    cocos/scripting/js-bindings/manual/jsb_dragonbones_manual.hpp
    cocos/scripting/js-bindings/manual/jsb_gfx_manual.cpp
    cocos/scripting/js-bindings/manual/jsb_gfx_manual.hpp
    cocos/scripting/js-bindings/manual/jsb_global.cpp
    cocos/scripting/js-bindings/manual/jsb_global.h
    cocos/scripting/js-bindings/manual/jsb_helper.cpp
    cocos/scripting/js-bindings/manual/jsb_helper.hpp
    cocos/scripting/js-bindings/manual/jsb_jni_manual.cpp
    cocos/scripting/js-bindings/manual/jsb_jni_manual.hpp
    cocos/scripting/js-bindings/manual/jsb_jni_utils.cpp
    cocos/scripting/js-bindings/manual/jsb_jni_utils.h
    cocos/scripting/js-bindings/manual/jsb_module_register.cpp
    cocos/scripting/js-bindings/manual/jsb_module_register.hpp
    # cocos/scripting/js-bindings/manual/jsb_node.cpp
    # cocos/scripting/js-bindings/manual/jsb_node.hpp
    cocos/scripting/js-bindings/manual/jsb_opengl_manual.cpp
    cocos/scripting/js-bindings/manual/jsb_opengl_manual.hpp
    cocos/scripting/js-bindings/manual/jsb_opengl_utils.cpp
    cocos/scripting/js-bindings/manual/jsb_opengl_utils.hpp
    cocos/scripting/js-bindings/manual/jsb_platform.h
    cocos/scripting/js-bindings/manual/jsb_renderer_manual.cpp
    cocos/scripting/js-bindings/manual/jsb_renderer_manual.hpp
    cocos/scripting/js-bindings/manual/jsb_socketio.cpp
    cocos/scripting/js-bindings/manual/jsb_socketio.hpp
    cocos/scripting/js-bindings/manual/jsb_spine_manual.cpp
    cocos/scripting/js-bindings/manual/jsb_spine_manual.hpp
    cocos/scripting/js-bindings/manual/jsb_websocket.cpp
    cocos/scripting/js-bindings/manual/jsb_websocket.hpp
    cocos/scripting/js-bindings/manual/jsb_websocket_server.cpp
    cocos/scripting/js-bindings/manual/jsb_websocket_server.hpp
    cocos/scripting/js-bindings/manual/jsb_xmlhttprequest.cpp
    cocos/scripting/js-bindings/manual/jsb_xmlhttprequest.hpp
)

if(ANDROID)
cocos_source_files(
    cocos/scripting/js-bindings/manual/JavaScriptJavaBridge.cpp
    cocos/scripting/js-bindings/manual/JavaScriptJavaBridge.h
    cocos/scripting/js-bindings/manual/jsb_platform_android.cpp
)
elseif(APPLE)
cocos_source_files(
    cocos/scripting/js-bindings/manual/jsb_platfrom_apple.mm
    cocos/scripting/js-bindings/manual/JavaScriptObjCBridge.h
    cocos/scripting/js-bindings/manual/JavaScriptObjCBridge.mm
)
elseif(WIN32)
cocos_source_files(
    cocos/scripting/js-bindings/manual/jsb_platfrom_win32.cpp
)
endif()

if(ANDROID)
    cocos_source_files(
        cocos/scripting/js-bindings/manual/jsb_platform_android.cpp
        cocos/scripting/js-bindings/manual/JavaScriptJavaBridge.cpp
        cocos/scripting/js-bindings/manual/JavaScriptJavaBridge.h
    )
elseif(APPLE)
    cocos_source_files(
        cocos/scripting/js-bindings/manual/jsb_platfrom_apple.mm    
        cocos/scripting/js-bindings/manual/JavaScriptObjCBridge.h
        cocos/scripting/js-bindings/manual/JavaScriptObjCBridge.mm
    )
elseif(WIN32)
    cocos_source_files(
        cocos/scripting/js-bindings/manual/jsb_platfrom_win32.cpp
    )
endif()

if(USE_SE_V8)
    cocos_source_files(
        cocos/scripting/js-bindings/jswrapper/v8/Base.h
        cocos/scripting/js-bindings/jswrapper/v8/Class.cpp
        cocos/scripting/js-bindings/jswrapper/v8/Class.hpp
        cocos/scripting/js-bindings/jswrapper/v8/HelperMacros.h
        cocos/scripting/js-bindings/jswrapper/v8/Object.cpp
        cocos/scripting/js-bindings/jswrapper/v8/Object.hpp
        cocos/scripting/js-bindings/jswrapper/v8/ObjectWrap.cpp
        cocos/scripting/js-bindings/jswrapper/v8/ObjectWrap.h
        cocos/scripting/js-bindings/jswrapper/v8/ScriptEngine.cpp
        cocos/scripting/js-bindings/jswrapper/v8/ScriptEngine.hpp
        cocos/scripting/js-bindings/jswrapper/v8/SeApi.h
        cocos/scripting/js-bindings/jswrapper/v8/Utils.cpp
        cocos/scripting/js-bindings/jswrapper/v8/Utils.hpp
    )
    if(USE_V8_DEBUGGER)
    cocos_source_files(
        cocos/scripting/js-bindings/jswrapper/v8/debugger/base64.h
        cocos/scripting/js-bindings/jswrapper/v8/debugger/env.cc
        cocos/scripting/js-bindings/jswrapper/v8/debugger/env.h
        cocos/scripting/js-bindings/jswrapper/v8/debugger/http_parser.c
        cocos/scripting/js-bindings/jswrapper/v8/debugger/http_parser.h
        cocos/scripting/js-bindings/jswrapper/v8/debugger/inspector_agent.cc
        cocos/scripting/js-bindings/jswrapper/v8/debugger/inspector_agent.h
        cocos/scripting/js-bindings/jswrapper/v8/debugger/inspector_io.cc
        cocos/scripting/js-bindings/jswrapper/v8/debugger/inspector_io.h
        cocos/scripting/js-bindings/jswrapper/v8/debugger/inspector_socket.cc
        cocos/scripting/js-bindings/jswrapper/v8/debugger/inspector_socket.h
        cocos/scripting/js-bindings/jswrapper/v8/debugger/inspector_socket_server.cc
        cocos/scripting/js-bindings/jswrapper/v8/debugger/inspector_socket_server.h
        cocos/scripting/js-bindings/jswrapper/v8/debugger/node.cc
        cocos/scripting/js-bindings/jswrapper/v8/debugger/node.h
        cocos/scripting/js-bindings/jswrapper/v8/debugger/node_debug_options.cc
        cocos/scripting/js-bindings/jswrapper/v8/debugger/node_debug_options.h
        cocos/scripting/js-bindings/jswrapper/v8/debugger/node_mutex.h
        cocos/scripting/js-bindings/jswrapper/v8/debugger/SHA1.cpp
        cocos/scripting/js-bindings/jswrapper/v8/debugger/SHA1.h
        cocos/scripting/js-bindings/jswrapper/v8/debugger/util-inl.h
        cocos/scripting/js-bindings/jswrapper/v8/debugger/util.cc
        cocos/scripting/js-bindings/jswrapper/v8/debugger/util.h
        cocos/scripting/js-bindings/jswrapper/v8/debugger/v8_inspector_protocol_json.h
    )
    endif()
endif()

if(USE_SE_JSC)
    cocos_source_files(
        cocos/scripting/js-bindings/jswrapper/jsc/Class.hpp
        cocos/scripting/js-bindings/jswrapper/jsc/EJConvert.h
        cocos/scripting/js-bindings/jswrapper/jsc/HelperMacros.h
        cocos/scripting/js-bindings/jswrapper/jsc/Utils.cpp
        cocos/scripting/js-bindings/jswrapper/jsc/EJConvertTypedArray.h
        cocos/scripting/js-bindings/jswrapper/jsc/PlatformUtils.h
        cocos/scripting/js-bindings/jswrapper/jsc/EJConvertTypedArray.m
        cocos/scripting/js-bindings/jswrapper/jsc/EJConvert.m
        cocos/scripting/js-bindings/jswrapper/jsc/Object.hpp
        cocos/scripting/js-bindings/jswrapper/jsc/Object.mm
        cocos/scripting/js-bindings/jswrapper/jsc/ScriptEngine.hpp
        cocos/scripting/js-bindings/jswrapper/jsc/ScriptEngine.mm
        cocos/scripting/js-bindings/jswrapper/jsc/PlatformUtils.mm
        cocos/scripting/js-bindings/jswrapper/jsc/SeApi.h
        cocos/scripting/js-bindings/jswrapper/jsc/Class.cpp
        cocos/scripting/js-bindings/jswrapper/jsc/Utils.hpp
        cocos/scripting/js-bindings/jswrapper/jsc/Base.h
    )
endif()

cocos_source_files(
    cocos/scripting/js-bindings/jswrapper/config.cpp
    cocos/scripting/js-bindings/jswrapper/config.hpp
    cocos/scripting/js-bindings/jswrapper/HandleObject.cpp
    cocos/scripting/js-bindings/jswrapper/HandleObject.hpp
    cocos/scripting/js-bindings/jswrapper/MappingUtils.cpp
    cocos/scripting/js-bindings/jswrapper/MappingUtils.hpp
    cocos/scripting/js-bindings/jswrapper/Object.hpp
    cocos/scripting/js-bindings/jswrapper/RefCounter.cpp
    cocos/scripting/js-bindings/jswrapper/RefCounter.hpp
    cocos/scripting/js-bindings/jswrapper/SeApi.h
    cocos/scripting/js-bindings/jswrapper/State.cpp
    cocos/scripting/js-bindings/jswrapper/State.hpp
    cocos/scripting/js-bindings/jswrapper/Value.cpp
    cocos/scripting/js-bindings/jswrapper/Value.hpp
)

cocos_source_files(
    cocos/scripting/js-bindings/event/CustomEventTypes.h 
    cocos/scripting/js-bindings/event/EventDispatcher.cpp
    cocos/scripting/js-bindings/event/EventDispatcher.h
)

#### storage
cocos_source_files(
    cocos/storage/local-storage/LocalStorage.cpp
    cocos/storage/local-storage/LocalStorage.h
)

if(ANDROID)
    cocos_source_files(
        cocos/storage/local-storage/LocalStorage-android.cpp
    )
endif()

#### ui
cocos_source_files(
    cocos/ui/edit-box/EditBox.h
)

if(ANDROID OR IOS) 
    if(USE_WEBVIEW)
        cocos_source_files(
            cocos/ui/videoplayer/VideoPlayer.h
        )
    endif()
    if(USE_VIDEOPLAYER)
        cocos_source_files(
            cocos/ui/webview/WebView-inl.h
            cocos/ui/webview/WebView.h
        )
    endif()
endif()

if(ANDROID)
    cocos_source_files(
        cocos/ui/edit-box/EditBox-android.cpp
    )
    if(USE_VIDEOPLAYER)
        cocos_source_files(
            cocos/ui/videoplayer/VideoPlayer-android.cpp
        )
    endif()
    if(USE_WEBVIEW)
        cocos_source_files(
            cocos/ui/webview/WebViewImpl-android.cpp
            cocos/ui/webview/WebViewImpl-android.h
        )
    endif()
elseif(MACOSX)
    cocos_source_files(
        cocos/ui/edit-box/EditBox-mac.mm
    )
elseif(IOS)
    cocos_source_files(
        cocos/ui/edit-box/EditBox-ios.mm
    )
    if(USE_VIDEOPLAYER)
        cocos_source_files(
            cocos/ui/videoplayer/VideoPlayer-ios.mm
        )
    endif()
    if(USE_WEBVIEW)
        cocos_source_files(
            cocos/ui/webview/WebViewImpl-ios.h
            cocos/ui/webview/WebViewImpl-ios.mm
        )
    endif()
elseif(WIN32)
    cocos_source_files(
        cocos/ui/edit-box/EditBox-win32.cpp
    )
endif()

### middle-ware
cocos_source_files(
    cocos/editor-support/IOBuffer.cpp
    cocos/editor-support/IOBuffer.h
    cocos/editor-support/IOTypedArray.cpp
    cocos/editor-support/IOTypedArray.h
    cocos/editor-support/MeshBuffer.cpp
    cocos/editor-support/MeshBuffer.h
    cocos/editor-support/middleware-adapter.cpp
    cocos/editor-support/middleware-adapter.h
    cocos/editor-support/MiddlewareMacro.h
    cocos/editor-support/MiddlewareManager.cpp
    cocos/editor-support/MiddlewareManager.h
    cocos/editor-support/TypedArrayPool.cpp
    cocos/editor-support/TypedArrayPool.h
)

### partical
cocos_source_files(
    cocos/editor-support/particle/ParticleSimulator.cpp
    cocos/editor-support/particle/ParticleSimulator.h
)

#### spine 
cocos_source_files(
    cocos/editor-support/spine/Animation.cpp
    cocos/editor-support/spine/Animation.h
    cocos/editor-support/spine/AnimationState.cpp
    cocos/editor-support/spine/AnimationState.h
    cocos/editor-support/spine/AnimationStateData.cpp
    cocos/editor-support/spine/AnimationStateData.h
    cocos/editor-support/spine/Atlas.cpp
    cocos/editor-support/spine/Atlas.h
    cocos/editor-support/spine/AtlasAttachmentLoader.cpp
    cocos/editor-support/spine/AtlasAttachmentLoader.h
    cocos/editor-support/spine/Attachment.cpp
    cocos/editor-support/spine/Attachment.h
    cocos/editor-support/spine/AttachmentLoader.cpp
    cocos/editor-support/spine/AttachmentLoader.h
    cocos/editor-support/spine/AttachmentTimeline.cpp
    cocos/editor-support/spine/AttachmentTimeline.h
    cocos/editor-support/spine/AttachmentType.h
    cocos/editor-support/spine/BlendMode.h
    cocos/editor-support/spine/Bone.cpp
    cocos/editor-support/spine/Bone.h
    cocos/editor-support/spine/BoneData.cpp
    cocos/editor-support/spine/BoneData.h
    cocos/editor-support/spine/BoundingBoxAttachment.cpp
    cocos/editor-support/spine/BoundingBoxAttachment.h
    cocos/editor-support/spine/ClippingAttachment.cpp
    cocos/editor-support/spine/ClippingAttachment.h
    cocos/editor-support/spine/Color.h
    cocos/editor-support/spine/ColorTimeline.cpp
    cocos/editor-support/spine/ColorTimeline.h
    cocos/editor-support/spine/Constraint.cpp
    cocos/editor-support/spine/Constraint.h
    cocos/editor-support/spine/ConstraintData.cpp
    cocos/editor-support/spine/ConstraintData.h
    cocos/editor-support/spine/ContainerUtil.h
    cocos/editor-support/spine/CurveTimeline.cpp
    cocos/editor-support/spine/CurveTimeline.h
    cocos/editor-support/spine/Debug.h
    cocos/editor-support/spine/DeformTimeline.cpp
    cocos/editor-support/spine/DeformTimeline.h
    cocos/editor-support/spine/dll.h
    cocos/editor-support/spine/DrawOrderTimeline.cpp
    cocos/editor-support/spine/DrawOrderTimeline.h
    cocos/editor-support/spine/Event.cpp
    cocos/editor-support/spine/Event.h
    cocos/editor-support/spine/EventData.cpp
    cocos/editor-support/spine/EventData.h
    cocos/editor-support/spine/EventTimeline.cpp
    cocos/editor-support/spine/EventTimeline.h
    cocos/editor-support/spine/Extension.cpp
    cocos/editor-support/spine/Extension.h
    cocos/editor-support/spine/HashMap.h
    cocos/editor-support/spine/HasRendererObject.h
    cocos/editor-support/spine/IkConstraint.cpp
    cocos/editor-support/spine/IkConstraint.h
    cocos/editor-support/spine/IkConstraintData.cpp
    cocos/editor-support/spine/IkConstraintData.h
    cocos/editor-support/spine/IkConstraintTimeline.cpp
    cocos/editor-support/spine/IkConstraintTimeline.h
    cocos/editor-support/spine/Json.cpp
    cocos/editor-support/spine/Json.h
    cocos/editor-support/spine/LinkedMesh.cpp
    cocos/editor-support/spine/LinkedMesh.h
    cocos/editor-support/spine/MathUtil.cpp
    cocos/editor-support/spine/MathUtil.h
    cocos/editor-support/spine/MeshAttachment.cpp
    cocos/editor-support/spine/MeshAttachment.h
    cocos/editor-support/spine/MixBlend.h
    cocos/editor-support/spine/MixDirection.h
    cocos/editor-support/spine/PathAttachment.cpp
    cocos/editor-support/spine/PathAttachment.h
    cocos/editor-support/spine/PathConstraint.cpp
    cocos/editor-support/spine/PathConstraint.h
    cocos/editor-support/spine/PathConstraintData.cpp
    cocos/editor-support/spine/PathConstraintData.h
    cocos/editor-support/spine/PathConstraintMixTimeline.cpp
    cocos/editor-support/spine/PathConstraintMixTimeline.h
    cocos/editor-support/spine/PathConstraintPositionTimeline.cpp
    cocos/editor-support/spine/PathConstraintPositionTimeline.h
    cocos/editor-support/spine/PathConstraintSpacingTimeline.cpp
    cocos/editor-support/spine/PathConstraintSpacingTimeline.h
    cocos/editor-support/spine/PointAttachment.cpp
    cocos/editor-support/spine/PointAttachment.h
    cocos/editor-support/spine/Pool.h
    cocos/editor-support/spine/PositionMode.h
    cocos/editor-support/spine/RegionAttachment.cpp
    cocos/editor-support/spine/RegionAttachment.h
    cocos/editor-support/spine/RotateMode.h
    cocos/editor-support/spine/RotateTimeline.cpp
    cocos/editor-support/spine/RotateTimeline.h
    cocos/editor-support/spine/RTTI.cpp
    cocos/editor-support/spine/RTTI.h
    cocos/editor-support/spine/ScaleTimeline.cpp
    cocos/editor-support/spine/ScaleTimeline.h
    cocos/editor-support/spine/ShearTimeline.cpp
    cocos/editor-support/spine/ShearTimeline.h
    cocos/editor-support/spine/Skeleton.cpp
    cocos/editor-support/spine/Skeleton.h
    cocos/editor-support/spine/SkeletonBinary.cpp
    cocos/editor-support/spine/SkeletonBinary.h
    cocos/editor-support/spine/SkeletonBounds.cpp
    cocos/editor-support/spine/SkeletonBounds.h
    cocos/editor-support/spine/SkeletonClipping.cpp
    cocos/editor-support/spine/SkeletonClipping.h
    cocos/editor-support/spine/SkeletonData.cpp
    cocos/editor-support/spine/SkeletonData.h
    cocos/editor-support/spine/SkeletonJson.cpp
    cocos/editor-support/spine/SkeletonJson.h
    cocos/editor-support/spine/Skin.cpp
    cocos/editor-support/spine/Skin.h
    cocos/editor-support/spine/Slot.cpp
    cocos/editor-support/spine/Slot.h
    cocos/editor-support/spine/SlotData.cpp
    cocos/editor-support/spine/SlotData.h
    cocos/editor-support/spine/SpacingMode.h
    cocos/editor-support/spine/spine.h
    cocos/editor-support/spine/SpineObject.cpp
    cocos/editor-support/spine/SpineObject.h
    cocos/editor-support/spine/SpineString.h
    cocos/editor-support/spine/TextureLoader.cpp
    cocos/editor-support/spine/TextureLoader.h
    cocos/editor-support/spine/Timeline.cpp
    cocos/editor-support/spine/Timeline.h
    cocos/editor-support/spine/TimelineType.h
    cocos/editor-support/spine/TransformConstraint.cpp
    cocos/editor-support/spine/TransformConstraint.h
    cocos/editor-support/spine/TransformConstraintData.cpp
    cocos/editor-support/spine/TransformConstraintData.h
    cocos/editor-support/spine/TransformConstraintTimeline.cpp
    cocos/editor-support/spine/TransformConstraintTimeline.h
    cocos/editor-support/spine/TransformMode.h
    cocos/editor-support/spine/TranslateTimeline.cpp
    cocos/editor-support/spine/TranslateTimeline.h
    cocos/editor-support/spine/Triangulator.cpp
    cocos/editor-support/spine/Triangulator.h
    cocos/editor-support/spine/TwoColorTimeline.cpp
    cocos/editor-support/spine/TwoColorTimeline.h
    cocos/editor-support/spine/Updatable.cpp
    cocos/editor-support/spine/Updatable.h
    cocos/editor-support/spine/Vector.h
    cocos/editor-support/spine/VertexAttachment.cpp
    cocos/editor-support/spine/VertexAttachment.h
    cocos/editor-support/spine/VertexEffect.cpp
    cocos/editor-support/spine/VertexEffect.h
    cocos/editor-support/spine/Vertices.h
)

cocos_source_files(
    cocos/editor-support/spine-creator-support/AttachmentVertices.cpp
    cocos/editor-support/spine-creator-support/AttachmentVertices.h
    cocos/editor-support/spine-creator-support/AttachUtil.cpp
    cocos/editor-support/spine-creator-support/AttachUtil.h
    cocos/editor-support/spine-creator-support/SkeletonAnimation.cpp
    cocos/editor-support/spine-creator-support/SkeletonAnimation.h
    cocos/editor-support/spine-creator-support/SkeletonCache.cpp
    cocos/editor-support/spine-creator-support/SkeletonCache.h
    cocos/editor-support/spine-creator-support/SkeletonCacheAnimation.cpp
    cocos/editor-support/spine-creator-support/SkeletonCacheAnimation.h
    cocos/editor-support/spine-creator-support/SkeletonCacheMgr.cpp
    cocos/editor-support/spine-creator-support/SkeletonCacheMgr.h
    cocos/editor-support/spine-creator-support/SkeletonDataMgr.cpp
    cocos/editor-support/spine-creator-support/SkeletonDataMgr.h
    cocos/editor-support/spine-creator-support/SkeletonRenderer.cpp
    cocos/editor-support/spine-creator-support/SkeletonRenderer.h
    cocos/editor-support/spine-creator-support/spine-cocos2dx.cpp
    cocos/editor-support/spine-creator-support/spine-cocos2dx.h
    cocos/editor-support/spine-creator-support/VertexEffectDelegate.cpp
    cocos/editor-support/spine-creator-support/VertexEffectDelegate.h
)


### dragon bones

cocos_source_files(
    cocos/editor-support/dragonbones/animation/Animation.cpp
    cocos/editor-support/dragonbones/animation/Animation.h
    cocos/editor-support/dragonbones/animation/AnimationState.cpp
    cocos/editor-support/dragonbones/animation/AnimationState.h
    cocos/editor-support/dragonbones/animation/BaseTimelineState.cpp
    cocos/editor-support/dragonbones/animation/BaseTimelineState.h
    cocos/editor-support/dragonbones/animation/IAnimatable.h
    cocos/editor-support/dragonbones/animation/TimelineState.cpp
    cocos/editor-support/dragonbones/animation/TimelineState.h
    cocos/editor-support/dragonbones/animation/WorldClock.cpp
    cocos/editor-support/dragonbones/animation/WorldClock.h
    cocos/editor-support/dragonbones/armature/Armature.cpp
    cocos/editor-support/dragonbones/armature/Armature.h
    cocos/editor-support/dragonbones/armature/Bone.cpp
    cocos/editor-support/dragonbones/armature/Bone.h
    cocos/editor-support/dragonbones/armature/Constraint.cpp
    cocos/editor-support/dragonbones/armature/Constraint.h
    cocos/editor-support/dragonbones/armature/DeformVertices.cpp
    cocos/editor-support/dragonbones/armature/DeformVertices.h
    cocos/editor-support/dragonbones/armature/IArmatureProxy.h
    cocos/editor-support/dragonbones/armature/Slot.cpp
    cocos/editor-support/dragonbones/armature/Slot.h
    cocos/editor-support/dragonbones/armature/TransformObject.cpp
    cocos/editor-support/dragonbones/armature/TransformObject.h
    cocos/editor-support/dragonbones/core/BaseObject.cpp
    cocos/editor-support/dragonbones/core/BaseObject.h
    cocos/editor-support/dragonbones/core/DragonBones.cpp
    cocos/editor-support/dragonbones/core/DragonBones.h
    cocos/editor-support/dragonbones/DragonBonesHeaders.h
    cocos/editor-support/dragonbones/event/EventObject.cpp
    cocos/editor-support/dragonbones/event/EventObject.h
    cocos/editor-support/dragonbones/event/IEventDispatcher.h
    cocos/editor-support/dragonbones/factory/BaseFactory.cpp
    cocos/editor-support/dragonbones/factory/BaseFactory.h
    cocos/editor-support/dragonbones/geom/ColorTransform.h
    cocos/editor-support/dragonbones/geom/Matrix.h
    cocos/editor-support/dragonbones/geom/Point.cpp
    cocos/editor-support/dragonbones/geom/Point.h
    cocos/editor-support/dragonbones/geom/Rectangle.h
    cocos/editor-support/dragonbones/geom/Transform.cpp
    cocos/editor-support/dragonbones/geom/Transform.h
    cocos/editor-support/dragonbones/model/AnimationConfig.cpp
    cocos/editor-support/dragonbones/model/AnimationConfig.h
    cocos/editor-support/dragonbones/model/AnimationData.cpp
    cocos/editor-support/dragonbones/model/AnimationData.h
    cocos/editor-support/dragonbones/model/ArmatureData.cpp
    cocos/editor-support/dragonbones/model/ArmatureData.h
    cocos/editor-support/dragonbones/model/BoundingBoxData.cpp
    cocos/editor-support/dragonbones/model/BoundingBoxData.h
    cocos/editor-support/dragonbones/model/CanvasData.cpp
    cocos/editor-support/dragonbones/model/CanvasData.h
    cocos/editor-support/dragonbones/model/ConstraintData.cpp
    cocos/editor-support/dragonbones/model/ConstraintData.h
    cocos/editor-support/dragonbones/model/DisplayData.cpp
    cocos/editor-support/dragonbones/model/DisplayData.h
    cocos/editor-support/dragonbones/model/DragonBonesData.cpp
    cocos/editor-support/dragonbones/model/DragonBonesData.h
    cocos/editor-support/dragonbones/model/SkinData.cpp
    cocos/editor-support/dragonbones/model/SkinData.h
    cocos/editor-support/dragonbones/model/TextureAtlasData.cpp
    cocos/editor-support/dragonbones/model/TextureAtlasData.h
    cocos/editor-support/dragonbones/model/UserData.cpp
    cocos/editor-support/dragonbones/model/UserData.h
    cocos/editor-support/dragonbones/parser/BinaryDataParser.cpp
    cocos/editor-support/dragonbones/parser/BinaryDataParser.h
    cocos/editor-support/dragonbones/parser/DataParser.cpp
    cocos/editor-support/dragonbones/parser/DataParser.h
    cocos/editor-support/dragonbones/parser/JSONDataParser.cpp
    cocos/editor-support/dragonbones/parser/JSONDataParser.h
)

cocos_source_files(
    cocos/editor-support/dragonbones-creator-support/ArmatureCache.cpp
    cocos/editor-support/dragonbones-creator-support/ArmatureCache.h
    cocos/editor-support/dragonbones-creator-support/ArmatureCacheMgr.cpp
    cocos/editor-support/dragonbones-creator-support/ArmatureCacheMgr.h
    cocos/editor-support/dragonbones-creator-support/AttachUtil.cpp
    cocos/editor-support/dragonbones-creator-support/AttachUtil.h
    cocos/editor-support/dragonbones-creator-support/CCArmatureCacheDisplay.cpp
    cocos/editor-support/dragonbones-creator-support/CCArmatureCacheDisplay.h
    cocos/editor-support/dragonbones-creator-support/CCArmatureDisplay.cpp
    cocos/editor-support/dragonbones-creator-support/CCArmatureDisplay.h
    cocos/editor-support/dragonbones-creator-support/CCDragonBonesHeaders.h
    cocos/editor-support/dragonbones-creator-support/CCFactory.cpp
    cocos/editor-support/dragonbones-creator-support/CCFactory.h
    cocos/editor-support/dragonbones-creator-support/CCSlot.cpp
    cocos/editor-support/dragonbones-creator-support/CCSlot.h
    cocos/editor-support/dragonbones-creator-support/CCTextureAtlasData.cpp
    cocos/editor-support/dragonbones-creator-support/CCTextureAtlasData.h
)

##### extensions

cocos_source_files(
    extensions/assets-manager/AssetsManagerEx.cpp
    extensions/assets-manager/AssetsManagerEx.h
    extensions/assets-manager/CCAsyncTaskPool.cpp
    extensions/assets-manager/CCAsyncTaskPool.h
    extensions/assets-manager/CCEventAssetsManagerEx.cpp
    extensions/assets-manager/CCEventAssetsManagerEx.h
    extensions/assets-manager/Manifest.cpp
    extensions/assets-manager/Manifest.h
    extensions/cocos-ext.h
    extensions/ExtensionExport.h
    extensions/ExtensionMacros.h
)

list(APPEND COCOS2D_SOURCE_LIST ${CC_EXTERNAL_SROUCES})

add_library(cocos2d ${COCOS2D_SOURCE_LIST})

# setup default flags
target_compile_definitions(cocos2d PUBLIC 
    $<$<NOT:$<BOOL:${USE_VIDEO}>>:USE_VIDEO=0>
    $<$<NOT:$<BOOL:${USE_WEBVIEW}>>:USE_WEB_VIEW=0>
    $<$<NOT:$<BOOL:${USE_AUDIO}>>:USE_AUDIO=0>
    $<$<NOT:$<BOOL:${USE_SOCKET}>>:USE_SOCKET=0>
    $<$<NOT:$<BOOL:${USE_TIFF}>>:CC_USE_TIFF=0>
    $<$<NOT:$<BOOL:${USE_V8_DEBUGGER}>>:USE_V8_DEBUGGER=0>
    $<$<BOOL:${USE_SE_JSC}>:SCRIPT_ENGINE_TYPE=3>
    $<$<CONFIG:Debug>:COCOS2D_DEBUG=1>
)

target_compile_definitions(cocos2d 
    PUBLIC USE_FILE32API
)

target_include_directories(cocos2d PUBLIC 
    ${CC_EXTERNAL_INCLUDES}
    ${CWD}
    ${CWD}/cocos
    ${CWD}/cocos/renderer
    ${CWD}/cocos/platform
    ${CWD}/cocos/renderer/core
    ${CWD}/cocos/renderer/gfx
    ${CWD}/external/sources/khronos
    ${CWD}/cocos/editor-support
    $<$<BOOL:USE_SE_JSC>:${CWD}/cocos/scripting/js-bindings/jswrapper>
)

if(WIN32)
    target_link_libraries(cocos2d PUBLIC
        ws2_32 userenv psapi winmm Version Iphlpapi opengl32
        ${CC_EXTERNAL_LIBS}
    )

    target_compile_definitions(cocos2d PUBLIC
        CC_STATIC 
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

if(ANDROID)
    find_library(LIB_EGL NAMES EGL)
    find_library(LIB_GLESv2 NAMES GLESv2)
    target_link_libraries(cocos2d PUBLIC
        android
        log
        OpenSLES
        ${LIB_EGL}
        ${LIB_GLESv2}
        GLESv2
        ${CC_EXTERNAL_LIBS}
    )
endif()

if(APPLE) 

    target_compile_options(cocos2d 
        PRIVATE  -Wno-objc-method-access 
    )

    target_include_directories(cocos2d PUBLIC
        ${CWD}/cocos/platform/ios
    )

    find_library(ICONV_LIBRARY iconv)
    find_library(AUDIOTOOLBOX_LIBRARY AudioToolBox)
    find_library(FOUNDATION_LIBRARY Foundation)
    find_library(OPENAL_LIBRARY OpenAL)
    find_library(GAMECONTROLLER_LIBRARY GameController)
    find_library(METAL_LIBRARY Metal)
    find_library(METALLIT_LIBRARY MetalKit)
    find_library(SQLITE3_LIBRARY SQLite3)
    find_library(SECURITY_LIBRARY Security)
    find_library(SystemConfiguration_LIBRARY SystemConfiguration)
    target_link_libraries(cocos2d PUBLIC
        ${ICONV_LIBRARY}
        ${AUDIOTOOLBOX_LIBRARY}
        ${FOUNDATION_LIBRARY}
        ${OPENAL_LIBRARY}
        ${GAMECONTROLLER_LIBRARY}
        ${METAL_LIBRARY}
        ${METALLIT_LIBRARY}
        ${SQLITE3_LIBRARY}
        ${SECURITY_LIBRARY}
        ${SystemConfiguration_LIBRARY}
        ${CC_EXTERNAL_LIBS}
    )
    if(MACOSX)
        find_library(OPENGL_LIBRARY OpenGL)
        find_library(APPKIT_LIBRARY AppKit)
        find_library(IOKIT_LIBRARY IOKit)
        find_library(COREVIDEO_LIBRARY CoreVideo)
        target_link_libraries(cocos2d PUBLIC
            ${OPENGL_LIBRARY}
            ${APPKIT_LIBRARY}
            ${IOKIT_LIBRARY}
            ${COREVIDEO_LIBRARY}
        )
        target_compile_definitions(cocos2d PUBLIC 
            CC_KEYBOARD_SUPPORT
            )
    elseif(IOS)
        find_library(QUARTZCORE_LIBRARY QuartzCore)
        find_library(UIKIT_LIBRARY UIKit)
        find_library(CORE_MOTION_LIBRARY CoreMotion)
        find_library(CFNETWORK_LIBRARY CFNetwork)
        find_library(CORE_MEDIA_LIBRARY CoreMedia)
        find_library(CORE_TEXT_LIBRARY CoreText)
        find_library(CORE_GRAPHICS_LIBRARY CoreGraphics)
        find_library(AV_FOUNDATION_LIBRARY AVFoundation)
        find_library(ZLIB_LIBRARY z)
        find_library(OPENGLES_LIBRARY OpenGLES)
        find_library(JAVASCRIPTCORE_LIBRARY JavaScriptCore)
        target_link_libraries(cocos2d PUBLIC
            ${QUARTZCORE_LIBRARY}
            ${UIKIT_LIBRARY}
            ${CFNETWORK_LIBRARY}
            ${ZLIB_LIBRARY}
            ${CORE_GRAPHICS_LIBRARY}
            ${AV_FOUNDATION_LIBRARY}
            ${CORE_TEXT_LIBRARY}
            ${CORE_MOTION_LIBRARY}
            ${OPENGLES_LIBRARY}
            ${JAVASCRIPTCORE_LIBRARY}
        )
        set_property(TARGET cocos2d PROPERTY XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET ${TARGET_IOS_VERSION})
        set_property(TARGET cocos2d PROPERTY XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")
        set_property(TARGET cocos2d PROPERTY XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "YES")
    endif()

endif()

source_group(TREE ${CWD} PREFIX "Source Files" FILES ${COCOS2D_SOURCE_LIST})


if(MSVC)
    file(GLOB_RECURSE WIN32_DLLS 
        ${CWD}/external/win32/libs/*.dll 
        ${CWD}/external/win32/libs/Debug/*.dll
    )
    target_compile_options(cocos2d PUBLIC /MP)
endif()