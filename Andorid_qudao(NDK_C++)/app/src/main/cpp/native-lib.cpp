#include <jni.h>
#include <string>
#include <zconf.h>
#include <android/log.h>
#include "PubTest.cpp"

int GetInt(Byte* byte){
        int* tmp = (int*)byte;
        return *tmp;
}
long GetLong(Byte* byte){
        long* tmp = (long*)byte;
        return *tmp;
}

Byte* GetSigendBlock(JNIEnv* env,Byte* byte,int* file_size)
{
        int i = *file_size - 22;
        if (!(*(byte + i) == 0x50 && *(byte + i + 1) ==0x4b && *(byte + i + 2) == 0x05 && *(byte + i + 3) == 0x06)) {
                return NULL;
        }

        int start_central_directory_value_pos = *file_size - 6;
        int start_central_directory = *((int*)(byte + start_central_directory_value_pos));
        char* tmp = (char*)(byte + start_central_directory - 16);

//        jstring s =env->NewString((jchar*)tmp,16);
        jstring ss = CharTojstring(env,tmp,16);
        const char* tmp1 = env->GetStringUTFChars(ss, JNI_FALSE);
        char* c = "APK Sig Block 42";
        if (strcmp(tmp1,c) == 0){
            int sgin_block_value_pos = start_central_directory - 16 - 8;
            long sign_block_size = GetLong(byte+sgin_block_value_pos);
            int sign_block_start_pos = start_central_directory - int(sign_block_size) - 8;
            long apkSigBlockSizeInHeader = GetLong(byte + sign_block_start_pos);
            if (sign_block_size != apkSigBlockSizeInHeader) {
                    return NULL;
            }

            int declan = sgin_block_value_pos+8-sign_block_start_pos - 16;
            Byte* result = (Byte*)malloc(declan);
            *file_size = declan;
            memcpy(result,byte + sign_block_start_pos + 8,declan);
            return result;
        }
        return NULL;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myapplication_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */context_object,
        jstring ApkFile,jint ID) {
        char* t;
        t = (char*)env->GetStringUTFChars(ApkFile,0);
        FILE* file = fopen(t, "r");
        fseek( file,0,SEEK_END);
        //获取文件的大小
        int file_size=ftell( file );
        Byte* byte = (Byte*)alloca(file_size);
        fseek( file,0,SEEK_SET);
        fread(byte,file_size,1,file);
        fclose(file);
        Byte* Blocked = GetSigendBlock(env,byte,&file_size);

        if (Blocked==NULL)
        {
                return env->NewStringUTF("不是2代签名");
        }

        int tmplen = 0;
        while (true){
                long pairlen = GetLong(Blocked);
                int id = GetInt(Blocked+8);
                char* Data = (char*)(Blocked + 12);
                Blocked = Blocked + pairlen + 8;
                tmplen = tmplen + pairlen + 8;
                if (id == ID){
                        jstring ss = CharTojstring(env,Data,pairlen-4);
                        return ss;
                }
                if (tmplen == file_size){
                        return env->NewStringUTF("没找到！");
                }
        }
}
