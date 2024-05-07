// Extension Related Stuff for Arf3
#include <arf3.h>
#pragma once


/* Global Vars */
namespace Arf3 {
	uint64_t	mstime = 0, systime = 0;
	int8_t		mindt = -37, maxdt = 37, idelta = 0;

	// Runtime Params
	uint8_t		judge_range = 37;
	float		object_size_x = 360.0f, object_size_y = 450.0f;
	float		xscale, yscale, xdelta, ydelta, rotsin, rotcos, SIN, COS;
	bool		daymode, allow_anmitsu;

	// Internals
	uint64_t												dt_p1, dt_p2;
	std::unordered_map<uint32_t, uint16_t>					last_wgo;
	std::vector<ab>											blocked;
}
Arf3::Fumen* Arf = nullptr;


/* Arf3 APIs */
using namespace Arf3_APIs;
static constexpr luaL_reg Arf3APIs[] = {
	{"InitArf3", InitArf3}, {"FinalArf", FinalArf}, {"UpdateArf", UpdateArf},
	{"SetDaymode", SetDaymode}, {"SetCam", SetCam},
#ifndef AR_BUILD_VIEWER
	{"SetHintSize", SetHintSize}, {"SetIDelta", SetIDelta}, {"JudgeArf", JudgeArf},
	{"SetJudgeRange", SetJudgeRange}, {"SetAllowAnmitsu", SetAllowAnmitsu},
	#ifdef AR_WITH_EXPORTER
		{"DumpArf", DumpArf},
	#endif
	#ifdef AR_COMPATIBILITY
		{"InitArf2", InitArf2},
	#endif
#else
	{"MakeArf", MakeArf}, {"DumpArf", DumpArf}, {"InitArf2", InitArf2}
#endif
};


/* Defold Lifecycle Related Stuff */
inline dmExtension::Result Arf3LuaInit(dmExtension::Params* p) {
	luaL_register(p->m_L, "Arf3", Arf3APIs);
	lua_pop(p->m_L, 1);   // Defold Restriction, Must Get the Lua Stack Balanced in the Initiation Process.
	return dmExtension::RESULT_OK;
}

inline dmExtension::Result Arf3APPOK(dmExtension::AppParams* params) {
	return dmExtension::RESULT_OK;
}

inline dmExtension::Result Arf3OK(dmExtension::Params* params) {
	return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(AcArf3, "AcArf3", Arf3APPOK, Arf3APPOK, Arf3LuaInit, nullptr, nullptr, Arf3OK)