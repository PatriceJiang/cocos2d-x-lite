package org.cocos2dx.test;

import android.util.Log;

import org.cocos2dx.lib.ByteCodeGenerator;

public class Sample {
    public static void runTest() {
        ByteCodeGenerator bc = new ByteCodeGenerator();
        bc.setName("pkg/Test");
        bc.addInterface("java/lang/Runnable");
        Class<?> kls = bc.build();
        if(kls != null) {
            Log.e("Sample", kls.getCanonicalName());
            try {
                Runnable r = (Runnable) kls.newInstance();
                r.run();
            } catch (InstantiationException e) {
                e.printStackTrace();
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            }
        }
    }
}
