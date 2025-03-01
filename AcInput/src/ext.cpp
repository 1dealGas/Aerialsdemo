/* Aerials Input
 * Copyright (c) 2025- 1dealGas Project, under the Apache 2.0 License.
 * This is a workaround for the input omission issue of Defold on Android Platform.
 */
#ifdef DM_PLATFORM_ANDROID
#include <android_native_app_glue.h>

using AndroidApp = android_app;
#include <android/input.h>
#include <dmsdk/sdk.h>


/* SPSC Task Queue
 * This utilizes arm64 natural atomicity & data dependency to avoid explicit thread safety primitives.
 * 1. Natural atomicity takes effect when assigning / reading an 8-byte-aligned 64b var.
 * 2. Data Dependency Requirement:
 *    Producer: Update buffer -> Update wIdx
 *    Consumer: Read wIdx -> Read buffer -> Update rIdx
 */
constexpr uint64_t QUEUE_MASK = 2047;
DM_ALIGNED(64) static struct {
	volatile uint64_t rIdx, _1_[7];
	volatile uint64_t wIdx, _2_[7];
			 uint64_t buffer[2048];
} Queue;


/* Producer Side */
union AcInputEvent {
	enum : uint8_t {
		GUI_PRESSED = 0, GUI_RELEASED = 1, MOVED = 2, CANCELLED = 3, PRESSED = 5, RELEASED = 6
	};
	struct {
		uint64_t x:24;					// float * 4096
		uint64_t pointerId:5, type:3;	// Android Pointer ID [0,31]
		uint64_t y:24;
	};
	uint64_t val;
};

static int32_t AcInputProduce(AndroidApp*, AInputEvent* event) noexcept {
	if( AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION ) {
		uint64_t wIdx = Queue.wIdx;
		const uint16_t action = AMotionEvent_getAction(event);
		const uint16_t type = action & 0xff, pointerIdx = action >> 8;

		if( type != AMOTION_EVENT_ACTION_MOVE ) {
			const AcInputEvent ae = {
				.x = (uint64_t)( AMotionEvent_getX(event, pointerIdx) * 4096 ),
				.y = (uint64_t)( AMotionEvent_getY(event, pointerIdx) * 4096 ),
				.pointerId = (uint64_t)AMotionEvent_getPointerId(event, pointerIdx),
				.type = type
			};
			Queue.buffer[( wIdx & QUEUE_MASK )] = ae.val;
			++wIdx;
		}
		else for( auto i = (int8_t)(AMotionEvent_getPointerCount(event) - 1); i != -1; --i ) {
			const AcInputEvent ae = {
				.x = (uint64_t)( AMotionEvent_getX(event, i) * 4096 ),
				.y = (uint64_t)( AMotionEvent_getY(event, i) * 4096 ),
				.pointerId = (uint64_t)AMotionEvent_getPointerId(event, i),
				.type = AMOTION_EVENT_ACTION_MOVE
			};
			Queue.buffer[( wIdx & QUEUE_MASK )] = ae.val;
			++wIdx;
		}

		Queue.wIdx = wIdx;
	}
	return 1;
}


/* Consumer Side */
static int AcInputConsume(lua_State* L) noexcept {
	/* Usage:
	 * local pCnt, rCnt, gPrs, isCcl = AcInput.Consume(pTbl, mTbl, rTbl)   -- Don't pass a dirty table into mTbl
	 */
	const uint64_t wIdx = Queue.wIdx;
		  uint64_t rIdx = Queue.rIdx, gPid = 0;
	uint8_t pressedCnt = 0, releasedCnt = 0, guiPressed = 0, isCancelled = 0;
	
	while( rIdx != wIdx ) {
		const AcInputEvent e = { .val = Queue.buffer[( rIdx & QUEUE_MASK )] };
		switch( e.type ) {
			case AcInputEvent::GUI_PRESSED:
				guiPressed = true, gPid = e.pointerId + 1;
			case AcInputEvent::PRESSED:
				lua_pushinteger( L, (lua_Integer)e.val ), lua_rawseti( L, 1, ++pressedCnt );
				break;
			case AcInputEvent::MOVED:
				lua_pushinteger( L, (lua_Integer)e.val ), lua_rawseti( L, 2, e.pointerId+1 );
				break;
			case AcInputEvent::GUI_RELEASED:
			case AcInputEvent::RELEASED:
				lua_pushinteger( L, e.pointerId+1 ), lua_rawseti( L, 3, ++releasedCnt );
				break;
			default:
				isCancelled = true;
		}
		++rIdx;
	}

	lua_pushinteger(L, pressedCnt), lua_pushinteger(L, releasedCnt);
	guiPressed ? lua_pushinteger(L, (lua_Integer)gPid) : lua_pushboolean(L, false);
	lua_pushboolean(L, isCancelled);

	Queue.rIdx = wIdx;
	return 4;
}

constexpr double RCP_4096 = 1.0 / 4096;
static int AcInputUnpack(lua_State* L) noexcept {
	/* Usage:
	 * local x, y, pointerLuaIdx = AcInput.Unpack(packedTouch)
	 */
	const AcInputEvent e = { .val = (uint64_t)lua_tointeger(L, 1) };
	lua_pushnumber( L, e.x * RCP_4096 ), lua_pushnumber( L, e.y * RCP_4096 );
	lua_pushinteger( L, e.pointerId+1 );
	return 3;
}

static int AcInputActivate(lua_State*) noexcept {
	/* Usage:
	 * AcInput.Activate()   -- Ready Then
	 */
	const auto app = (AndroidApp*)dmGraphics::GetNativeAndroidApp();
	return	   app -> onInputEvent = AcInputProduce, 0;
}


/* Defold Extension Related */
constexpr luaL_reg AcInputAPIs[] = {
	{"Activate", AcInputActivate},
	{"Consume", AcInputConsume},
	{"Unpack", AcInputUnpack},
	{nullptr, nullptr}
};
static dmExtension::Result AcInputInit(dmExtension::Params* p) noexcept {
	luaL_register(p->m_L, "AcInput", AcInputAPIs), lua_pop(p->m_L, 1);
	return dmExtension::RESULT_OK;
}
#else
#include <dmsdk/sdk.h>
	static dmExtension::Result AcInputInit(dmExtension::Params* p) noexcept {
		return dmExtension::RESULT_OK;
	}
#endif

static dmExtension::Result AcInputOK(dmExtension::Params*) noexcept {
	return dmExtension::RESULT_OK;
}
static dmExtension::Result AcInputAppOK(dmExtension::AppParams*) noexcept {
	return dmExtension::RESULT_OK;
}
DM_DECLARE_EXTENSION(AcInput, "AcInput", AcInputAppOK, AcInputAppOK, AcInputInit, nullptr, nullptr, AcInputOK)