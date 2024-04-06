// Defold Binding Related Stuff for Aerials
#pragma once


// Module "Arf2"
#include "p_functions.hpp"
static const luaL_reg Arf2[] =   // Considering Adding a "JudgeArfController" Function.
{

	{"InitArf", InitArf}, {"UpdateArf", UpdateArf}, {"FinalArf", FinalArf},
	{"SetIDelta", SetIDelta}, {"SetJudgeRange", SetJudgeRange}, {"JudgeArf", JudgeArf},

	{"SetXScale", SetXS}, {"SetYScale", SetYS}, {"SetXDelta", SetXD}, {"SetYDelta", SetYD},
	{"SetRotDeg", SetRotDeg}, {"SetDaymode", SetDaymode},

	{"NewTable", NewTable}, {"SetHintSize", SetHintSize}, {nullptr, nullptr}

};


inline dmExtension::Result Arf2LuaInit(dmExtension::Params* p) {
	/* Defold Restriction:
	 * Must Get the Lua Stack Balanced in the Initiation Process. */
	luaL_register(p->m_L, "Arf2", Arf2);		lua_pop(p->m_L, 1);
	return dmExtension::RESULT_OK;
}

inline dmExtension::Result Arf2OK(dmExtension::Params* params) {
	return dmExtension::RESULT_OK;
}

inline dmExtension::Result Arf2APPOK(dmExtension::AppParams* params) {
	return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(AcPlay, "AcPlay", Arf2APPOK, Arf2APPOK, Arf2LuaInit, nullptr, nullptr, Arf2OK)