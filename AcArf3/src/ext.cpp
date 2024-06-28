// Extension Related Stuff for Arf3
#include <arf3.h>
using namespace Arf3_APIs;


/* Global Vars */
namespace Arf3 {
	uint64_t	mstime = 0, systime = 0;
	int8_t		mindt = -37, maxdt = 37, idelta = 0;

	// Runtime Params
	uint8_t		judge_range = 37;
	float		object_size_x = 324.0f, object_size_y = 324.0f;
	float		xscale, yscale, xdelta, ydelta, rotsin, rotcos, SIN, COS;
	bool		daymode, allow_anmitsu;

	// Internals
	uint64_t												dt_p1, dt_p2;
	std::unordered_map<uint32_t, uint16_t>					last_wgo;
	std::vector<ab>											blocked;
}
Arf3::Fumen* Arf = nullptr;


/* Provide NewTable Func for the Viewer Build */
#ifdef AR_BUILD_VIEWER
Arf3_API NewTable(lua_State* L) {
	lua_createtable( L, (int)luaL_checknumber(L, 1), (int)luaL_checknumber(L, 2) );
	return 1;
}
#endif


/* Defold Lifecycle Relateds */
static const luaL_reg Arf3APIs[] = {
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
	{"MakeArf", MakeArf}, {"DumpArf", DumpArf}, {"InitArf2", InitArf2}, {"NewTable", NewTable},
#endif
	{nullptr, nullptr}
};
inline dmExtension::Result Arf3LuaInit(dmExtension::Params* p) {
	/* Defold Restriction:
	 * Must Get the Lua Stack Balanced in the Initiation Process. */
	luaL_register(p->m_L, "Arf3", Arf3APIs);		lua_pop(p->m_L, 1);
	return dmExtension::RESULT_OK;
}
inline dmExtension::Result Arf3APPOK(dmExtension::AppParams* params) {
	return dmExtension::RESULT_OK;
}
inline dmExtension::Result Arf3OK(dmExtension::Params* params) {
	return dmExtension::RESULT_OK;
}
DM_DECLARE_EXTENSION(AcArf3, "AcArf3", Arf3APPOK, Arf3APPOK, Arf3LuaInit, nullptr, nullptr, Arf3OK)