/****************************************************************************
Copyright (c) 2020 Xiamen Yaji Software Co., Ltd.

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
#include "platform/ohos/jni/JniCocosAbility.h"
#include "base/Log.h"
#include "platform/Application.h"
#include "platform/android/View.h"
#include "platform/java/jni/JniHelper.h"
#include "platform/ohos/FileUtils-ohos.h"
#include "platform/ohos/jni/AbilityConsts.h"
#include <fcntl.h>
#include <future>
#include <hilog/log.h>
#include <jni.h>
#include <native_layer.h>
#include <native_layer_jni.h>
#include <netdb.h>
#include <platform/FileUtils.h>
#include <thread>
#include <unistd.h>
#include <vector>

#define LOGV(...) HILOG_INFO(LOG_APP, __VA_ARGS__)

extern "C" cc::Application *cocos_main(int, int) __attribute__((weak));

namespace cc {
CocosApp cocosApp;
}

using namespace cc::ohos;

namespace {
int messagePipe[2];
int pipeRead = 0;
int pipeWrite = 0;
cc::Application *game = nullptr;

struct CommandMsg {
    int cmd;
    std::function<void()> callback;
};

void createGame(NativeLayer *window) {
    int width = NativeLayerHandle(window, NativeLayerOps::GET_WIDTH);
    int height = NativeLayerHandle(window, NativeLayerOps::GET_HEIGHT);
    game = cocos_main(width, height);
    game->init();
}

void writeCommandAsync(int cmd) {
    CommandMsg msg{.cmd = cmd, .callback = nullptr};
    write(pipeWrite, &msg, sizeof(msg));
}

void writeCommandSync(int cmd) {
    std::promise<void> fu;
    CommandMsg msg{.cmd = cmd, .callback = [&fu]() {
                       fu.set_value();
                   }};
    write(pipeWrite, &msg, sizeof(msg));
    fu.get_future().get();
}

int readCommand(CommandMsg &msg) {
    return read(pipeRead, &msg, sizeof(msg));
}

void handlePauseResume(int8_t cmd) {
    LOGV("activityState=%d", cmd);
    cc::cocosApp.activityState = cmd;
}

void preExecCmd(int cmd) {
    switch (cmd) {
        case ABILITY_CMD_INIT_WINDOW: {
            LOGV("ABILITY_CMD_INIT_WINDOW");
            cc::cocosApp.window = cc::cocosApp.pendingWindow;
            cc::cocosApp.animating = true;

            if (!game) {
                createGame(cc::cocosApp.window);
            }
        } break;
        case ABILITY_CMD_TERM_WINDOW:
            LOGV("ABILITY_CMD_TERM_WINDOW");
            cc::cocosApp.animating = false;
            break;
        case ABILITY_CMD_RESUME:
            LOGV("ABILITY_CMD_RESUME");
            handlePauseResume(cmd);
            break;
        case ABILITY_CMD_PAUSE:
            LOGV("ABILITY_CMD_PAUSE");
            handlePauseResume(cmd);
            break;
        case ABILITY_CMD_DESTROY:
            LOGV("ABILITY_CMD_DESTROY");
            cc::cocosApp.destroyRequested = true;
            break;
        default:
            break;
    }
}

void postExecCmd(int cmd) {
    switch (cmd) {
        case ABILITY_CMD_TERM_WINDOW:
            cc::cocosApp.window = nullptr;
            break;
        default:
            break;
    }
}

void glThreadEntry() {
    cc::cocosApp.glThreadPromise.set_value();
    cc::cocosApp.running = true;

    int cmd = 0;
    CommandMsg msg;
    while (1) {
        if (readCommand(msg) > 0) {
            cmd = msg.cmd;
            preExecCmd(cmd);
            cc::View::engineHandleCmd(cmd);
            postExecCmd(cmd);
            if (msg.callback) {
                msg.callback();
            }
        }

        if (!cc::cocosApp.animating) continue;

        if (game) {
            // Handle java events send by UI thread. Input events are handled here too.
            cc::JniHelper::callStaticVoidMethod("com.cocos.lib.CocosHelper",
                                                "flushTasksOnGameThread");
            game->tick();
        }

        if (cc::cocosApp.destroyRequested) break;
    }

    close(pipeRead);
    close(pipeWrite);

    delete game;
    game = nullptr;
}

void setWindow(NativeLayer *window) {
    if (cc::cocosApp.pendingWindow) {
        writeCommandSync(ABILITY_CMD_TERM_WINDOW);
    }
    cc::cocosApp.pendingWindow = window;
}

void tryInitGame() {
    if (cc::cocosApp.pendingWindow) {
        writeCommandSync(ABILITY_CMD_INIT_WINDOW);
    }
}

void setActivityState(int8_t cmd) {
    writeCommandSync(cmd);
}
} // namespace

extern "C" {

JNIEXPORT void JNICALL Java_com_cocos_lib_CocosAbilitySlice_onCreateNative(JNIEnv *env, jobject obj, jobject ability,
                                                                           jstring assetPath, jobject resourceManager,
                                                                           jint sdkVersion) {
    if (cc::cocosApp.running) {
        return;
    }

    jboolean isCopy = false;
    const char *assetPathStr = env->GetStringUTFChars(assetPath, &isCopy);

    cc::cocosApp.sdkVersion = sdkVersion;
    cc::JniHelper::init(env, ability);
    cc::cocosApp.resourceManager = InitNativeResourceManager(env, resourceManager);
    cc::FileUtilsOHOS::initResourceManager(cc::cocosApp.resourceManager, assetPathStr);

    if (isCopy) {
        env->ReleaseStringUTFChars(assetPath, assetPathStr);
        assetPathStr = nullptr;
    }

    if (pipe(messagePipe)) {
        LOGV("Can not create pipe: %s", strerror(errno));
    }
    pipeRead = messagePipe[0];
    pipeWrite = messagePipe[1];
    if (fcntl(pipeRead, F_SETFL, O_NONBLOCK) < 0) {
        LOGV("Can not make pipe read to non blocking mode.");
    }

    std::thread glThread(glThreadEntry);
    glThread.detach();

    cc::cocosApp.glThreadPromise.get_future().get();
}

JNIEXPORT void JNICALL
Java_com_cocos_lib_CocosAbilitySlice_onSurfaceCreatedNative(JNIEnv *env, jobject obj, jobject surface) {
    //    setWindow(GetNativeLayer(env, surface));
}
JNIEXPORT void JNICALL
Java_com_cocos_lib_CocosAbilitySlice_onSurfaceChangedNative(JNIEnv *env, jobject obj, jobject surface, jint x,
                                                            jint width, jint height) {
    setWindow(GetNativeLayer(env, surface));
    tryInitGame();
}

JNIEXPORT void JNICALL Java_com_cocos_lib_CocosAbilitySlice_onSurfaceDestroyNative(JNIEnv *env, jobject obj) {
    setWindow(nullptr);
}

JNIEXPORT void JNICALL Java_com_cocos_lib_CocosAbilitySlice_onStartNative(JNIEnv *env, jobject obj) {
}

JNIEXPORT void JNICALL Java_com_cocos_lib_CocosAbilitySlice_onPauseNative(JNIEnv *env, jobject obj) {
    setActivityState(ABILITY_CMD_PAUSE);
}

JNIEXPORT void JNICALL Java_com_cocos_lib_CocosAbilitySlice_onResumeNative(JNIEnv *env, jobject obj) {
    setActivityState(ABILITY_CMD_RESUME);
}

JNIEXPORT void JNICALL Java_com_cocos_lib_CocosAbilitySlice_onStopNative(JNIEnv *env, jobject obj) {
}

JNIEXPORT void JNICALL Java_com_cocos_lib_CocosAbilitySlice_onLowMemoryNative(JNIEnv *env, jobject obj) {
    writeCommandSync(ABILITY_CMD_LOW_MEMORY);
}

JNIEXPORT void JNICALL Java_com_cocos_lib_CocosAbilitySlice_onOrientationChangedNative(JNIEnv *env, jobject obj, jint orientation, jint width, jint height) {
    cc::EventDispatcher::dispatchResizeEvent(width, height);
}

JNIEXPORT void JNICALL
Java_com_cocos_lib_CocosAbilitySlice_onWindowFocusChangedNative(JNIEnv *env, jobject obj, jboolean has_focus) {
}
}
