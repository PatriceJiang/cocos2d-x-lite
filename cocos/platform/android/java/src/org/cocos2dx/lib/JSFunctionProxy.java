package org.cocos2dx.lib;

import java.util.concurrent.atomic.AtomicBoolean;

public class JSFunctionProxy {

    final static int MAX_WAIT = 300000;
    final static int TICK = 3; //millis

    public static void printArgumentsV(final Object[] args) {
        String methodName = Thread.currentThread().getStackTrace()[3].getMethodName();

        final AtomicBoolean finished = new AtomicBoolean(false);

        if (Cocos2dxHelper.inRendererThread()) {
            ByteCodeGenerator.callJS(finished, methodName, args);
            return;
        } else {
            Cocos2dxHelper.runOnGLThread(new Runnable() {
                @Override
                public void run() {
                    ByteCodeGenerator.callJS(finished, methodName, args);
                }
            });
            int maxWait = MAX_WAIT;
            while (!finished.get() && maxWait > 0) {
                try {
                    Thread.sleep(TICK);
                    maxWait -= TICK;
                } catch (InterruptedException e) {
                    e.printStackTrace();
                    break;
                }
            }
        }
    }
}
