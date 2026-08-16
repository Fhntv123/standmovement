#include <jni.h>
#include "globals.hpp"

globals_t g{};

jint JNIEXPORT JNI_OnLoad(JavaVM* vm, void* r) {
	JNIEnv* env{};
	vm->GetEnv((void**)&env, oxorany(JNI_VERSION_1_6));
	g.jenv = env;
	g.entry();
	return 0x1337;
}