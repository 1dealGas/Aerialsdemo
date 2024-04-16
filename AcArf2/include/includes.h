// Aerials Common Header
#pragma once														// Using "pragma once" Style.

// Sys Includes
#include <dmsdk/sdk.h>
#include <dmsdk/dlib/time.h>
#include <dmsdk/dlib/vmath.h>
#include <dmsdk/dlib/buffer.h>
#include <dmsdk/script/script.h>
#include <dmsdk/gameobject/gameobject.h>
#include <unordered_map>

// Aerials Includes
#include <arf2_generated.h>											// std::vector included
#include <ease_constants.h>

// Commons
struct ab { float a,b; };											// 2-float Structure
struct JudgeResult { uint8_t hit,early,late; bool sh_judged; };		// 4-uint Structure
uint64_t before, mstime, systime;									// Shared Vars

enum {
	HINT_NONJUDGED = 0, HINT_NONJUDGED_LIT,							// Hint Status Constants
	HINT_JUDGED, HINT_JUDGED_LIT, HINT_SWEEPED, HINT_AUTO,

	HINT_HIT = 0, HINT_EARLY, HINT_LATE,							// Hint Judge Constants

	T_WGO = 2, T_HGO, T_AGO_L, T_AGO_R,								// Table Update Constants
	T_WTINT, T_HTINT, T_ATINT
};

JudgeResult JudgeArf(const ab*, uint8_t, bool, bool);