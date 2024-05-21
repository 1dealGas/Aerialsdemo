/* Arf3 Update API */
#include <arf3.h>
using namespace Arf3;


//
	/* Here we store some tint values, which are changed rarely. */
	static constexpr auto H_EARLY_R = 0.275f, H_EARLY_G = 0.495f, H_EARLY_B = 0.5603125f;
	static constexpr auto H_LATE_R = 0.5603125f, H_LATE_G = 0.3403125f, H_LATE_B = 0.275f;
	static constexpr auto H_HIT_R = 0.73f, H_HIT_G = 0.6244921875f, H_HIT_B = 0.4591015625f;

	static constexpr auto A_EARLY_R = 0.3125f, A_EARLY_G = 0.5625f, A_EARLY_B = 0.63671875f;
	static constexpr auto A_LATE_R = 0.63671875f, A_LATE_G = 0.38671875f, A_LATE_B = 0.3125f;
	static constexpr auto A_HIT_R = 1.0f, A_HIT_G = 0.85546875f, A_HIT_B = 0.62890625f;

	/* Here we use some typedefs to simplify our codes. */
	typedef dmGameObject::HInstance GO;
	typedef dmVMath::Vector3 v3i, *v3;					typedef dmVMath::Point3 p3;
	typedef dmVMath::Vector4 v4i, *v4;					typedef dmVMath::Quat Qt;

	/* Here are some Quat Utils. */
	static const Qt D73(0.0f, 0.0f, 0.594822786751341f, 0.803856860617217f);
	inline Qt GetZQuad(const double degree) { return GetSINCOS(degree*0.5), Qt(0.0f,0.0f, SIN,COS); }
//


/* Object sweeping behaviors are gathered here. */
#ifndef AR_BUILD_VIEWER
inline JudgeResult SweepObjects(const uint16_t init_group, const uint16_t beyond_group) {
	JudgeResult result = {};
	for( uint16_t current_group = init_group; current_group < beyond_group; current_group++ ) {

		/* Wish -> EchoChild */
		for(const auto  current_wish_id : Arf->index[current_group].widx) {
				  auto& current_wish = Arf->wish[current_wish_id];
				  auto& current_ecs = current_wish.echochilds_ms_order;
			const auto  ec_count = current_ecs.size();
			if(!ec_count)																		continue;
			if(mstime-current_ecs[0]->ms < 0  ||  mstime-current_ecs[ec_count-1]->ms > 470)		continue;

			auto& entry = Arf->wish[current_wish_id].ec_ms_entry;
			while(entry > -1  &&  mstime-current_ecs[entry]->ms < 0)							entry--;

			for( int16_t ei = entry+1; ei < ec_count; ei++ ) {
				EchoChild& current_echo = *current_ecs[ei];
				const int32_t dt = mstime - current_echo.ms;
				if( dt > 470 )		{ entry = ei; continue; }
				if( dt < 0 )		  break;

				if( dt > 100  &&  current_echo.status <= NONJUDGED_LIT )   //  NONJUDGED -> 0   N_L -> 1
					result.late++, current_echo.status = LOST;
				else if( current_echo.status==JUDGED_LIT )
					result.hit++, current_echo.status = JUDGED, current_echo.judged_ms = mstime;
			}
		}

		/* Echo */
		for( const auto current_echo_id : Arf->index[current_group].eidx ) {
			auto& current_echo = Arf->echo[current_echo_id];
			const int32_t dt = mstime - current_echo.ms;
			if(dt < 0  ||  dt > 982)		break;   // when dt>982, all objs in this group meet dt>470
			if(dt > 470)					continue;

			if( dt > 100  &&  current_echo.status <= NONJUDGED_LIT )   //  NONJUDGED -> 0   N_L -> 1
				result.late++, current_echo.status = LOST;
			else if( current_echo.status==JUDGED_LIT )
				result.hit++, current_echo.status = JUDGED, current_echo.judged_ms = mstime;
		}

		/* Hint */
		for( const auto current_hint_id : Arf->index[current_group].hidx ) {
			auto& current_hint = Arf->hint[current_hint_id];
			const int32_t dt = mstime - current_hint.ms;
			if( dt<100 )
				break;
			if( current_hint.status <= NONJUDGED_LIT )   //  NONJUDGED -> 0   N_L -> 1
				result.late++, current_hint.status = LOST;
		}
	}
	return result;
}
#endif


/* Param Setting Funcs */
using cfloat = const float;
inline void UseWgo(lua_State* L, uint16_t& wgo_used, cfloat x, cfloat y, cfloat z, float& w) {
	if( x>=-36.0f && x<=1836.0f ) {   // X trim
		if( y>=-36.0f && y<=1116.0f ) {   // Y trim
			const uint32_t k = (uint32_t)(x * 1009.0f) + (uint32_t)(y * 1013.0f);
			if( last_wgo.count(k) ) {   // Overlap trim
				const uint16_t lwidx_lua = last_wgo[k];

				// tint.w Setting
				lua_pushnumber(L, 1.0);
				lua_rawseti(L, WTINT, lwidx_lua);

				// Scale Setting
				lua_rawgeti(L, WGO, lwidx_lua);
				SetScale(dmScript::CheckGOInstance(L, -1), 0.637f);
				lua_pop(L, 1);
			}
			else {   // Pass
				wgo_used++;   // This functions as the wgo_used result and lua index simultaneously
				lua_pushnumber(L, w);
				lua_rawseti(L, WTINT, wgo_used);

				// Pos & Scale Setting
				lua_rawgeti(L, WGO, wgo_used);
				const GO WGO = dmScript::CheckGOInstance(L, -1);
				lua_pop(L, 1);		SetPosition( WGO, p3(x, y, z) );
				w = 1.0f - w;		SetScale( WGO, 0.637f + 0.437f * w*w );
				last_wgo[k] = wgo_used;			// As 0.637f + 0.437f - 0.437f * (1 - tintw * tintw)
			}
		}
	}
}

inline void UseHgo(lua_State* L, uint16_t& hgo_used, uint16_t& ago_used, cfloat dt, cfloat jdt,
				   const uint8_t status, const uint8_t elstatus, cfloat x, cfloat y) {
	// Actually this could be merged into the main update func, with no avoidable duplication.
	// This is just for some isologue considerations.

	/* Prepare Rennder Elements */
	lua_rawgeti(L, HGO, hgo_used+1);		const GO hgo	= dmScript::CheckGOInstance(L, -1);
	lua_rawgeti(L, AGOL, ago_used+1);		const GO agol	= dmScript::CheckGOInstance(L, -1);
	lua_rawgeti(L, AGOR, ago_used+1);		const GO agor	= dmScript::CheckGOInstance(L, -1);
	lua_rawgeti(L, HTINT, hgo_used+1);		const v4 htint	= dmScript::CheckVector4(L, -1);
	lua_rawgeti(L, ATINT, ago_used+1);		const v4 atint	= dmScript::CheckVector4(L, -1);
	lua_pop(L, 5);

	/* Pass Hint & Anim Params */
	// Specify all tint.w as 1 in the initialization
	if( dt < -370 ) {
		SetPosition( hgo, p3(x, y, dt*0.00001f - 0.04f) );
		const float color = 0.1337f + (float)(0.0005 * (510+dt) );   // As 0.07/140
		htint -> setX(color).setY(color).setZ(color);
		hgo_used++;
	}
	else if( dt <= 370 ) switch(status) {
		case NONJUDGED:
			htint -> setX(0.2037f).setY(0.2037f).setZ(0.2037f);
			SetPosition( hgo, p3(x, y, -0.04f) );
			hgo_used++;
			break;
		case NONJUDGED_LIT:
			htint -> setX(0.3737f).setY(0.3737f).setZ(0.3737f);
			SetPosition( hgo, p3(x, y, -0.037f) );
			hgo_used++;
			break;
		case JUDGED_LIT:   // No break here
			SetPosition( hgo, p3(x, y, -0.01f) );
			switch(elstatus) {
				case COMMON:
					if(daymode)		htint -> setX(H_HIT_R).setY(H_HIT_G).setZ(H_HIT_B);
					else			htint -> setX(H_HIT_R).setY(H_HIT_R).setZ(H_HIT_R);
					break;
				case EARLY:
					htint -> setX(H_EARLY_R).setY(H_EARLY_G).setZ(H_EARLY_B);
					break;
				case LATE:
					htint -> setX(H_LATE_R).setY(H_LATE_G).setZ(H_LATE_B);
				default:;   // break omitted
			}
			hgo_used++;
		case JUDGED:
			if(jdt <= 370) {
				// Anim Settings
				ago_used++;

				/* Position */ {
					SetPosition( agol, p3(x, y, -jdt * 0.00001f) );
					SetPosition( agor, p3(x, y, 0.00001f - jdt * 0.00001f) );
				}

				/* tint.xyz */
				switch(elstatus) {
					case COMMON:
						if(daymode)		atint -> setX(A_HIT_R).setY(A_HIT_G).setZ(A_HIT_B);
						else			atint -> setX(A_HIT_R).setY(A_HIT_R).setZ(A_HIT_R);   // 1.0f
						break;
					case EARLY:
						atint -> setX(A_EARLY_R).setY(A_EARLY_G).setZ(A_EARLY_B);
						break;
					case LATE:
						atint -> setX(A_LATE_R).setY(A_LATE_G).setZ(A_LATE_B);
					default:;   // break omitted
				}

				/* tint.w */
				if(jdt < 73) {
					float tintw = jdt * 0.01f;
					tintw = 0.637f * tintw * (2.0f - tintw);
					atint -> setW( 0.17199f + tintw );
				}
				else {
					float tintw = (jdt - 73) * 0.003367003367003367f;   // 1/297 = 0.003367···
					tintw = 0.637f * tintw * (2.0f - tintw);
					atint -> setW(0.637f - tintw);
				}

				/* Rotation & Scale */ {
					double anim_calculate;
					if(jdt <= 193) {
						anim_calculate = jdt * 0.005181347150259;   // 1/193
						SetRotation( agol, GetZQuad(45.0 + 28.0*anim_calculate) );
						anim_calculate = anim_calculate * (2.0 - anim_calculate);
						SetScale( agol, 1.0 + 0.637 * anim_calculate );
					}
					else {
						SetRotation(agol, D73);
						SetScale(agol, 1.637f);
					}

					anim_calculate = jdt * 0.002702702702702;   // 1/370
					anim_calculate = anim_calculate * (2.0 - anim_calculate);
					SetRotation( agor, GetZQuad(45.0 - 8.0 * anim_calculate) );
					SetScale( agor, 1.0 + 0.637 * anim_calculate );
				}
			}
			break;
		case LOST:
			{
				SetPosition( hgo, p3(x, y, -0.02f + dt*0.00001f) );
				float color = 0.437f - dt*0.00037f;			htint -> setX(color);
				color *= 0.51f;								htint -> setY(color).setZ(color);
				hgo_used++;
			}
			break;
		case AUTO:
			// Hint
			if(dt < 0) {
				htint -> setX(0.2037f).setY(0.2037f).setZ(0.2037f);
				SetPosition( hgo, p3(x, y, -0.04f) );
				hgo_used++;
			}
			else if(dt < 101) {
				SetPosition( hgo, p3(x, y, -0.01f) );
				if(daymode)		htint -> setX(H_HIT_R).setY(H_HIT_G).setZ(H_HIT_B);
				else 			htint -> setX(H_HIT_R).setY(H_HIT_R).setZ(H_HIT_R);   // 0.73f
				hgo_used++;
			}

			// Anim
			if(dt >= 0) {
				ago_used++;

				/* Position */ {
					SetPosition( agol, p3(x, y, -dt * 0.00001f) );
					SetPosition( agor, p3(x, y, 0.00001f - dt * 0.00001f) );
				}
				/* tint.xyz */ {
					if(daymode)		atint -> setX(A_HIT_R).setY(A_HIT_G).setZ(A_HIT_B);
					else			atint -> setX(A_HIT_R).setY(A_HIT_R).setZ(A_HIT_R);   // 1.0f
				}

				/* tint.w */
				if(dt < 73) {
					float tintw = dt * 0.01f;
					tintw = 0.637f * tintw * (2.0f - tintw);
					atint -> setW( 0.17199f + tintw );
				}
				else {
					float tintw = (dt - 73) * 0.003367003367003367f;   // 1/297 = 0.003367···
					tintw = 0.637f * tintw * (2.0f - tintw);
					atint -> setW(0.637f - tintw);
				}

				/* Rotation & Scale */
				{
					double anim_calculate;
					if(dt <= 193) {
						anim_calculate = dt * 0.005181347150259;   // 1/193
						SetRotation( agol, GetZQuad(45.0 + 28.0*anim_calculate) );
						anim_calculate = anim_calculate * (2.0 - anim_calculate);
						SetScale( agol, 1.0 + 0.637 * anim_calculate );
					}
					else {
						SetRotation(agol, D73);
						SetScale(agol, 1.637f);
					}

					anim_calculate = dt * 0.002702702702702;   // 1/370
					anim_calculate = anim_calculate * (2.0 - anim_calculate);
					SetRotation( agor, GetZQuad(45.0 - 8.0*anim_calculate) );
					SetScale( agor, 1.0 + 0.637 * anim_calculate );
				}
			}
		default:;   // break omitted
	}
	else if( jdt<=370 && (status==JUDGED || status==JUDGED_LIT) ) {
		ago_used++;   // We only render the Anim here.

		/* Position */ {
			SetPosition( agol, p3(x, y, -jdt * 0.00001f) );
			SetPosition( agor, p3(x, y, 0.00001f - jdt * 0.00001f) );
		}

		/* tint.xyz */ {
			if(elstatus == LATE)
				atint -> setX(A_LATE_R).setY(A_LATE_G).setZ(A_LATE_B);
			else if(daymode)
				atint -> setX(A_HIT_R).setY(A_HIT_G).setZ(A_HIT_B);
			else
				atint -> setX(A_HIT_R).setY(A_HIT_R).setZ(A_HIT_R);   // 1.0f
		}

		/* tint.w */ {   // jdt must be larger than 270
			float tintw = (jdt - 73) * 0.003367003367003367f;   // 1/297 = 0.003367···
			tintw = 0.637f * tintw * (2.f - tintw);
			atint -> setW(0.637f - tintw);
		}

		/* Rotation & Scale */ {   // jdt must be larger than 270
			SetRotation(agol, D73);
			SetScale(agol, 1.637f);

			double anim_calculate = jdt * 0.002702702702702;   // 1/370
			anim_calculate = anim_calculate * (2.0 - anim_calculate);
			SetRotation( agor, GetZQuad(45.0 - 8.0*anim_calculate) );
			SetScale( agor, 1.0 + 0.637 * anim_calculate );
		}
	}
}

inline void UseEgo(lua_State* L, uint16_t& ego_used, cfloat dt, const uint8_t status,
				   cfloat x, cfloat y, cfloat z, float& w) {
	const auto ego_used_before = ego_used;
	const auto ego_pos = p3(x, y, z);

	/* Prepare Rennder Elements */
	lua_rawgeti(L, EGO, ego_used+1);		const GO ego	= dmScript::CheckGOInstance(L, -1);
	lua_rawgeti(L, ETINT, ego_used+1);		const v4 etint	= dmScript::CheckVector4(L, -1);
	lua_pop(L, 2);

	/* Hint-like Behaviors: Pass Echo Params */
	if( dt < -370 ) {
		SetPosition(ego, ego_pos);
		const float color = 0.1337f + (float)( dt<-510 ? 0 : 0.0005 * (510+dt) );
		etint -> setX(color).setY(color).setZ(color);
		ego_used++;
	}
	else if( dt <= 370 ) switch(status) {
		case NONJUDGED:
			etint -> setX(0.2037f).setY(0.2037f).setZ(0.2037f);
			SetPosition(ego, ego_pos);
			ego_used++;
			break;
		case NONJUDGED_LIT:
			etint -> setX(0.3737f).setY(0.3737f).setZ(0.3737f);
			SetPosition(ego, ego_pos);
			ego_used++;
			break;
		case JUDGED_LIT:
			SetPosition(ego, ego_pos);
			if(daymode)		etint -> setX(H_HIT_R).setY(H_HIT_G).setZ(H_HIT_B);
			else			etint -> setX(H_HIT_R).setY(H_HIT_R).setZ(H_HIT_R);
			ego_used++;
			break;
		case LOST:
			{
				SetPosition(ego, ego_pos);
				float color = 0.437f - dt*0.00037f;			etint -> setX(color);
				color *= 0.51f;								etint -> setY(color).setZ(color);
				ego_used++;
			}
			break;
		case AUTO:
			SetPosition(ego, ego_pos);
			if(dt < 0)
				etint -> setX(0.2037f).setY(0.2037f).setZ(0.2037f);
			else if(dt < 101) {
				if(daymode)		etint -> setX(H_HIT_R).setY(H_HIT_G).setZ(H_HIT_B);
				else 			etint -> setX(H_HIT_R).setY(H_HIT_R).setZ(H_HIT_R);   // 0.73f
			}
			ego_used++;   // Break omitted
		default:;
	}

	/* Wish-like Behaviors */
	if(ego_used != ego_used_before) {
		etint -> setW(w);
		w = 1.0f - w;			SetScale( ego, 0.637f + 0.437f * w*w );
		lua_pop(L, 1);			// As 0.637f + 0.437f - 0.437f * (1 - tintw * tintw)
	}
}

inline void UseAgoForEgo(lua_State* L, uint16_t& ago_used, cfloat dt, float jdt,
						 const uint8_t status, cfloat ax, cfloat ay) {
	/* Prepare Rennder Elements */
	lua_rawgeti(L, AGOL, ago_used+1);		const GO agol	= dmScript::CheckGOInstance(L, -1);
	lua_rawgeti(L, AGOR, ago_used+1);		const GO agor	= dmScript::CheckGOInstance(L, -1);
	lua_rawgeti(L, ATINT, ago_used+1);		const v4 atint	= dmScript::CheckVector4(L, -1);
	lua_pop(L, 3);

	if( dt <= 370 ) switch(status) {
		case AUTO:
			if( dt < 0 )  break;
			jdt = dt;
		case JUDGED_LIT:
		case JUDGED:
			if(jdt <= 370) {
				ago_used++;

				/* Position */ {
					SetPosition( agol, p3(ax, ay, -jdt * 0.00001f) );
					SetPosition( agor, p3(ax, ay, 0.00001f - jdt * 0.00001f) );
				}
				/* tint.xyz */
				if(daymode)		atint -> setX(A_HIT_R).setY(A_HIT_G).setZ(A_HIT_B);
				else			atint -> setX(A_HIT_R).setY(A_HIT_R).setZ(A_HIT_R);   // 1.0f

				/* tint.w */
				if(jdt < 73) {
					float tintw = jdt * 0.01f;
					tintw = 0.637f * tintw * (2.0f - tintw);
					atint -> setW( 0.17199f + tintw );
				}
				else {
					float tintw = (jdt - 73) * 0.003367003367003367f;   // 1/297 = 0.003367···
					tintw = 0.637f * tintw * (2.0f - tintw);
					atint -> setW(0.637f - tintw);
				}

				/* Rotation & Scale */ {
					double anim_calculate;
					if(jdt <= 193) {
						anim_calculate = jdt * 0.005181347150259;   // 1/193
						SetRotation( agol, GetZQuad(45.0 + 28.0*anim_calculate) );
						anim_calculate = anim_calculate * (2.0 - anim_calculate);
						SetScale( agol, 1.0 + 0.637 * anim_calculate );
					}
					else {
						SetRotation(agol, D73);
						SetScale(agol, 1.637f);
					}
					anim_calculate = jdt * 0.002702702702702;   // 1/370
					anim_calculate = anim_calculate * (2.0 - anim_calculate);
					SetRotation( agor, GetZQuad(45.0 - 8.0 * anim_calculate) );
					SetScale( agor, 1.0 + 0.637 * anim_calculate );
				}
			}
		default:;   // break omitted
	}
	else if( jdt<=370 && (status==JUDGED || status==JUDGED_LIT) ) {
		ago_used++;

		/* Position */ {
			SetPosition( agol, p3(ax, ay, -jdt * 0.00001f) );
			SetPosition( agor, p3(ax, ay, 0.00001f - jdt * 0.00001f) );
		}
		/* tint.xyz */ {
			if(daymode)
				atint -> setX(A_HIT_R).setY(A_HIT_G).setZ(A_HIT_B);
			else
				atint -> setX(A_HIT_R).setY(A_HIT_R).setZ(A_HIT_R);   // 1.0f
		}
		/* tint.w */ {   // jdt must be larger than 270
			float tintw = (jdt - 73) * 0.003367003367003367f;   // 1/297 = 0.003367···
			tintw = 0.637f * tintw * (2.f - tintw);
			atint -> setW(0.637f - tintw);
		}

		/* Rotation & Scale */ {   // jdt must be larger than 270
			SetRotation(agol, D73);
			SetScale(agol, 1.637f);

			double anim_calculate = jdt * 0.002702702702702;   // 1/370
			anim_calculate = anim_calculate * (2.0 - anim_calculate);
			SetRotation( agor, GetZQuad(45.0 - 8.0*anim_calculate) );
			SetScale( agor, 1.0 + 0.637 * anim_calculate );
		}
	}
}


/* Main Update Func */
// Z Distribution: Hint&Anim(-0.06,0)  Layer1{0,0.02,0.04}  Layer2{0.01,0.03,0.05}  Echo{0.06}
Arf3_API UpdateArf(lua_State* L) {
	if( !Arf )	return 0;

	/* Time */
	mstime	= (uint32_t)luaL_checknumber(L, 1); {
		if(mstime < 2)							mstime = 2;
		else if(mstime >= Arf->before)			return 0;
	}
	systime = dmTime::GetTime();

	/* Group */
	const uint32_t	location_group	= mstime >> 9;
	const uint16_t	index_size		= Arf -> index.size();
	const uint16_t  init_group		= location_group > 1 ? (location_group - 1) : 0 ;
		  uint16_t	beyond_group	= init_group + 3;
					beyond_group	= beyond_group < index_size ? beyond_group : index_size ;

	/* Misc Preparations */
#ifdef AR_BUILD_VIEWER
	constexpr		JudgeResult    sweep_result = {};
#else
	const auto		sweep_result = SweepObjects(init_group, beyond_group);
#endif
	const double	dt1 = QueryDt(mstime, Arf->d1, dt_p1), dt2 = QueryDt(mstime, Arf->d2, dt_p2);
		  uint16_t  wgo_used = 0, hgo_used = 0, ego_used = 0, ago_used = 0;


	/* Wish, WishChild, EchoChild */
	for(const auto  wish_cid : Arf->index[location_group].widx) {
			  auto& wish_c = Arf->wish[wish_cid];
		const auto  current_dt = wish_c.of_layer2 ? dt2 : dt1 ;

		/* Nodes */
		const auto& nodes = wish_c.nodes;
			  auto& node_x = wish_c.current_cdx;			auto& node_y = wish_c.current_cdy;
		{
			// Jump Judging
			if(mstime < nodes[0].ms) continue;   // Wish For loop
			const uint16_t nodes_tail = nodes.size() - 1;
			if(mstime >= nodes[nodes_tail].ms)  continue;   // Wish For loop

			// Regression
			auto& entry = wish_c.n_entry;
			while( entry > 0  &&  mstime < nodes[entry].ms )
				entry--;

			// Pos Acquisition
			while(entry < nodes_tail) {   // [)
				const auto& node_c = nodes[ entry ];
				const auto& node_n = nodes[ entry + 1 ];
				if(mstime < node_n.ms) {
					if(node_c.easetype) {   // Easing.
						float ratio; {
							const uint32_t difms = node_n.ms - node_c.ms;
							if(!difms)				ratio = 0.0f;
							else if(difms<8193)		ratio = (mstime - node_c.ms) * RCP[ difms-1 ];
							else					ratio = (mstime - node_c.ms) / (float)difms;
						}
						if(node_c.easetype > LINEAR) {
							// Get fr
							float xfr = 0, yfr;
							xfr = yfr = node_c.ci + (node_c.ce - node_c.ci) * ratio ;
							switch(node_c.easetype) {
								case LCIRC:   /*  x -> ESIN   y -> ECOS  */
									xfr = ESIN[ (uint16_t)(1000 * ratio) ];
									yfr = ECOS[ (uint16_t)(1000 * ratio) ];
									break;
								case RCIRC:   /*  x -> ECOS   y -> ESIN  */
									xfr = ECOS[ (uint16_t)(1000 * ratio) ];
									yfr = ESIN[ (uint16_t)(1000 * ratio) ];
									break;
								case INQUAD:
									xfr = yfr = (ratio * ratio);
									break;
								case OUTQUAD:
									ratio = 1.0f - ratio;
									xfr = yfr = (1.0f - ratio * ratio);
								default:;   // break omitted
							}

							// Apply the Formula
							// P = ( (fce - fr) * p1 + (fr - fci) * p2 ) * p_dnm
							// P = p1 + fr * (p2 - p1)   when fce=1, fci = 0
							if( node_c.ci != 0.0f  ||  node_c.ce != 1.0f ) {
								node_x = (node_c.x_fce-xfr) * node_c.c_dx + (xfr-node_c.x_fci) * node_n.c_dx;
								node_x *= node_c.x_dnm;
								node_y = (node_c.y_fce-yfr) * node_c.c_dy + (yfr-node_c.y_fci) * node_n.c_dy;
								node_y *= node_c.y_dnm;
							}
							else {
								node_x = node_n.c_dx - node_c.c_dx;   // As dx
								node_x = node_c.c_dx + node_x * xfr;
								node_y = node_n.c_dy - node_c.c_dy;   // As dy
								node_y = node_c.c_dy + node_y * yfr;
							}
						}
						else {
							node_x = node_n.c_dx - node_c.c_dx;   // As dx
							node_x = node_c.c_dx + node_x * ratio;
							node_y = node_n.c_dy - node_c.c_dy;   // As dy
							node_y = node_c.c_dy + node_y * ratio;
						}
					}
					else
						node_x = node_c.c_dx, node_y = node_c.c_dy;
					break;   // Node While loop
				}	entry++; // Node While loop if mstime >= node_n.ms
			}
		}
		const float x = 900.f + (node_x * rotcos - node_y * rotsin) * xscale + xdelta,
					y = 540.f + (node_x * rotsin + node_y * rotcos) * yscale + ydelta;
			  float w = mstime - nodes[0].ms;
					w = w>=151 ? 1.0f : w * 0.006622516556291f;   // As  ms_passed / 151.0f
		UseWgo(L, wgo_used, x, y, wish_c.of_layer2 ? 0.0f : 0.01f, w);


		/* WishChilds */ {
			auto& wcs = wish_c.wishchilds;
			const uint16_t wc_count = wcs.size();

			if(wc_count &&   // Judge if there is any WishChild to render
				wcs[0].dt-current_dt<=wish_c.max_visitable_distance && current_dt<wcs[wc_count-1].dt) {

				// Entry Regression
				auto& entry = wish_c.wc_entry;
				while( entry > -1  &&  current_dt < wcs[entry].dt )
					entry--;

				// Process Childs
				for(auto ci = (uint16_t)(entry+1); ci < wc_count; ci++) {
					auto& child_c = wcs[ci];
					if(current_dt >= child_c.dt) {									// "Too Late" Judging
						entry = ci;
						continue;   // Child For Loop
					}
					const double radius = child_c.dt - current_dt;					// "Too Early" Judging
					if(radius > wish_c.max_visitable_distance)
						break;   // Child For Loop

					/* Angle Iteration */
					double angle = 90.0; {
						const auto& anodes = child_c.anodes;
						const int16_t anodes_tail = anodes.size() - 1;
						if(anodes_tail < 0)
							continue;   // Child For Loop

						if(current_dt >= anodes[anodes_tail].dt)   // Tail Judging
							angle = anodes[anodes_tail].degree;
						else if(current_dt <= anodes[0].dt)   // Head Judging
							angle = anodes[0].degree;
						else {   // Iteration when Needed
							// Regression
							auto& a_entry = child_c.a_entry;
							while( a_entry > 0  &&  current_dt < anodes[a_entry].dt )
								a_entry--;

							// Angle Acquisition
							while(a_entry < anodes_tail) {
								const auto& anode_c = anodes[ a_entry ];
								const auto& anode_n = anodes[ a_entry + 1 ];
								if(current_dt < anode_n.dt) {
									if(anode_c.easetype) {   // Do Easing.
										float ratio = (current_dt - anode_c.dt) * anode_c.rcp;
										switch(anode_c.easetype) {
											case LINEAR:
												break;
											case ASIN:
												ratio = ESIN[ (uint16_t)(1000 * ratio) ];
												break;
											case ACOS:
												ratio = ECOS[ (uint16_t)(1000 * ratio) ];
												break;
											case INQUAD:
												ratio *= ratio;
												break;
											case OUTQUAD:
												ratio = 1.0f - ratio;
												ratio = (1.0f - ratio * ratio);
											default:;   // Break omitted
										}
										angle = anode_n.degree - anode_c.degree;   // As dx
										angle = anode_c.degree + angle * ratio;
									}
									else
										angle = anode_c.degree;
									break;   // Angle While loop
								}
									entry++; // Angle While loop if current_dt >= anode_n.dt
							}
						}
					}

					GetSINCOS(angle);
					const float child_x = node_x + radius * 112.5f * COS,
								child_y = node_y + radius * 112.5f * SIN;
					const float cx = 900.f + (child_x * rotcos - child_y * rotsin) * xscale + xdelta,
								cy = 540.f + (child_x * rotsin + child_y * rotcos) * yscale + ydelta;

					// As (wish_c.mvb - radius) / (wish_c.mvb * 0.151f)
						  float cw = 3390 * (wish_c.max_visitable_distance - radius) *
									 RCP[ (uint16_t)(wish_c.max_visitable_distance * 511.89f) ];
								cw = cw>1.0f ? 1.0f : cw;
					UseWgo(L, wgo_used, cx, cy, wish_c.of_layer2 ? 0.02f : 0.03f, cw);
				}
			}
		}


		/* EchoChilds */ {
			auto& ecs = wish_c.echochilds;
			const uint16_t ec_count = ecs.size();

			if(ec_count &&   // Judge if there is any EchoChild to render
				ecs[0].dt-current_dt<=wish_c.max_visitable_distance && current_dt<ecs[ec_count-1].dt) {

				// Entry Regression
				auto& entry = wish_c.ec_dt_entry;
				while( entry > -1  &&  current_dt < ecs[entry].dt )
					entry--;

				// Process Childs
				for(auto ci = (uint16_t)(entry+1); ci < ec_count; ci++) {
					auto& child_c = ecs[ci];
					if(current_dt >= child_c.dt) {									// "Too Late" Judging
						entry = ci;
						continue;   // Child For Loop
					}
					const double radius = child_c.dt - current_dt;					// "Too Early" Judging
					if(radius > wish_c.max_visitable_distance)
						break;   // Child For Loop

					/* Angle Iteration */
					double angle = 90.0; {
						const auto& anodes = child_c.anodes;
						const int16_t anodes_tail = anodes.size() - 1;
						if(anodes_tail < 0)
							continue;   // Child For Loop

						if(current_dt >= anodes[anodes_tail].dt)   // Tail Judging
							angle = anodes[anodes_tail].degree;
						else if(current_dt <= anodes[0].dt)   // Head Judging
							angle = anodes[0].degree;
						else {   // Iteration when Needed
							// Regression
							auto& a_entry = child_c.a_entry;
							while( a_entry > 0  &&  current_dt < anodes[a_entry].dt )
								a_entry--;

							// Angle Acquisition
							while(a_entry < anodes_tail) {
								const auto& anode_c = anodes[ a_entry ];
								const auto& anode_n = anodes[ a_entry + 1 ];
								if(current_dt < anode_n.dt) {
									if(anode_c.easetype) {   // Do Easing.
										float ratio = (current_dt - anode_c.dt) * anode_c.rcp;
										switch(anode_c.easetype) {
											case LINEAR:
												break;
											case ASIN:
												ratio = ESIN[ (uint16_t)(1000 * ratio) ];
												break;
											case ACOS:
												ratio = ECOS[ (uint16_t)(1000 * ratio) ];
												break;
											case INQUAD:
												ratio *= ratio;
												break;
											case OUTQUAD:
												ratio = 1.0f - ratio;
												ratio = (1.0f - ratio * ratio);
											default:;   // Break omitted
										}
										angle = anode_n.degree - anode_c.degree;   // As dx
										angle = anode_c.degree + angle * ratio;
									}
									else
										angle = anode_c.degree;
									break;   // Angle While loop
								}
									entry++; // Angle While loop if current_dt >= anode_n.dt
							}
						}
					}

					GetSINCOS(angle);
					const float child_x = node_x + radius * 112.5f * COS,
								child_y = node_y + radius * 112.5f * SIN;
					const float cx = 900.f + (child_x * rotcos - child_y * rotsin) * xscale + xdelta,
								cy = 540.f + (child_x * rotsin + child_y * rotcos) * yscale + ydelta;

					// As (wish_c.mvb - radius) / (wish_c.mvb * 0.151f)
						  float cw = 3390 * (wish_c.max_visitable_distance - radius) *
									 RCP[ (uint16_t)(wish_c.max_visitable_distance * 511.89f) ];
								cw = cw>1.0f ? 1.0f : cw;

					// Anim GOs will be applied on mstime basis later.
					UseEgo(L, ego_used, (mstime-child_c.ms), child_c.status,
						 cx, cy, wish_c.of_layer2 ? 0.04f : 0.05f, cw);
				}
			}
		}
	}


	/* Hint, Echo, Anim of EchoChild */
	for( uint16_t current_group = init_group; current_group < beyond_group; current_group++ ) {
		for( const auto hint_cid : Arf->index[current_group].hidx ) {
			/* Time & Jump */
			const auto& hint_c = Arf->hint[hint_cid];
			const auto dt = (int32_t)mstime - (int32_t)hint_c.ms;
			if(dt < -510  ||  dt > 982)		break;   // when dt>982, all objs in this group meet dt>470
			if(dt > 470)					continue;

			/* Other Params */
			const float x = 900.f + (hint_c.c_dx * rotcos - hint_c.c_dy * rotsin) * xscale + xdelta;
			const float y = 540.f + (hint_c.c_dx * rotsin + hint_c.c_dy * rotcos) * yscale + ydelta;
			const auto  j = (int32_t)mstime - (int32_t)hint_c.judged_ms;
			UseHgo(L, hgo_used, ago_used, dt, j, hint_c.status, hint_c.elstatus, x, y);
		}

		for( const auto echo_cid : Arf->index[current_group].eidx ) {
			/* Time & Jump */
			const auto& echo_c = Arf->echo[echo_cid];
			const auto  dt = (int32_t)mstime - (int32_t)echo_c.ms;
			if(dt < -510  ||  dt > 982)		break;   // when dt>982, all objs in this group meet dt>470
			if(dt > 470)					continue;

			/* Other Params */
				  float w = (dt>=-359) ? 1.0f : (-dt-359)*0.006622516556291f;   // As  ms_passed / 151
			const float x = 900.f + (echo_c.c_dx * rotcos - echo_c.c_dy * rotsin) * xscale + xdelta;
			const float y = 540.f + (echo_c.c_dx * rotsin + echo_c.c_dy * rotcos) * yscale + ydelta;
			const auto  j = (int32_t)mstime - (int32_t)echo_c.judged_ms;
			UseAgoForEgo(L, ago_used, dt, j, echo_c.status, x, y);
			UseEgo(L, ego_used, dt, echo_c.status, x, y, 0.06f, w);
		}

		for(const auto  current_wish_id : Arf->index[current_group].widx) {
				  auto& wish_c = Arf->wish[current_wish_id];
				  auto& current_ecs = wish_c.echochilds_ms_order;
			const auto  ec_count = current_ecs.size();

			if(!ec_count)																		continue;
			if(mstime-current_ecs[0]->ms < -100  ||  mstime-current_ecs[ec_count-1]->ms > 470)	continue;

			auto& entry = Arf->wish[current_wish_id].ec_ms_entry;
			while(entry > -1  &&  mstime-current_ecs[entry]->ms < -100)							entry--;

			const float x = 900.f + (wish_c.current_cdx*rotcos - wish_c.current_cdy*rotsin) * xscale + xdelta;
			const float y = 540.f + (wish_c.current_cdx*rotsin + wish_c.current_cdy*rotcos) * yscale + ydelta;
			for(auto ei = (uint16_t)(entry+1); ei < ec_count; ei++) {
				EchoChild& current_echo = *current_ecs[ei];
				const int32_t dt  = mstime - current_echo.ms;
				const int32_t jdt = mstime - current_echo.judged_ms;
				UseAgoForEgo(L, ago_used, dt, jdt, current_echo.status, x, y);
			}
		}
	}

	/* Do Returns */
	lua_pushnumber(L, sweep_result.hit);		lua_pushnumber(L, sweep_result.late);
	lua_pushnumber(L, wgo_used);				lua_pushnumber(L, hgo_used);
	lua_pushnumber(L, ego_used);				lua_pushnumber(L, ago_used);
	return last_wgo.clear(), 6;
}