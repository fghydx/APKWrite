//
// Created by GLPC on 2020/5/18.
//

#include <android/native_activity.h>
#include "PubTest.h"

extern ANativeActivity* nativeActivity;

static inline jclass GetClass(JNIEnv* envPtr,char* className)
{
    static  jobject   classLoaderObj = NULL;
    static  jmethodID loadClassId    = NULL;

    if (classLoaderObj == NULL)
    {
        jclass    activityCls      = envPtr->FindClass( "android/app/NativeActivity");
        jclass    loaderCls        = envPtr->FindClass("java/lang/ClassLoader");
        jmethodID getClassLoaderId = envPtr->GetMethodID
                (
                        activityCls,
                        "getClassLoader",
                        "()Ljava/lang/ClassLoader;"
                );

        classLoaderObj  = envPtr->CallObjectMethod(nativeActivity->clazz, getClassLoaderId);
        loadClassId   = envPtr->GetMethodID
                (
                        loaderCls,
                        "loadClass",
                        "(Ljava/lang/String;)Ljava/lang/Class;"
                );
    }

    jstring classNameStr = envPtr->NewStringUTF(className);
    jclass  cls          = (jclass) envPtr->CallObjectMethod(classLoaderObj, loadClassId, classNameStr);

    envPtr->DeleteLocalRef(classNameStr);

    return cls;
}

static jvalue CallObjectMethod(JNIEnv*   envPtr,jobject object, char* methodName, char* paramCode, ...)
{
    jclass    cls       = envPtr->GetObjectClass(object);
    jmethodID methodId  = envPtr->GetMethodID(cls, methodName, paramCode);

    char* p = paramCode;

    // skip '()' to find out the return type
    while (*p != ')')
    {
        p++;
    }
    // skip ')'
    p++;

    va_list  args;
    va_start(args, paramCode);
    jvalue   value;

    switch (*p)
    {
        case 'V':
            envPtr->CallVoidMethodV(object, methodId, args);
            break;

        case '[':
        case 'L':
            value.l = envPtr->CallObjectMethodV (object, methodId, args);
            break;

        case 'Z':
            value.z = envPtr->CallBooleanMethodV(object, methodId, args);
            break;

        case 'B':
            value.b = envPtr->CallByteMethodV   (object, methodId, args);
            break;

        case 'C':
            value.c = envPtr->CallCharMethodV   (object, methodId, args);
            break;

        case 'S':
            value.s = envPtr->CallShortMethodV  (object, methodId, args);
            break;

        case 'I':
            value.i = envPtr->CallIntMethodV    (object, methodId, args);
            break;

        case 'J':
            value.j = envPtr->CallLongMethodV   (object, methodId, args);
            break;

        case 'F':
            value.f = envPtr->CallFloatMethodV  (object, methodId, args);
            break;

        case 'D':
            value.d = envPtr->CallDoubleMethodV (object, methodId, args);
            break;
    }

    va_end(args);

    return value;
}

static inline jvalue CallClassMethodV(JNIEnv*   envPtr,jclass cls, char* methodName, char* paramCode, va_list args)
{
    jmethodID methodId  = envPtr->GetStaticMethodID(cls, methodName, paramCode);
    char* p = paramCode;

    // skip '()' to find out the return type
    while (*p != ')')
    {
        p++;
    }
    // skip ')'
    p++;

    jvalue value;
    switch (*p)
    {
        case 'V':
            envPtr->CallStaticVoidMethodV(cls, methodId, args);
            break;

        case '[':
        case 'L':
            value.l = envPtr->CallStaticObjectMethodV (cls, methodId, args);
            break;

        case 'Z':
            value.z = envPtr->CallStaticBooleanMethodV(cls, methodId, args);
            break;

        case 'B':
            value.b = envPtr->CallStaticByteMethodV   (cls, methodId, args);
            break;

        case 'C':
            value.c = envPtr->CallStaticCharMethodV   (cls, methodId, args);
            break;

        case 'S':
            value.s = envPtr->CallStaticShortMethodV  (cls, methodId, args);
            break;

        case 'I':
            value.i = envPtr->CallStaticIntMethodV    (cls, methodId, args);
            break;

        case 'J':
            value.j = envPtr->CallStaticLongMethodV   (cls, methodId, args);
            break;

        case 'F':
            value.f = envPtr->CallStaticFloatMethodV  (cls, methodId, args);
            break;

        case 'D':
            value.d = envPtr->CallStaticDoubleMethodV (cls, methodId, args);
            break;
    }

    return value;
}

static inline jvalue CallClassMethod(JNIEnv*   envPtr,jclass cls, char* methodName, char* paramCode, ...)
{
    va_list args;
    va_start(args, paramCode);
    jvalue value = CallClassMethodV(envPtr,cls, methodName, paramCode, args);
    va_end(args);

    return value;
}

static inline jvalue CallStaticMethod(JNIEnv*   envPtr,char* className, char* methodName, char* paramCode, ...)
{
    va_list args;
    va_start(args, paramCode);
    jvalue value = CallClassMethodV(envPtr,GetClass(envPtr,className), methodName, paramCode, args);
    va_end(args);

    return value;
}

static jvalue GetStaticField(JNIEnv*   envPtr,char* className, char* fieldName, char* typeCode)
{
    jclass   cls     = GetClass(envPtr,className);
    jfieldID fieldID = envPtr->GetStaticFieldID(cls, fieldName, typeCode);
    jvalue   value;

    while (*typeCode != ')')
    {
        typeCode++;
    }

    switch (*typeCode)
    {
        case '[':
        case 'L':
            value.l = envPtr->GetStaticObjectField (cls, fieldID);
            break;

        case 'Z':
            value.z = envPtr->GetStaticBooleanField(cls, fieldID);
            break;

        case 'B':
            value.b = envPtr->GetStaticByteField   (cls, fieldID);
            break;

        case 'C':
            value.c = envPtr->GetStaticCharField   (cls, fieldID);
            break;

        case 'S':
            value.s = envPtr->GetStaticShortField  (cls, fieldID);
            break;

        case 'I':
            value.i = envPtr->GetStaticIntField    (cls, fieldID);
            break;

        case 'J':
            value.j = envPtr->GetStaticLongField   (cls, fieldID);
            break;

        case 'F':
            value.f = envPtr->GetStaticFloatField  (cls, fieldID);
            break;

        case 'D':
            value.d = envPtr->GetStaticDoubleField (cls, fieldID);
            break;
    }

    return value;
}


static inline jvalue GetField(JNIEnv*   envPtr,jobject object, char* fieldName, char* typeCode)
{
    jfieldID fieldID = envPtr->GetFieldID(envPtr->GetObjectClass(object), fieldName, typeCode);
    jvalue   value   = {};

    while (*typeCode != ')')
    {
        typeCode++;
    }

    switch (*typeCode)
    {
        case '[':
        case 'L':
            value.l = envPtr->GetObjectField (object, fieldID);
            break;

        case 'Z':
            value.z = envPtr->GetBooleanField(object, fieldID);
            break;

        case 'B':
            value.b = envPtr->GetByteField   (object, fieldID);
            break;

        case 'C':
            value.c = envPtr->GetCharField   (object, fieldID);
            break;

        case 'S':
            value.s = envPtr->GetShortField  (object, fieldID);
            break;

        case 'I':
            value.i = envPtr->GetIntField    (object, fieldID);
            break;

        case 'J':
            value.j = envPtr->GetLongField   (object, fieldID);
            break;

        case 'F':
            value.f = envPtr->GetFloatField  (object, fieldID);
            break;

        case 'D':
            value.d = envPtr->GetDoubleField (object, fieldID);
            break;;
    }

    return value;
}

static jsize GetArrayLength(JNIEnv*   envPtr,jarray array)
{
    return envPtr->GetArrayLength(array);
}

static jvalue GetArrayAt(JNIEnv*   envPtr,jarray array, jint index, char typeChar)
{
    jvalue value = {};

    switch (typeChar)
    {
        case '[':
        case 'L':
            value.l = envPtr->GetObjectArrayElement  (static_cast<jobjectArray>(array), index);
            break;

        case 'Z':
            value.z = envPtr->GetBooleanArrayElements(static_cast<jbooleanArray>(array), JNI_FALSE)[index];
            break;

        case 'B':
            value.b = envPtr->GetByteArrayElements   (static_cast<jbyteArray>(array), JNI_FALSE)[index];
            break;

        case 'C':
            value.c = envPtr->GetCharArrayElements   (static_cast<jcharArray>(array), JNI_FALSE)[index];
            break;

        case 'S':
            value.s = envPtr->GetShortArrayElements  (static_cast<jshortArray>(array), JNI_FALSE)[index];
            break;

        case 'I':
            value.i = envPtr->GetIntArrayElements    (static_cast<jintArray>(array), JNI_FALSE)[index];
            break;

        case 'J':
            value.j = envPtr->GetLongArrayElements   (static_cast<jlongArray>(array), JNI_FALSE)[index];
            break;

        case 'F':
            value.f = envPtr->GetFloatArrayElements  (static_cast<jfloatArray>(array), JNI_FALSE)[index];
            break;

        case 'D':
            value.d = envPtr->GetDoubleArrayElements (static_cast<jdoubleArray>(array), JNI_FALSE)[index];
            break;
    }

    return value;
}

static jvalue GetNativeActivityField(JNIEnv*   envPtr,char* fieldName, char* typeCode)
{
    return GetField(envPtr,nativeActivity->clazz, fieldName, typeCode);
}

static jstring CharTojstring(JNIEnv* env, const char* pat,int len) {
    //定义java String类 strClass
    jclass strClass = (env)->FindClass("java/lang/String");
    //获取String(byte[],String)的构造器,用于将本地byte[]数组转换为一个新String
    jmethodID ctorID = (env)->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    //建立byte数组
    jbyteArray bytes = (env)->NewByteArray(len);
    //将char* 转换为byte数组
    (env)->SetByteArrayRegion(bytes, 0, len, (jbyte*) pat);
    // 设置String, 保存语言类型,用于byte数组转换至String时的参数
    jstring encoding = (env)->NewStringUTF("GB2312");
    //将byte数组转换为java String,并输出
    return (jstring) (env)->NewObject(strClass, ctorID, bytes, encoding);
}

