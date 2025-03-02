/* Aerials Util Functions */
#include <dmsdk/sdk.h>
#include <dmsdk/dlib/crypt.h>   // This including is omitted in the API Reference


/* Crypt */
static int AuStrToMd5(lua_State* L) {
	size_t InSize;
	const auto Input = (uint8_t*)luaL_checklstring(L, 1, &InSize);

	uint8_t Output[16];
	dmCrypt::HashMd5(Input, (uint32_t)InSize, Output);

	lua_pushlstring(L, (char*)Output, 16);
	return 1;
}

static int AuStrToSha1(lua_State* L) {
	size_t InSize;
	const auto Input = (uint8_t*)luaL_checklstring(L, 1, &InSize);

	uint8_t Output[20];
	dmCrypt::HashSha1(Input, (uint32_t)InSize, Output);

	lua_pushlstring(L, (char*)Output, 20);
	return 1;
}

static int AuStrToSha256(lua_State* L) {
	size_t InSize;
	const auto Input = (uint8_t*)luaL_checklstring(L, 1, &InSize);

	uint8_t Output[32];
	dmCrypt::HashSha256(Input, (uint32_t)InSize, Output);

	lua_pushlstring(L, (char*)Output, 32);
	return 1;
}

static int AuStrToSha512(lua_State* L) {
	size_t InSize;
	const auto Input = (uint8_t*)luaL_checklstring(L, 1, &InSize);

	uint8_t Output[64];
	dmCrypt::HashSha512(Input, (uint32_t)InSize, Output);

	lua_pushlstring(L, (char*)Output, 64);
	return 1;
}

static int AuBase64Encode(lua_State* L) {
	size_t InSize;
	const auto Input = (uint8_t*)luaL_checklstring(L, 1, &InSize);

	size_t OutSize = InSize * 4/3 + 1;
	const auto Output = new uint8_t[OutSize];
	dmCrypt::Base64Encode(Input, (uint32_t)InSize, Output, (uint32_t*)&OutSize);
	lua_pushlstring(L, (char*)Output, OutSize);

	delete[] Output;
	return 1;
}

static int AuBase64Decode(lua_State* L) {
	size_t Size;
	const auto Input = (uint8_t*)luaL_checklstring(L, 1, &Size);

	const auto Output = new uint8_t[Size];
	dmCrypt::Base64Decode(Input, (uint32_t)Size, Output, (uint32_t*)&Size);
	lua_pushlstring(L, (char*)Output, Size);

	delete[] Output;
	return 1;
}


/* Misc */
static int AuDoHapticFeedback(lua_State*) {
	#if defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_IOS)
		extern void AcUtilDoHapticFeedback();
					AcUtilDoHapticFeedback();
	#endif
	return 0;
}

static int AuPushNullptr(lua_State* L) {   // Sth just for pairing with a metatable, and not a Table
	lua_pushlightuserdata(L, nullptr);
	return 1;
}

static int AuNewTable(lua_State* L) {
	lua_createtable( L, (int)lua_tointeger(L, 1), (int)lua_tointeger(L, 2) );
	return 1;
}


/* Defold Extension Related */
#if defined(DM_PLATFORM_ANDROID)   // Android Specific Funcs
	extern JavaVM*  pVm;			extern jclass	  hapticClass;
	extern jobject  pActivity;		extern jmethodID  hapticMethod;

	extern int AuInputActivate(lua_State* L);
	extern int AuInputConsume(lua_State* L);
	extern int AuInputUnpack(lua_State* L);
#endif

static constexpr luaL_reg AuAPIs[] = {
	{"StrToMd5", AuStrToMd5},
	{"StrToSha1", AuStrToSha1},
	{"StrToSha256", AuStrToSha256},
	{"StrToSha512", AuStrToSha512},
	{"Base64Encode", AuBase64Encode},
	{"Base64Decode", AuBase64Decode},
	{"DoHapticFeedback", AuDoHapticFeedback},
	{"PushNullptr", AuPushNullptr},
	{"NewTable", AuNewTable},
#if defined(DM_PLATFORM_ANDROID)
	{"ActivateInput", AuInputActivate},
	{"ConsumeInput", AuInputConsume},
	{"UnpackInput", AuInputUnpack},
#endif
	{nullptr, nullptr}
};

static dmExtension::Result AuInit(dmExtension::Params* p) {
	#ifdef DM_PLATFORM_ANDROID
		JNIEnv* pEnv;
		pVm = dmGraphics::GetNativeAndroidJavaVM();
		pVm-> AttachCurrentThread(&pEnv, NULL);
		hapticClass = (jclass)pEnv->NewGlobalRef(
			pEnv->CallObjectMethod(
				/* Object */ pEnv->CallObjectMethod(
					pActivity = dmGraphics::GetNativeAndroidActivity(),
					pEnv->GetMethodID( pEnv->FindClass("android/app/NativeActivity"), "getClassLoader",
													   "()Ljava/lang/ClassLoader;" )
				),
				/* Method */ pEnv->GetMethodID( pEnv->FindClass("java/lang/ClassLoader"), "loadClass",
											   "(Ljava/lang/String;)Ljava/lang/Class;" ),
				/* Arg */	 pEnv->NewStringUTF("ideal.acutil.Haptic")
			)
		);
		hapticMethod = pEnv->GetStaticMethodID(hapticClass, "DoHapticFeedback", "(Landroid/app/Activity;)V");
		pEnv -> ExceptionClear();
		pVm  -> DetachCurrentThread();
	#endif
	return luaL_register(p->m_L, "AcUtil", AuAPIs), lua_pop(p->m_L, 1), dmExtension::RESULT_OK;
}

static dmExtension::Result AuFinal(dmExtension::Params*) {
	#ifdef DM_PLATFORM_ANDROID
		JNIEnv* pEnv;
		pVm  -> AttachCurrentThread(&pEnv, NULL);
		pEnv -> DeleteGlobalRef(hapticClass);
		pVm  -> DetachCurrentThread();
	#endif
	return dmExtension::RESULT_OK;
}

static dmExtension::Result AuAPPOK(dmExtension::AppParams*) {
	return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(AcUtil, "AcUtil", AuAPPOK, AuAPPOK, AuInit, nullptr, nullptr, AuFinal)