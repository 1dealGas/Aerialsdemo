/* Aerials Haptic Feedback Android Implementation */
#ifdef DM_PLATFORM_ANDROID
#include <dmsdk/sdk.h>


// You know, JNI is a piece of terrible sh*t
// This function is from "adamwestman/extension-vibrate", under the MIT License.
inline jclass GetClass(JNIEnv* env, const char* classname) {
	jclass activity_class = env->FindClass("android/app/NativeActivity");
	jmethodID get_class_loader = env->GetMethodID(activity_class,"getClassLoader", "()Ljava/lang/ClassLoader;");
	jobject cls = env->CallObjectMethod(dmGraphics::GetNativeAndroidActivity(), get_class_loader);
	jclass class_loader = env->FindClass("java/lang/ClassLoader");
	jmethodID find_class = env->GetMethodID(class_loader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
	jstring str_class_name = env->NewStringUTF(classname);
	jclass outcls = (jclass)env->CallObjectMethod(cls, find_class, str_class_name);
	env->DeleteLocalRef(str_class_name);
	return outcls;
}


void AcUtilDoHapticFeedback() {

	// Attach VM & Env
	JNIEnv* Env;
	JavaVM* VM = dmGraphics::GetNativeAndroidJavaVM();
			VM-> AttachCurrentThread(&Env, NULL);

	// Call Class Method
	jclass hClass = GetClass(Env, "com.acutil.haptic.HapticExtension");
	jmethodID DHF = Env -> GetStaticMethodID(hClass, "DoHapticFeedback", "(Landroid/app/Activity;)V");
	Env -> CallStaticVoidMethod( hClass, DHF, dmGraphics::GetNativeAndroidActivity() );

	// Clean Up
	Env -> ExceptionClear();
	VM -> DetachCurrentThread();

}


#endif