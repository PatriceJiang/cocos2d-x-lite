package org.cocos2dx.test;

import android.util.Log;

import org.cocos2dx.lib.ByteCodeGenerator;
import org.cocos2dx.lib.Cocos2dxHelper;

import java.util.concurrent.atomic.AtomicBoolean;

import static java.lang.Thread.currentThread;
import static java.lang.Thread.sleep;

public class Main {

    final static int MAX_WAIT = 3000;
    final static int TICK = 3; //millis
    public static void printArgumentsV(final Object [] args) {
        String methodName = Thread.currentThread().getStackTrace()[3].getMethodName();
        Log.e("Inspect function ", methodName);
        for(Object x : args) {
            Log.e("  Inspect arguments", x.toString());
        }

        final AtomicBoolean finished = new AtomicBoolean(false);

        if(Cocos2dxHelper.inRendererThread()) {
            ByteCodeGenerator.callJS(finished, methodName, args);
        }else {
            Cocos2dxHelper.runOnGLThread(new Runnable() {
                @Override
                public void run() {
                    ByteCodeGenerator.callJS(finished, methodName, args);
                }
            });
        }
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
