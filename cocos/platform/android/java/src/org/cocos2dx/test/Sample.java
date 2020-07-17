package org.cocos2dx.test;

import android.util.Log;

import org.cocos2dx.lib.ByteCodeGenerator;

public class Sample {
    public static void runTest() {
        ByteCodeGenerator bc = new ByteCodeGenerator();
        bc.setName("pkg/Test");
        bc.addInterface("java/lang/Runnable");
        bc.addInterface("org/cocos2dx/test/Sample$Sum");
        Class<?> kls = bc.build();
        if(kls != null) {
            Log.e("Sample", kls.getCanonicalName());
            try {
                Runnable r = (Runnable) kls.newInstance();
                r.run();
                Sum s  = (Sum) r;
                s.add(1, 4);
            } catch (InstantiationException e) {
                e.printStackTrace();
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            }
        }
    }

    public static interface Sum {
        void add(int a, int b);
    }

}
