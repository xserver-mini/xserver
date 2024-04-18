#include <jni.h>
#include <string>

#include "core/platform/jnihelper.h"
#include "ctrl.h"

#define DEFINEFUN(__FUNC__) Java_com_linyou_xserver_XServer_##__FUNC__

extern "C" JNIEXPORT jstring JNICALL
DEFINEFUN(StringFromJNI)(JNIEnv* env, jobject /* this */)
{
    std::string hello = "Hello XServer";
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT void JNICALL
DEFINEFUN(MainJNI)(JNIEnv* env, jobject /* this */)
{
    Ctrl::GetInstance().start();
}

extern "C" JNIEXPORT void JNICALL
DEFINEFUN(NativeSetContext)(JNIEnv* env, jobject thiz, jobject context)
{
    JniHelper::setClassLoaderFrom(context);
}

extern "C" JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved)
{
    JniHelper::setJavaVM(vm);
    return JNI_VERSION_1_4;
}
