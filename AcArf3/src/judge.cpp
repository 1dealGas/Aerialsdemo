/* Arf3 Judge APIs */
#include <arf3.h>
using namespace Arf3;


#ifndef AR_BUILD_VIEWER
inline bool has_touch_near(const float cdx, const float cdy, const ab* valid_fingers, const uint8_t vf_count) {
	// Unpack the Object
	const float l = 900.0f - object_size_x * 0.5f + (cdx * rotcos - cdy * rotsin) * xscale + xdelta;
	const float d = 540.0f - object_size_y * 0.5f + (cdx * rotsin + cdy * rotcos) * yscale + ydelta;
	const float r = l + object_size_x, u = d + object_size_y;

	// Detect Touches
	for( uint8_t i=0; i<vf_count; i++ ) {
		const float x = valid_fingers[i].a;
		if( (x>=l) && x<=r ) {
			const float y = valid_fingers[i].b;
			if( (y>=d) && y<=u )
				return true;
		}
	}

	// Process Detection Failure
	return false;
}

inline bool anmitsu_register(const float cdx, const float cdy) {
	if( !allow_anmitsu ) return false;
	const float l = cdx - object_size_x, d = cdy - object_size_y;
	const float r = cdx + object_size_x, u = cdy + object_size_y;

	// Iterate blocked blocks
	for(const auto i : blocked)
		if( (i.a > l) && (i.a < r) )
			if( (i.b > d) && (i.b < u) )
				return false;

	// Register "Safe when Anmitsu" Objects
	const ab swao = {cdx, cdy};
	blocked.emplace_back(swao);
	return true;
}


Arf3_JUD JudgeArf(const ab* const vt, const uint8_t vtcount, const bool any_pressed, const bool any_released) {
	JudgeResult result = {};
	if( !Arf )						return result;
	if( any_released )				blocked.clear();

	// Calculate the Context Time
	const uint64_t judge_microsecond = dmTime::GetTime() - (systime - mstime*1000);
	const uint32_t mstime   = judge_microsecond / 1000;
		  uint32_t min_time = 0;

	// Prepare the Iteration Scale
	const uint32_t location_group =	mstime >> 9;
	const uint32_t index_size	  =	Arf->index.size();
	const uint16_t init_group	  =	location_group > 1 ? (location_group - 1) : 0 ;
		  uint16_t beyond_group	  =	init_group + 3;
				   beyond_group	  =	beyond_group < index_size ? beyond_group : index_size ;
	for(  uint16_t current_group = init_group; current_group < beyond_group; current_group++  ) {

		/* Wish -> EchoChild */
		for(const auto  current_wish_id : Arf->index[current_group].widx) {
				  auto& current_wish = Arf->wish[current_wish_id];
				  auto& current_ecs = current_wish.echochilds_ms_order;
			const auto  ec_count = current_ecs.size();
			if(!ec_count)																			continue;
			if(mstime - current_ecs[0]->ms < -370  ||  mstime - current_ecs[ec_count-1]->ms > 470)	continue;

			auto& entry = Arf->wish[current_wish_id].ec_ms_entry;
			while(entry > -1  &&  mstime-current_ecs[entry]->ms < -370)								entry--;

			const bool htn = has_touch_near(current_wish.current_cdx, current_wish.current_cdy, vt, vtcount);
			for( int16_t ei = entry+1; ei < ec_count; ei++ ) {
				EchoChild& current_echo = *current_ecs[ei];
				const int32_t dt = mstime - current_echo.ms;
				if( dt > 470 )		{ entry = ei; continue; }
				if( dt < -370 )		  break;

				switch(current_echo.status) {
					case NONJUDGED:   // No break here
					case NONJUDGED_LIT:
						current_echo.status = htn ? NONJUDGED_LIT : NONJUDGED;
						break;
					case JUDGED_LIT:
						if(!htn) {
							current_echo.judged_ms = mstime;
							current_echo.status = JUDGED;
							result.hit++;   // This means some Echoes will reach JUDGED by sweeping
						}
					default:;   // break omitted
				}

				if( current_echo.status == NONJUDGED_LIT  &&  dt > -100  &&  dt < 100  ) {
					anmitsu_register(current_wish.current_cdx, current_wish.current_cdy);
					current_echo.status = JUDGED_LIT;
					min_time = 4294967295;   // Means that the min_time strategy is banned
				}
			}
		}

		/* Echo */
		for( const auto current_echo_id : Arf->index[current_group].eidx ) {
			auto& current_echo = Arf->echo[current_echo_id];
			const int32_t dt = mstime - current_echo.ms;
			if(dt < -370  ||  dt > 982)		break;   // when dt>982, all objs in this group meet dt>470
			if(dt > 470)					continue;

			const bool htn = has_touch_near(current_echo.c_dx, current_echo.c_dy, vt, vtcount);
			switch(current_echo.status) {
				case NONJUDGED:   // No break here
				case NONJUDGED_LIT:
					current_echo.status = htn ? NONJUDGED_LIT : NONJUDGED;
					break;
				case JUDGED_LIT:
					if(!htn) {
						current_echo.judged_ms = mstime;
						current_echo.status = JUDGED;
						result.hit++;   // This means some Echoes will reach the JUDGED status by sweeping
					}
				default:;   // break omitted
			}

			if( current_echo.status == NONJUDGED_LIT  &&  dt > -100  &&  dt < 100  ) {
				anmitsu_register(current_echo.c_dx, current_echo.c_dy);
				current_echo.status = JUDGED_LIT;
				min_time = 4294967295;   // Means that the min_time strategy is banned
			}
		}

		/* Hint */
		if(any_pressed) {
			for( const auto current_hint_id : Arf->index[current_group].hidx ) {
				auto& current_hint = Arf->hint[current_hint_id];
				const int32_t dt = mstime - current_hint.ms;
				if(dt < -370  ||  dt > 982)		break;   // when dt>982, all objs in this group meet dt>470
				if(dt > 470)					continue;

				const bool htn = has_touch_near(current_hint.c_dx, current_hint.c_dy, vt, vtcount);
				switch(current_hint.status) {
					case NONJUDGED:   // No break here
					case NONJUDGED_LIT:
						current_hint.status = htn ? NONJUDGED_LIT : NONJUDGED;
						break;
					case JUDGED_LIT:
						if(!htn)
							current_hint.status = JUDGED;
					default:;   // break omitted
				}

				// Judge the Hint
				if( current_hint.status == NONJUDGED_LIT  &&  dt > -100  &&  dt < 100 ) {
					const bool iswa = anmitsu_register(current_hint.c_dx, current_hint.c_dy);
					if( !min_time )													/* Register */
						min_time = current_hint.ms;
					else if( min_time == current_hint.ms )
						{  }
					else if( !iswa )
						continue;

					current_hint.judged_ms = mstime;								/* Status Update */
					current_hint.status = JUDGED_LIT;
					if(current_hint_id == Arf->special_hint)
						result.sh_judged = (bool)(Arf->special_hint);

					if(dt < mindt)													/* Classify */
						result.early++,	current_hint.elstatus = EARLY;
					else if(dt <= maxdt)
						result.hit++;
					else
						result.late++,	current_hint.elstatus = LATE;
				}
			}
		}
		else {
			for( const auto current_hint_id : Arf->index[current_group].hidx ) {
				auto& current_hint = Arf->hint[current_hint_id];
				const int32_t dt = mstime - current_hint.ms;
				if(dt < -370  ||  dt > 982)		break;   // when dt>982, all objs in this group meet dt>470
				if(dt > 470)					continue;

				const bool htn = has_touch_near(current_hint.c_dx, current_hint.c_dy, vt, vtcount);
				switch(current_hint.status) {
					case NONJUDGED:   // No break here
					case NONJUDGED_LIT:
						current_hint.status = htn ? NONJUDGED_LIT : NONJUDGED;
						break;
					case JUDGED_LIT:
						if(!htn)
							current_hint.status = JUDGED;
					default:;   // break omitted
				}
			}
		}
	}
		return result;
}

Arf3_API JudgeArf(lua_State* L) {
	// Unpack Touches
	ab vt[10];
	uint8_t vtcount = 0, any_pressed = false, any_released = false;
	for( uint8_t i=0; i<10; i++ ) {
		const dmVMath::Vector3* f = (lua_rawgeti(L, 1, i+1), dmScript::CheckVector3(L, -1));
		switch( lua_pop(L,1), (uint8_t)f->getZ() ) {
			case 1:
				any_pressed = true;   // No break here
			case 2:
				vt[vtcount].a = f->getX();		vt[vtcount].b = f->getY();		vtcount++;
				break;
			case 3:
				any_released = true;
			default:;   // break omitted
		}
	}

	// Do Returns
	const auto result = JudgeArf(vt, vtcount, any_pressed, any_released);
	lua_pushnumber(L, result.hit);			lua_pushnumber(L, result.early);
	lua_pushnumber(L, result.late);			lua_pushboolean(L, result.sh_judged);
	return 4;
}
#endif