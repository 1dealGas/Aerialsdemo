#include <dmsdk/sdk.h>
#ifdef DM_PLATFORM_ANDROID   /* Aerials Haptic Feedback Android Implementation */
jobject  pActivity;		jmethodID  hapticMethod;
JavaVM*  pVm;			jclass	   hapticClass;

void AcUtilDoHapticFeedback() {
	JNIEnv* pEnv;
	pVm  -> AttachCurrentThread(&pEnv, NULL);
	pEnv -> CallStaticVoidMethod(hapticClass, hapticMethod, pActivity);   // This method doesn't throw exceptions.
	pVm  -> DetachCurrentThread();
}
#endif