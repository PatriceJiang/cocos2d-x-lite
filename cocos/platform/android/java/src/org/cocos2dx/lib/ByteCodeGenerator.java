package org.cocos2dx.lib;


import android.util.Log;

import com.android.dx.Code;
import com.android.dx.DexMaker;
import com.android.dx.FieldId;
import com.android.dx.Local;
import com.android.dx.MethodId;
import com.android.dx.TypeId;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.lang.reflect.Array;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.lang.reflect.Type;
import java.math.BigInteger;
import java.security.MessageDigest;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.jar.JarEntry;
import java.util.jar.JarOutputStream;

import javax.xml.validation.TypeInfoProvider;

public class ByteCodeGenerator {


    public static final String NATIVE_ID = "__native_id__";
    public static final String PROXY_CLASS_NAME = "org/cocos2dx/lib/JSFunctionProxy";
    public static final String PROXY_METHOD_NAME = "dispatchToJS_";
    public static final String LOGCAT_TAG = "Bytecode Generator";

    static int object_id = 10000;

    private static Map<String, ClassLoader> classLoaders = new ConcurrentHashMap<>();

    static class MethodRecord {
        public int modifiers;
        public String signature;
        public Method method;
        public String name;
    }


    public static <T, G extends T> Object newInstance(String className, String[] interfaces) {

        ByteCodeGenerator bc = new ByteCodeGenerator();
//        bc.setClassName(name);
        bc.setSuperClass(className);
        bc.setInterfaces(interfaces);
        bc.setClassName("pkg/anonymous/K_" + bc.getHashKeyForAnonymousClass());

        DexMaker maker = new DexMaker();
        TypeId<?> superClass = TypeId.get("L" + bc.superClass + ";");
        TypeId<?>[] interfaceTypes = new TypeId<?>[bc.interfaces.size()];
        for (int i = 0; i < bc.interfaces.size(); i++) {
            String interfaceName = bc.interfaces.get(i);
            interfaceTypes[i] = TypeId.get("L" + interfaceName + ";");
        }
        TypeId<?> generatedType = TypeId.get("L" + className + ";");
        maker.declare(generatedType, bc.getClassNameWithDots() + ".generated", Modifier.PUBLIC, superClass, interfaceTypes);

        FieldId<?, Integer> native_id = generatedType.getField(TypeId.INT, NATIVE_ID);

        MethodId<T, ?> superInit = (MethodId<T, ?>) superClass.getConstructor();
        MethodId<?, ?> initMethod = generatedType.getConstructor();

        Code constructorCode = maker.declare(initMethod, Modifier.PUBLIC);
        Local<Integer> id = constructorCode.newLocal(TypeId.INT);
        Local<G> thisObj = (Local<G>) constructorCode.getThis(generatedType);
        constructorCode.invokeDirect(superInit, null, thisObj);
        MethodId<ByteCodeGenerator, Integer> getIdMethod = TypeId.get(ByteCodeGenerator.class).getMethod(TypeId.INT, "getId");
        constructorCode.invokeStatic(getIdMethod, id);
        constructorCode.sput(native_id, id);
        MethodId<ByteCodeGenerator, Void> registerThis = TypeId.get(ByteCodeGenerator.class).getMethod(TypeId.VOID, "registerInstance", TypeId.OBJECT, TypeId.INT);
        constructorCode.invokeStatic(registerThis, null, thisObj, id);
        constructorCode.returnVoid();

    /*
        List<MethodRecord> virtualMethods = bc.findAllVirtualMethod();
        for (MethodRecord m : virtualMethods) {
            TypeId<?> returnType = TypeId.get(m.method.getReturnType());
            TypeId<?> argTypes[] = getMethodArguments(m.method);
            MethodId<?, ?> mId = generatedType.getMethod(returnType, m.name, argTypes);
            MethodId<Object[], Void> arrayConstructor = TypeId.get(Object[].class).getConstructor(TypeId.INT);
//            MethodId<List, Void> arrayAdd = TypeId.get(List.class).getMethod(TypeId.VOID, "add", TypeId.OBJECT);

            int modifier = m.modifiers & (~Modifier.ABSTRACT);
            Code funcCode = maker.declare(mId, modifier);

            Local<?> funcThisObj = funcCode.getThis(generatedType);
            Local<Object[]> arglist = funcCode.newLocal(TypeId.get(Object[].class));
            Local<Integer> nativeId = funcCode.newLocal(TypeId.INT);
            Local<?> finalRet = funcCode.newLocal(returnType);

            Local<Integer> intRet = null;
            Local<Boolean> boolRet = null;
            Local<Short> shortRet = null;
            Local<Byte> byteRet = null;
            Local<Character> charRet = null;
            Local<Long> longRet = null;
            Local<Float> floatRet= null;
            Local<Double> doubleRet = null;
            Local<Object> objectRet = null;
            if (returnType == TypeId.INT) {
                intRet = funcCode.newLocal(TypeId.INT);
            } else if (returnType == TypeId.BOOLEAN) {
                boolRet = funcCode.newLocal(TypeId.BOOLEAN);
            } else if (returnType == TypeId.SHORT) {
                shortRet = funcCode.newLocal(TypeId.SHORT);
            } else if (returnType == TypeId.BYTE) {
                byteRet = funcCode.newLocal(TypeId.BYTE);
            } else if (returnType == TypeId.CHAR) {
                charRet = funcCode.newLocal(TypeId.CHAR);
            } else if (returnType == TypeId.LONG) {
                longRet = funcCode.newLocal(TypeId.LONG);
            } else if (returnType == TypeId.FLOAT) {
                floatRet = funcCode.newLocal(TypeId.FLOAT);
            } else if (returnType == TypeId.DOUBLE) {
                doubleRet = funcCode.newLocal(TypeId.DOUBLE);
            } else if (returnType != TypeId.VOID) {
                objectRet = funcCode.newLocal(TypeId.OBJECT);
            }

            Local<Integer> arraySize = funcCode.newLocal(TypeId.INT);
            Local<Integer> arrayIdx = funcCode.newLocal(TypeId.INT);

            funcCode.loadConstant(arraySize, argTypes.length + 2);
            funcCode.loadConstant(arrayIdx, 0);


            funcCode.invokeDirect(arrayConstructor, null, arglist, arraySize);
            funcCode.invokeStatic(getIdMethod, nativeId);
            funcCode.aput(arglist, arrayIdx, nativeId);
            funcCode.loadConstant(arrayIdx, 1);
            funcCode.aput(arglist, arrayIdx, funcThisObj);

            for (int i = 0; i < argTypes.length; i++) {
                funcCode.loadConstant(arrayIdx, 2 + i);
                funcCode.aput(arglist, arrayIdx, funcCode.getParameter(i, argTypes[i]));
//                funcCode.invokeVirtual(arrayAdd, null, arglist, funcCode.getParameter(i, argTypes[i]));
            }

            final String dispatchPrefix = "dispatchToJS_";

            if (returnType == TypeId.VOID) {
                MethodId<?, ?> dispatch = TypeId.get(JSFunctionProxy.class).getMethod(TypeId.VOID, dispatchPrefix + "V", TypeId.get(Object[].class));
                funcCode.invokeStatic(dispatch, null, arglist);
                funcCode.returnVoid();
            } else if (returnType == TypeId.INT) {
                MethodId<?, Integer> dispatch = TypeId.get(JSFunctionProxy.class).getMethod(TypeId.INT, dispatchPrefix + "I", TypeId.get(Object[].class));
                funcCode.invokeStatic(dispatch, intRet, arglist);
                funcCode.returnValue(intRet);
            } else if (returnType == TypeId.BOOLEAN) {
                MethodId<?, Boolean> dispatch = TypeId.get(JSFunctionProxy.class).getMethod(TypeId.BOOLEAN, dispatchPrefix + "Z", TypeId.get(Object[].class));
                funcCode.invokeStatic(dispatch, boolRet, arglist);
                funcCode.returnValue(boolRet);
            } else if (returnType == TypeId.SHORT) {
                MethodId<?, Short> dispatch = TypeId.get(JSFunctionProxy.class).getMethod(TypeId.SHORT, dispatchPrefix + "S", TypeId.get(Object[].class));
                funcCode.invokeStatic(dispatch, shortRet, arglist);
                funcCode.returnValue(shortRet);
            } else if (returnType == TypeId.BYTE) {
                MethodId<?, Byte> dispatch = TypeId.get(JSFunctionProxy.class).getMethod(TypeId.BYTE, dispatchPrefix + "B", TypeId.get(Object[].class));
                funcCode.invokeStatic(dispatch, byteRet, arglist);
                funcCode.returnValue(byteRet);
            } else if (returnType == TypeId.CHAR) {
                MethodId<?, Character> dispatch = TypeId.get(JSFunctionProxy.class).getMethod(TypeId.CHAR, dispatchPrefix + "C", TypeId.get(Object[].class));
                funcCode.invokeStatic(dispatch, charRet, arglist);
                funcCode.returnValue(charRet);
            } else if (returnType == TypeId.LONG) {
                MethodId<?, Long> dispatch = TypeId.get(JSFunctionProxy.class).getMethod(TypeId.LONG, dispatchPrefix + "J", TypeId.get(Object[].class));
                funcCode.invokeStatic(dispatch, longRet, arglist);
                funcCode.returnValue(longRet);
            } else if (returnType == TypeId.FLOAT) {
                MethodId<?, Float> dispatch = TypeId.get(JSFunctionProxy.class).getMethod(TypeId.FLOAT, dispatchPrefix + "F", TypeId.get(Object[].class));
                funcCode.invokeStatic(dispatch, floatRet, arglist);
                funcCode.returnValue(floatRet);
            } else if (returnType == TypeId.DOUBLE) {
                MethodId<?, Double> dispatch = TypeId.get(JSFunctionProxy.class).getMethod(TypeId.DOUBLE, dispatchPrefix + "D", TypeId.get(Object[].class));
                funcCode.invokeStatic(dispatch, doubleRet, arglist);
                funcCode.returnValue(doubleRet);
            } else {
                MethodId<?, Object> dispatch = TypeId.get(JSFunctionProxy.class).getMethod(TypeId.OBJECT, dispatchPrefix + "L", TypeId.get(Object[].class));
                funcCode.invokeStatic(dispatch, objectRet, arglist);
                funcCode.cast(finalRet, objectRet);
                funcCode.returnValue(finalRet);
            }
        }
    */
        // Create the dex file and load it.
        File outputDir = new File(getJarCacheDir(), bc.getHashKeyForAnonymousClass());
        if(!outputDir.exists()){
            outputDir.mkdirs();
        }

        File jarDir = getGenClassDir();
        if(!jarDir.exists()) {
            jarDir.mkdirs();
        }


        try {

            File fp = File.createTempFile("DexMaker",".dex", jarDir);
            FileOutputStream fos = new FileOutputStream(fp);
            fos.write(maker.generate());
            fos.close();
            ClassLoader loader = maker.generateAndLoad(ByteCodeGenerator.class.getClassLoader(), outputDir);
            Class<?> generatedClass = loader.loadClass(bc.getClassNameWithDots());
            return generatedClass.newInstance();
        } catch (IOException | ClassNotFoundException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (InstantiationException e) {
            e.printStackTrace();
        }

        return null;
    }

    public static TypeId<?>[] getMethodArguments(Method m) {
        Class<?> types[] = m.getParameterTypes();
        TypeId<?> ret[] = new TypeId<?>[types.length];
        for (int i = 0; i < types.length; i++) {
            ret[i] = TypeId.get(types[i]);
        }
        return ret;
    }


    private String superClass = "java/lang/Object";
    private ArrayList<String> interfaces = new ArrayList<>();
    private String className;

    public ByteCodeGenerator() {
    }

    public void setClassName(String className) {
        if (className != null && !className.isEmpty()) {
            this.className = className.replaceAll("\\.", "/");
        }
    }

    public String getClassNameWithDots() {
        return className.replaceAll("/", ".");
    }

    public void setSuperClass(String superClass) {
        if (superClass != null && !superClass.isEmpty()) {
            this.superClass = superClass;
        }
    }

    public String getSuperClassDot() {
        return superClass.replaceAll("/", ".");
    }

    public void setInterfaces(String interfaces[]) {
        for (String itf : interfaces) {
            this.interfaces.add(itf.replaceAll("\\.", "/"));
        }
    }

    /**
     * ID for objects generated by `newInstance`
     *
     * @return ID
     */
    public static int getId() {
        return object_id++;
    }

    /**
     * Create folders for bytecode/class data
     */
    static public void init() {
//        renameAndDelete(ByteCodeGenerator.getDexDir());
//        renameAndDelete(ByteCodeGenerator.getGenClassDir());
//        renameAndDelete(ByteCodeGenerator.getJarCacheDir());
    }

    private static void renameAndDelete(File dir) {
        if (!dir.exists()) return;
        if (dir.isDirectory()) {
            String oldPath = dir.getAbsolutePath();
            if (oldPath.endsWith("/")) oldPath = oldPath.substring(0, oldPath.length() - 1);
            File tmpDir = new File(oldPath + "_to_delete");
            if (tmpDir.exists()) deleteFilesInFolder(tmpDir);
            dir.renameTo(tmpDir);
            new Thread(new Runnable() {
                @Override
                public void run() {
                    deleteFilesInFolder(tmpDir);
                }
            }).start();
        } else {
            throw new RuntimeException(dir.toString() + " is expected to be a directory!");
        }
    }

    @SuppressWarnings("ResultOfMethodCallIgnored")
    private static void deleteFilesInFolder(File dir) {
        if (dir.isDirectory()) {
            String[] children = dir.list();
            for (String child : children) {
                File sub = new File(dir, child);
                if (sub.isFile()) {
                    sub.delete();
                } else {
                    deleteFilesInFolder(sub);
                }
            }
        }

        dir.delete();
    }

    native static public void registerInstance(Object self, int id);

    /**
     * Redirect Java callback to JS object function attribute
     *
     * @param finish
     * @param methodName
     * @param args
     * @return
     */
    native static public Object callJS(AtomicBoolean finish, String methodName, Object[] args);


    /**
     * Query all `abstract` methods
     *
     * @return
     */
    private List<MethodRecord> findAllVirtualMethod() {
        List<Method> methods = this.superClass.equals("java/lang/Object") ? new ArrayList<>() : this.queryVirutalMethodsForInterface(this.getSuperClassDot());
        for (String itf : this.interfaces) {
            methods.addAll(this.queryVirutalMethodsForInterface(itf));
        }
        List<MethodRecord> ret = new ArrayList<>();
        for (Method m : methods) {
            ret.add(recordAbstractMethod(m));
        }
        return ret;
    }

    private MethodRecord recordAbstractMethod(Method m) {
        MethodRecord vm = new MethodRecord();
        vm.modifiers = m.getModifiers();
        vm.method = m;
        vm.name = m.getName();
        return vm;
    }

    /**
     * Query all abstract methods for single class
     *
     * @param classPath
     * @return
     */
    private List<Method> queryVirutalMethodsForInterface(String classPath) {
        ArrayList<Method> methods = new ArrayList<>();
        try {
            if (classPath.contains("/")) classPath = classPath.replaceAll("/", ".");
            Class<?> c = Class.forName(classPath);
            Method[] all = c.getMethods();
            for (Method m : all) {
                int modifiers = m.getModifiers();
                if ((modifiers & Modifier.ABSTRACT) != 0
                        && (modifiers & Modifier.STATIC) == 0
                        && (modifiers & Modifier.PUBLIC) != 0) {
                    // System.out.println("impl method " + m.getName());
                    methods.add(m);
                }
            }
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        }
        return methods;
    }

    public static File getDexDir() {
        return new File(Cocos2dxHelper.getCacheDir().getPath() + "/j2j/dex");
    }

    public static File getGenClassDir() {
        return new File(Cocos2dxHelper.getCacheDir().getPath() + "/j2j/genclasses");
    }

    public static File getJarCacheDir() {
        return new File(Cocos2dxHelper.getCacheDir().getPath() + "/j2j/jarCache");
    }

    public String getHashKeyForAnonymousClass() {
        // With the java libraries
        ArrayList<String> list = new ArrayList<>();
        list.add(this.superClass.trim());
        for (String intf : this.interfaces) {
            list.add(intf.trim());
        }
        Collections.sort(list);
        StringBuilder sb = new StringBuilder();
        for (String it : list) {
            sb.append(it);
        }

        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-1");
            digest.reset();
            digest.update(sb.toString().getBytes("utf8"));
            return String.format("%040x", new BigInteger(1, digest.digest()));
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

    private File getJarFile() {
        File jarCacheDir = getJarCacheDir();
        if (!jarCacheDir.exists()) {
            jarCacheDir.mkdirs();
        }
        return new File(jarCacheDir, "GenJar_" + this.getHashKeyForAnonymousClass() + ".jar");
    }


}
