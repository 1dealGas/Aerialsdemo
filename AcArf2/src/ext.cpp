// Defold Binding Related Stuff for Aerials
#pragma once


// Module "Arf2"
#include "p_functions.hpp"
static const luaL_reg Arf2[] =   // Considering Adding a "JudgeArfController" Function.
{
	{"InitArf", InitArf}, {"JudgeArf", JudgeArf}, {"UpdateArf", UpdateArf}, {"FinalArf", FinalArf},
	{"SetIDelta", SetIDelta}, {"SetAllowAnmitsu", SetAllowAnmitsu}, {"SetJudgeRange", SetJudgeRange}, {"SetHintSize", SetHintSize},
	{"SetCam", SetCam}, {"SetDaymode", SetDaymode}, {nullptr, nullptr}
};


// Lifecycle Funcs
inline dmExtension::Result Arf2LuaInit(dmExtension::Params* p) {
	/* Defold Restriction:
	 * Must Get the Lua Stack Balanced in the Initiation Process. */
	luaL_register(p->m_L, "Arf2", Arf2);		lua_pop(p->m_L, 1);
	return dmExtension::RESULT_OK;
}
inline dmExtension::Result Arf2OK(dmExtension::Params* params)  { return dmExtension::RESULT_OK; }
inline dmExtension::Result Arf2APPOK(dmExtension::AppParams* params)  { return dmExtension::RESULT_OK; }
DM_DECLARE_EXTENSION(AcPlay, "AcPlay", Arf2APPOK, Arf2APPOK, Arf2LuaInit, nullptr, nullptr, Arf2OK)