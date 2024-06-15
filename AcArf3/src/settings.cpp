/* Arf3 Setting APIs */
#include <arf3.h>
using namespace Arf3;


Arf3_API SetDaymode(lua_State *L) {
	return (daymode = lua_toboolean(L,1), 0);
}

Arf3_API SetCam(lua_State *L) {
	xscale = luaL_checknumber(L, 1);		yscale = luaL_checknumber(L, 2);
	xdelta = luaL_checknumber(L, 3);		ydelta = luaL_checknumber(L, 4);
	return (GetSINCOS( luaL_checknumber(L,5) ), rotsin = SIN, rotcos = COS, 0);
}

#ifndef AR_BUILD_VIEWER
	Arf3_API SetAllowAnmitsu(lua_State* L) {
		return (allow_anmitsu = lua_toboolean(L,1), 0);
	}

	Arf3_API SetIDelta(lua_State* L) {
		idelta = luaL_checknumber(L, 1);
		mindt = idelta - judge_range;			mindt = (mindt < -100) ? -100 : mindt;
		maxdt = idelta + judge_range;			maxdt = (maxdt >  100) ?  100 : maxdt;
		return 0;
	}

	Arf3_API SetJudgeRange(lua_State *L) {
		const uint8_t jr = luaL_checknumber(L, 1);
		judge_range = (jr > 100) ? 100 : jr;	judge_range = (jr < 1) ?   1  : jr;
		mindt = idelta - judge_range;			mindt = (mindt < -100) ? -100 : mindt;
		maxdt = idelta + judge_range;			maxdt = (maxdt >  100) ?  100 : maxdt;
		return 0;
	}

	Arf3_API SetHintSize(lua_State* L) {
		lua_Number object_size_x_script = luaL_checknumber(L, 1);
		lua_Number object_size_y_script = object_size_x_script;
		if( lua_isnumber(L,2) )
			object_size_y_script = luaL_checknumber(L, 2);

		object_size_x_script = (object_size_x_script > 2.88) ? object_size_x_script : 2.88 ;
		object_size_x_script = (object_size_x_script < 48.0) ? object_size_x_script : 48.0 ;
		object_size_y_script = (object_size_y_script > 2.88) ? object_size_y_script : 2.88 ;
		object_size_y_script = (object_size_y_script < 24.0) ? object_size_y_script : 24.0 ;

		object_size_x = object_size_x_script * 112.5f;
		object_size_y = object_size_y_script * 112.5f;
		return 0;
	}
#endif