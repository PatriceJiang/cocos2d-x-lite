package org.cocos2dx.lib;


import android.util.Log;

import com.android.tools.r8.D8;
import com.android.tools.r8.D8Command;

import org.objectweb.asm.ClassWriter;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;
import org.objectweb.asm.tree.ClassNode;
import org.objectweb.asm.tree.FieldInsnNode;
import org.objectweb.asm.tree.FieldNode;
import org.objectweb.asm.tree.InsnList;
import org.objectweb.asm.tree.InsnNode;
import org.objectweb.asm.tree.MethodInsnNode;
import org.objectweb.asm.tree.MethodNode;
import org.objectweb.asm.tree.TypeInsnNode;
import org.objectweb.asm.tree.VarInsnNode;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.security.SecureClassLoader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.jar.JarEntry;
import java.util.jar.JarOutputStream;

import dalvik.system.DexClassLoader;
import dalvik.system.PathClassLoader;

public class ByteCodeGenerator {


    public static final String NATIVE_ID = "__native_id__";
    public static final String PROXY_CLASS_NAME = "org/cocos2dx/test/Main";
    public static final String PROXY_METHOD_NAME = "printArguments";
    public static final String LOGCAT_TAG = "Bytecode Generator";

    static int object_id = 10000;

    class VirtualMethod {
        public int modifiers;
        public String signature;
        public String desc;
        public Type[] args;
        public Type ret;
        public String name;
    }

    private String superClass = "java/lang/Object";
    private ArrayList<String> interfaces = new ArrayList<>();
    private String name;
    private ClassNode cn;

    static MyClassLoader classLoader = null;

    public ByteCodeGenerator() {
        cn = new ClassNode();
    }

    public void setName(String name) {
        if (!name.isEmpty()) {
            this.name = name.replaceAll("\\.", "/");
        }
    }

    public String getNameDot() {
        return name.replaceAll("/", ".");
    }

    public void setSuperClass(String superClass) {
        this.superClass = superClass;
    }

    public String getSuperClassDot() {
        return superClass.replaceAll("/", ".");
    }

    public void addInterface(String itf) {
        this.interfaces.add(itf.replaceAll("\\.", "/"));
    }

    public Class<?> build() {


        cn.version = Opcodes.V1_6;
        cn.access = Opcodes.ACC_PUBLIC | Opcodes.ACC_SUPER;
        cn.name = this.name;
        cn.superName = this.superClass;
        if (!this.interfaces.isEmpty()) {
            cn.interfaces.addAll(this.interfaces);
        }

        setupDefaultConstructor();
        addIdentifyField();
        List<VirtualMethod> methods = findAllVirtualMethod();
        for (VirtualMethod m : methods) {
            setupAbstructFunction(m);
        }

        ClassWriter cw = new ClassWriter(ClassWriter.COMPUTE_FRAMES);
        cn.accept(cw);
//        Class<?> c = classLoader.defineClass(this.name.replaceAll("/", "."), cw.toByteArray());

        return loadJavaClassBytes(this.name, cw.toByteArray());
    }


    public static int getId() {
        return object_id++;
    }

    private void setupDefaultConstructor() {
        MethodNode constructor = new MethodNode(Opcodes.ACC_PUBLIC, "<init>", "()V", null, null);
        InsnList inst = constructor.instructions;
        inst.add(new VarInsnNode(Opcodes.ALOAD, 0));
        inst.add(new MethodInsnNode(Opcodes.INVOKESPECIAL, this.superClass, "<init>", "()V"));
        // update id
        inst.add(new MethodInsnNode(Opcodes.INVOKESTATIC, "org/cocos2dx/lib/ByteCodeGenerator", "getId", "()I"));
        inst.add(new VarInsnNode(Opcodes.ISTORE, 1));
        inst.add(new VarInsnNode(Opcodes.ALOAD, 0));
        inst.add(new VarInsnNode(Opcodes.ILOAD, 1));
        inst.add(new FieldInsnNode(Opcodes.PUTFIELD, this.name, NATIVE_ID, "I"));


        inst.add(new VarInsnNode(Opcodes.ALOAD, 0));
        inst.add(new VarInsnNode(Opcodes.ILOAD, 1));
        inst.add(new MethodInsnNode(Opcodes.INVOKESTATIC, "org/cocos2dx/lib/ByteCodeGenerator", "registerInstance", "(Ljava/lang/Object;I)V"));

        constructor.maxLocals = 2;
        constructor.maxStack = 2;

        inst.add(new InsnNode(Opcodes.RETURN));


        cn.methods.add(constructor);
    }

    private void addIdentifyField() {
        FieldNode fn = new FieldNode(Opcodes.ACC_PUBLIC, NATIVE_ID, "I", null, null);
        cn.fields.add(fn);
    }

    private void setupAbstructFunction(VirtualMethod method) {
        int modifier = method.modifiers & (~Opcodes.ACC_ABSTRACT);
        MethodNode mn = new MethodNode(modifier, method.name, method.desc, null, null);

        int argumentCnt = method.args.length;


        int slotSize = 0;
        for (int i = 0; i < argumentCnt; i++) {
            slotSize += method.args[i].getSize();
        }


        mn.maxLocals = slotSize + 2;
        mn.maxStack = 6;

        InsnList inst = mn.instructions;
        inst.add(new TypeInsnNode(Opcodes.NEW, "java/util/ArrayList"));
        inst.add(new InsnNode(Opcodes.DUP));
        inst.add(new MethodInsnNode(Opcodes.INVOKESPECIAL, "java/util/ArrayList", "<init>", "()V"));

        // setup objectID key
        inst.add(new InsnNode(Opcodes.DUP));
        inst.add(new TypeInsnNode(Opcodes.NEW, "java/lang/Integer"));
        inst.add(new InsnNode(Opcodes.DUP));
        inst.add(new VarInsnNode(Opcodes.ALOAD, 0)); // this
        inst.add(new FieldInsnNode(Opcodes.GETFIELD, this.name, NATIVE_ID, "I"));
        inst.add(new MethodInsnNode(Opcodes.INVOKESPECIAL, "java/lang/Integer", "<init>", "(I)V"));
        inst.add(new MethodInsnNode(Opcodes.INVOKEINTERFACE, "java/util/List", "add", "(Ljava/lang/Object;)Z"));
        inst.add(new InsnNode(Opcodes.POP));

        // push this
        inst.add(new InsnNode(Opcodes.DUP));
        inst.add(new VarInsnNode(Opcodes.ALOAD, 0));
        inst.add(new MethodInsnNode(Opcodes.INVOKEINTERFACE, "java/util/List", "add", "(Ljava/lang/Object;)Z"));
        inst.add(new InsnNode(Opcodes.POP));


        // push other arguments
        int slotOffset = 1;  // first is this object
        int typeSize = 0;
        for (int i = 0; i < argumentCnt; i++) {
            Type t = method.args[i];
            typeSize = t.getSize();
            inst.add(new InsnNode(Opcodes.DUP));
            switch (t.getSort()) {
                case Type.BOOLEAN:
                    inst.add(new TypeInsnNode(Opcodes.NEW, "java/lang/Boolean"));
                    inst.add(new InsnNode(Opcodes.DUP));
                    inst.add(new VarInsnNode(Opcodes.ILOAD, slotOffset));
                    inst.add(new MethodInsnNode(Opcodes.INVOKESPECIAL, "java/lang/Boolean", "<init>", "(Z)V"));
                    break;
                case Type.CHAR:
                    inst.add(new TypeInsnNode(Opcodes.NEW, "java/lang/Character"));
                    inst.add(new InsnNode(Opcodes.DUP));
                    inst.add(new VarInsnNode(Opcodes.ILOAD, slotOffset));
                    inst.add(new MethodInsnNode(Opcodes.INVOKESPECIAL, "java/lang/Character", "<init>", "(C)V"));
                    break;
                case Type.BYTE:
                    inst.add(new TypeInsnNode(Opcodes.NEW, "java/lang/Byte"));
                    inst.add(new InsnNode(Opcodes.DUP));
                    inst.add(new VarInsnNode(Opcodes.ILOAD, slotOffset));
                    inst.add(new MethodInsnNode(Opcodes.INVOKESPECIAL, "java/lang/Byte", "<init>", "(B)V"));
                    break;
                case Type.SHORT:
                    inst.add(new TypeInsnNode(Opcodes.NEW, "java/lang/Short"));
                    inst.add(new InsnNode(Opcodes.DUP));
                    inst.add(new VarInsnNode(Opcodes.ILOAD, slotOffset));
                    inst.add(new MethodInsnNode(Opcodes.INVOKESPECIAL, "java/lang/Short", "<init>", "(S)V"));
                    break;
                case Type.INT:
                    inst.add(new TypeInsnNode(Opcodes.NEW, "java/lang/Integer"));
                    inst.add(new InsnNode(Opcodes.DUP));
                    inst.add(new VarInsnNode(Opcodes.ILOAD, slotOffset));
                    inst.add(new MethodInsnNode(Opcodes.INVOKESPECIAL, "java/lang/Integer", "<init>", "(I)V"));
                    break;
                case Type.LONG:
                    inst.add(new TypeInsnNode(Opcodes.NEW, "java/lang/Long"));
                    inst.add(new InsnNode(Opcodes.DUP));
                    inst.add(new VarInsnNode(Opcodes.LLOAD, slotOffset));
                    inst.add(new MethodInsnNode(Opcodes.INVOKESPECIAL, "java/lang/Long", "<init>", "(J)V"));
                    break;
                case Type.FLOAT:
                    inst.add(new TypeInsnNode(Opcodes.NEW, "java/lang/Float"));
                    inst.add(new InsnNode(Opcodes.DUP));
                    inst.add(new VarInsnNode(Opcodes.FLOAD, slotOffset));
                    inst.add(new MethodInsnNode(Opcodes.INVOKESPECIAL, "java/lang/Float", "<init>", "(F)V"));
                    break;
                case Type.DOUBLE:
                    inst.add(new TypeInsnNode(Opcodes.NEW, "java/lang/Double"));
                    inst.add(new InsnNode(Opcodes.DUP));
                    inst.add(new VarInsnNode(Opcodes.DLOAD, slotOffset));
                    inst.add(new MethodInsnNode(Opcodes.INVOKESPECIAL, "java/lang/Double", "<init>", "(D)V"));
                    break;
                case Type.OBJECT:
                    inst.add(new VarInsnNode(Opcodes.ALOAD, slotOffset));
                    break;
                default:
                    break;

            }
            inst.add(new MethodInsnNode(Opcodes.INVOKEINTERFACE, "java/util/List", "add", "(Ljava/lang/Object;)Z"));

            inst.add(new InsnNode(Opcodes.POP)); // pop boolean from List.add
            slotOffset += typeSize;
        }
        inst.add(new InsnNode(Opcodes.DUP));
        inst.add(new MethodInsnNode(Opcodes.INVOKEINTERFACE, "java/util/List", "toArray", "()[Ljava/lang/Object;"));


        String proxyMethodName = PROXY_METHOD_NAME + method.ret.toString();
        inst.add(new MethodInsnNode(Opcodes.INVOKESTATIC, PROXY_CLASS_NAME, proxyMethodName, "([Ljava/lang/Object;)" + method.ret.toString()));

        inst.add(new InsnNode(Opcodes.RETURN));
        cn.methods.add(mn);
    }

    private List<VirtualMethod> findAllVirtualMethod() {
        List<Method> methods = this.superClass.equals("java/lang/Object") ? new ArrayList<>() : this.getVirutalMethods(this.getSuperClassDot());
        for (String itf : this.interfaces) {
            methods.addAll(this.getVirutalMethods(itf));
        }
        List<VirtualMethod> ret = new ArrayList<>();
        for (Method m : methods) {
            ret.add(convertMethodToVirtualMethod(m));
        }
        return ret;
    }

    private VirtualMethod convertMethodToVirtualMethod(Method m) {
        VirtualMethod vm = new VirtualMethod();
        vm.modifiers = m.getModifiers();
        vm.signature = Type.getType(m).toString();
        vm.desc = Type.getMethodDescriptor(m).toString();
        vm.args = Type.getArgumentTypes(m);
        vm.ret = Type.getReturnType(m);
        vm.name = m.getName();

        return vm;
    }

    private List<Method> getVirutalMethods(String classPath) {
        ArrayList<Method> methods = new ArrayList<>();
        try {
            if (classPath.contains("/")) classPath = classPath.replaceAll("/", ".");
            Class c = Class.forName(classPath);
            Method[] all = c.getMethods();
            for (Method m : all) {
                int modifiers = m.getModifiers();
                if ((modifiers & Modifier.ABSTRACT) != 0
                        && (modifiers & Modifier.STATIC) == 0
                        && (modifiers & Modifier.PUBLIC) != 0) {
                    System.out.println("impl method " + m.getName());
                    methods.add(m);
                }
            }
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        }
        return methods;
    }

    final private static String join(String sep, String []parts) {
        if(parts.length == 0) return "";
        StringBuffer sb  = new StringBuffer();
        sb.append(parts[0]);
        for(int i =1 ;i < parts.length; i++) {
            sb.append(sep).append(parts[i]);
        }
        return sb.toString();
    }

    public Class<?> loadJavaClassBytes(String className, byte[] data) {

        String cacheDir = Cocos2dxHelper.getCacheDir().getPath();
        String classParts [] = className.split("[./]");
        if(!cacheDir.endsWith("/")) {
            cacheDir += "/";
        }

        File saveDexDir = new File(cacheDir + "/dex");
        if(!saveDexDir.exists()) {
            if(!saveDexDir.mkdirs()){
                Log.e(LOGCAT_TAG, "failed to create dir " + saveDexDir.getPath());
                return null;
            }

        }

        // save klass path
        File generatedClassesDir = new File(cacheDir + "/genclasses");
        if(!generatedClassesDir.exists()){
            if(!generatedClassesDir.mkdirs()){
                Log.e(LOGCAT_TAG, "failed to create dir " + generatedClassesDir.getPath());
                return null;
            }
        }

        File generatedClassFile = null;
        try {
            generatedClassFile = File.createTempFile("GClassFile", ".class", generatedClassesDir);
            generatedClassFile.deleteOnExit();
            FileOutputStream fos = new FileOutputStream(generatedClassFile);
            fos.write(data);
            fos.close();
        }catch (IOException e){
            e.printStackTrace();
            return null;
        }

        final String inputGeneratedClassPath = generatedClassFile.getPath();
        final String dexOutputPath = saveDexDir.getPath();

        Thread taskGenerateDex = new Thread(new Runnable() {
            @Override
            public void run() {
                D8.main(new String[]{inputGeneratedClassPath , "--output", dexOutputPath});
//                ExceptionUtils.withMainProgramHandler(() -> {
//                    D8Command command = (D8Command)D8Command.parse(new String[]{klassPath}, CommandLineOrigin.INSTANCE).build();
//                    D8.run(command);
//                });
            }
        });

        taskGenerateDex.start();
        try {
            taskGenerateDex.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
            return null;
        }

        File jarCacheDir = new File(cacheDir + "dexCache");
        File dexFile = new File(dexOutputPath + "/classes.dex");
        try{
            if(!jarCacheDir.exists()) {
                jarCacheDir.mkdirs();
            }

            return this.loadDexClass(dexFile, jarCacheDir);
        }catch (Exception e) {
            e.printStackTrace();
        }finally {
//            dstKlsFile.delete();
//            dexPath.delete();
        }
        return null;
    }

    private Class<?> loadDexClass(File dexFile, File jarCachePath) throws IOException, ClassNotFoundException, NoSuchMethodException, IllegalAccessException, InvocationTargetException, InstantiationException {

        byte[] dex = new byte[(int) dexFile.length()];

        FileInputStream fis = new FileInputStream(dexFile);
        fis.read(dex);

        File result = File.createTempFile("GenJar", ".jar", jarCachePath);
        result.deleteOnExit();
        JarOutputStream jarOut = new JarOutputStream(new FileOutputStream(result));
        //jarOut.putNextEntry(new JarEntry(this.name+".dex"));
        jarOut.putNextEntry(new JarEntry("classes.dex"));
        jarOut.write(dex);
        jarOut.closeEntry();
        jarOut.close();
        ClassLoader loader = (ClassLoader) Class.forName("dalvik.system.DexClassLoader")
                    .getConstructor(String.class, String.class, String.class, ClassLoader.class)
                    .newInstance(result.getPath(), jarCachePath.getAbsolutePath(), null, getClass().getClassLoader());

        Class<?> ret = null;
        if(loader != null) {
             ret = loader.loadClass(this.getNameDot());
        }
        return ret;
    }


    public static class MyClassLoader extends SecureClassLoader {

        public Class defineClass(String name, byte[] b) {
            return defineClass(name, b, 0, b.length);
        }

    }


    static {
        classLoader = new MyClassLoader();
    }


    native static public void registerInstance(Object self, int id);
    native static public Object callJS(AtomicBoolean finish, String methodName, Object []args);
}
