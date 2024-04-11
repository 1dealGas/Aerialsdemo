/* Aerials Util Functions */
#include <dmsdk/sdk.h>
#include <dmsdk/dlib/crypt.h>   // This including is omitted in the API Reference


/* APIs */
#if defined(DM_PLATFORM_IOS) || defined(DM_PLATFORM_ANDROID)
	extern void AcUtilDoHapticFeedback();
#endif

namespace AcUtil {
	inline int NewTable(lua_State* L) {   // LUA_MINSTACK should be larger than 20
		lua_createtable( L, (int)luaL_checknumber(L, 1), (int)luaL_checknumber(L, 2) );
		return 1;
	}
	inline int PushNullptr(lua_State* L) {   // Sth just for pairing with a metatable, and not a Table
		lua_pushlightuserdata(L, nullptr);
		return 1;
	}
	inline int DoHapticFeedback(lua_State* L) {
		#if defined(DM_PLATFORM_IOS) || defined(DM_PLATFORM_ANDROID)
		AcUtilDoHapticFeedback();
		#endif
		return 0;
	}

	inline int StrToSha1(lua_State* L) {
		size_t InSize;
		const auto Input	= (uint8_t*)luaL_checklstring(L, 1, &InSize);
			  auto Output	= new uint8_t[20];
		dmCrypt::HashSha1(Input, (uint32_t)InSize, Output);
		lua_pushlstring(L, (char*)Output, 20);
		delete[] Output;
		return 1;
	}

	inline int StrToSha256(lua_State* L) {
		size_t InSize;
		const auto Input	= (uint8_t*)luaL_checklstring(L, 1, &InSize);
			  auto Output	= new uint8_t[32];
		dmCrypt::HashSha256(Input, (uint32_t)InSize, Output);
		lua_pushlstring(L, (char*)Output, 32);
		delete[] Output;
		return 1;
	}

	inline int StrToSha512(lua_State* L) {
		size_t InSize;
		const auto Input = (uint8_t*)luaL_checklstring(L, 1, &InSize);
			  auto Output = new uint8_t[64];
		dmCrypt::HashSha512(Input, (uint32_t)InSize, Output);
		lua_pushlstring(L, (char*)Output, 64);
		delete[] Output;
		return 1;
	}

	inline int StrToMd5(lua_State* L) {
		size_t InSize;
		const auto Input = (uint8_t*)luaL_checklstring(L, 1, &InSize);
			  auto Output = new uint8_t[16];
		dmCrypt::HashMd5(Input, (uint32_t)InSize, Output);
		lua_pushlstring(L, (char*)Output, 16);
		delete[] Output;
		return 1;
	}
}


/* Binding Stuff */
namespace AuBinding {
	constexpr luaL_reg LuaAPIs[] = {
		{"NewTable", AcUtil::NewTable}, {"PushNullptr", AcUtil::PushNullptr},
		{"StrToSha1", AcUtil::StrToSha1}, {"StrToSha256", AcUtil::StrToSha256}, {"StrToSha512", AcUtil::StrToSha512},
		{"StrToMd5", AcUtil::StrToMd5}, {"DoHapticFeedback", AcUtil::DoHapticFeedback}, {nullptr, nullptr}
	};

	// Lifecycle Calls
	inline dmExtension::Result Init(dmExtension::Params* p) {   // Init
		luaL_register( p->m_L, "AcUtil", AuBinding::LuaAPIs );
		lua_pop( p->m_L, 1 );
		return dmExtension::RESULT_OK;
	}
	inline dmExtension::Result OK(dmExtension::Params* p) {   // Final
		return dmExtension::RESULT_OK;
	}
	inline dmExtension::Result APPOK(dmExtension::AppParams* params) {   // AppInit, AppFinal
		return dmExtension::RESULT_OK;
	}
}
DM_DECLARE_EXTENSION(AcUtilExt, "AcUtil", AuBinding::APPOK, AuBinding::APPOK, AuBinding::Init, nullptr, nullptr, AuBinding::OK)