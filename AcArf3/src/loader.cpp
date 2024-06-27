/* Arf3 Loader APIs */
#include <arf3.h>
#include <algorithm>
static const auto EMSRT = [](const Arf3::EchoChild* const a, const Arf3::EchoChild* const b)
						  {  return (a->ms) < (b->ms);  };

Arf3_API InitArf3(lua_State* L) {
	if(Arf) return 0;
	void* ArfBuf = nullptr;				using namespace Arf3;

	uint32_t ArfSize = 0; {
		if( dmScript::IsBuffer(L, 1) ) {
			const dmScript::LuaHBuffer *B = dmScript::CheckBuffer(L, 1);
			dmBuffer::GetBytes(B->m_Buffer, &ArfBuf, &ArfSize);
		}
		else
			ArfBuf = const_cast<char*>( luaL_checklstring(L, 1, (size_t*)&ArfSize) );
	}
	if(!ArfSize) return 0;

	Arf = new Fumen;
	Decode( (uint8_t*)ArfBuf, ArfSize ).object(*Arf);   // uint8_t*, int -> some_ref
	if( Arf->before ) {
		uint64_t dx_len = Arf->d1.size();
		for(uint64_t i=1; i<dx_len; i++) {
			const auto& node_f = Arf->d1[i-1];
				  auto& node_c = Arf->d1[i];
			node_c.base_dt = node_f.base_dt + (node_c.init_ms - node_f.init_ms) * (double)node_f.ratio;
		}

		dx_len = Arf->d2.size();
		for(uint64_t i=1; i<dx_len; i++) {
			const auto& node_f = Arf->d2[i-1];
				  auto& node_c = Arf->d2[i];
			node_c.base_dt = node_f.base_dt + (node_c.init_ms - node_f.init_ms) * (double)node_f.ratio;
		}

		const auto current_status = lua_toboolean(L, 2) ? AUTO : NONJUDGED;
		for( auto& wish: Arf->wish ) {
			// PosNodes
			for( auto& pobj : wish.nodes )
				if( pobj.easetype > LINEAR  &&  (pobj.ci != 0.0f  ||  pobj.ce != 1.0f) ) {
					const float ci = pobj.ci,  ce = pobj.ce;
					switch(pobj.easetype) {
						case LCIRC:   /*  x -> ESIN   y -> ECOS  */
							pobj.x_fci = ESIN[ (uint16_t)(1000*ci) ], pobj.y_fci = ECOS[ (uint16_t)(1000*ci) ];
							pobj.x_fce = ESIN[ (uint16_t)(1000*ce) ], pobj.y_fce = ECOS[ (uint16_t)(1000*ce) ];
							pobj.x_dnm = (float)( 1.0 / (pobj.x_fce - pobj.x_fci) );
							pobj.y_dnm = (float)( 1.0 / (pobj.y_fce - pobj.y_fci) );
							break;
						case RCIRC:   /*  x -> ECOS   y -> ESIN  */
							pobj.x_fci = ECOS[ (uint16_t)(1000*ci) ], pobj.y_fci = ESIN[ (uint16_t)(1000*ci) ];
							pobj.x_fce = ECOS[ (uint16_t)(1000*ce) ], pobj.y_fce = ESIN[ (uint16_t)(1000*ce) ];
							pobj.x_dnm = (float)( 1.0 / (pobj.x_fce - pobj.x_fci) );
							pobj.y_dnm = (float)( 1.0 / (pobj.y_fce - pobj.y_fci) );
							break;
						case INQUAD:
							pobj.x_fci = pobj.y_fci = (ci * ci);
							pobj.x_fce = pobj.y_fce = (ce * ce);
							pobj.x_dnm = pobj.y_dnm = (float)( 1.0 / (pobj.x_fce - pobj.x_fci) );
							break;
						case OUTQUAD:
							pobj.x_fci = pobj.y_fci = ( ci * (2.0f-ci) );
							pobj.x_fce = pobj.y_fce = ( ce * (2.0f-ce) );
							pobj.x_dnm = pobj.y_dnm = (float)( 1.0 / (pobj.x_fce - pobj.x_fci) );
						default:;   // break omitted
					}
				}

			// WishChilds
			for( auto& wc: wish.wishchilds ) {
				auto& anodes = wc.anodes;
				for(uint64_t ai = anodes.size()-1; ai > 0; ai--) {
					anodes[ai-1].rcp = 1.0 / (anodes[ai].dt - anodes[ai-1].dt);
				}
			}

			// EchoChilds
			wish.echochilds_ms_order.reserve( wish.echochilds.size() );
			for( auto& ec: wish.echochilds ) {
				auto& anodes = ec.anodes;
				for(uint64_t ai = anodes.size()-1; ai > 0; ai--) {
					anodes[ai-1].rcp = 1.0 / (anodes[ai].dt - anodes[ai-1].dt);
				}
				ec.status = current_status, ec.judged_ms = 0, wish.echochilds_ms_order.push_back(&ec);
			}
			std::stable_sort(wish.echochilds_ms_order.begin(), wish.echochilds_ms_order.end(), EMSRT);
		}
		for( auto& hint: Arf->hint )
			hint.status = current_status, hint.elstatus = hint.judged_ms = 0;
		for( auto& echo: Arf->echo )
			echo.status = current_status, echo.elstatus = echo.judged_ms = 0;

		lua_pushnumber(L, Arf->before);			lua_pushnumber(L, Arf->object_count);
		lua_pushnumber(L, Arf->wgo_required);	lua_pushnumber(L, Arf->hgo_required);	lua_pushnumber(L, Arf->ego_required);
		return 5;
	}
	return (delete Arf, Arf = nullptr, 0);
}

Arf3_API FinalArf(lua_State *L) {
	using namespace Arf3;

	// Reset Runtime Params
	blocked.clear();					last_wgo.clear();
	xdelta = ydelta = rotsin = 0.0f;	xscale = yscale = rotcos = 1.0f;
	mstime = systime = dt_p1 = dt_p2 = 0;
	daymode = false;

	// Reset Judge Params
	judge_range = 37;					object_size_x = object_size_y = 360.0f;
	mindt = idelta - judge_range;		mindt = (mindt < -100) ? -100 : mindt;
	maxdt = idelta + judge_range;		maxdt = (maxdt >  100) ?  100 : maxdt;

	return (delete Arf, Arf = nullptr, 0);
}


#if defined(AR_BUILD_VIEWER) || defined(AR_WITH_EXPORTER)
Arf3_API DumpArf(lua_State* L) {
	// Preparation
	if( !Arf ) return 0;				using namespace Arf3;
	if( !lua_toboolean(L, 1) ) {   // Clear Judge Params
		for( auto& wish: Arf->wish )
			for( auto& ec: wish.echochilds)
				ec.status = ec.judged_ms = 0;
		for( auto& hint: Arf->hint )
			hint.status = hint.elstatus = hint.judged_ms = 0;
		for( auto& echo: Arf->echo )
			echo.status = echo.elstatus = echo.judged_ms = 0;
	}

	// Serialization
	std::vector<uint8_t> buf;
	auto encoder = Encoder(buf);
	encoder.object(*Arf), encoder.adapter().flush();

	// Return
	const size_t bufsize = encoder.adapter().writtenBytesCount();
	if(bufsize)
		return (lua_pushlstring(L, (char*)&buf[0], bufsize), 1);
	return 0;
}
#endif


#if defined(AR_BUILD_VIEWER) || defined(AR_COMPATIBILITY)
#include <arf2.h>
const auto DTSRT = [](const Arf3::AngleNode& a, const Arf3::AngleNode& b) { return a.dt < b.dt; };

Arf3_API InitArf2(lua_State* L) {
	if(Arf) return 0;
	void* ArfBuf = nullptr;				using namespace Arf3;

	uint32_t ArfSize = 0; {
		if( dmScript::IsBuffer(L, 1) ) {
			dmScript::LuaHBuffer *B = dmScript::CheckBuffer(L, 1);
			dmBuffer::GetBytes(B -> m_Buffer, &ArfBuf, &ArfSize);
		}
		else
			ArfBuf = const_cast<char*>( luaL_checklstring(L, 1, (size_t*)&ArfSize) );
	}
	if(!ArfSize) return 0;

	auto verifier = flatbuffers::Verifier( (uint8_t*)ArfBuf, ArfSize );
	if( VerifyArf2Buffer(verifier) ) {
		const auto A	= GetArf2(ArfBuf);
				   Arf	= new Fumen;

		/* Metadata */ {
			Arf->before		  = A->before();			Arf->ego_required = 0;
			Arf->wgo_required = A->wgo_required();		Arf->hgo_required = A->hgo_required();
			Arf->object_count = A->hint()->size();		Arf->special_hint = A->special_hint();
		}

		/* DeltaNodes in Layer 1 */ {
			const auto d = A -> dts_layer1();
			const auto d1c = d -> size();   // Must be larger than 0

			Arf->d1.resize(d1c); {
				const auto	dn = d -> Get(0);
					  auto& dnobj = Arf->d1[0];
				dnobj.ratio = (dn >> 50) * 0.00001f;   // For 1st DeltaNode, init_ms and base_dt must be 0
			}

			for(uint64_t i = 1; i < d1c; i++) {   // Other DeltaNode(s)
				const auto	dn = d -> Get(i);
					  auto& dnobj = Arf->d1[i];
					  auto& former_obj = Arf->d1[i-1];

				dnobj.base_dt = (dn & 0xffffffff) * 0.00002;
				dnobj.ratio = (dn >> 50) * 0.00001f;
				dnobj.init_ms = ((dn>>32) & 0x3ffff) * 2;

				if( dnobj.base_dt < former_obj.base_dt )
					former_obj.ratio = -former_obj.ratio;
			}
		}

		/* DeltaNodes in Layer 2 */ {
			const auto d = A -> dts_layer2();
			const auto d2c = d -> size();   // Must be larger than 0

			Arf->d2.resize(d2c); {
				const auto	dn = d -> Get(0);
					  auto& dnobj = Arf->d2[0];
				dnobj.ratio = (dn >> 50) * 0.00001f;   // For 1st DeltaNode, init_ms and base_dt must be 0
			}

			for(uint64_t i = 1; i < d2c; i++) {   // Other DeltaNode(s)
				const auto	dn = d -> Get(i);
					  auto& dnobj = Arf->d2[i];
					  auto& former_obj = Arf->d2[i-1];

				dnobj.base_dt = (dn & 0xffffffff) * 0.00002;
				dnobj.ratio = (dn >> 50) * 0.00001f;
				dnobj.init_ms = ((dn>>32) & 0x3ffff) * 2;

				if( dnobj.base_dt < former_obj.base_dt )
					former_obj.ratio = -former_obj.ratio;
			}
		}

		/* Group Index */ {
			const auto idx = A -> index();
			const auto ic = idx -> size();

			Arf->index.resize(ic);
			for(uint64_t i = 0; i < ic; i++) {
				auto& idxobj = Arf->index[i];

				/* Wish Index of Current Group */ {
					const auto widxfb = idx -> Get(i) -> widx();
					const auto widx_count = widxfb -> size();
					idxobj.widx.resize(widx_count);

					for(uint64_t wi = 0; wi < widx_count; wi++)
						idxobj.widx[wi] = widxfb -> Get(wi);
				}

				/* Hint Index of Current Group */ {
					const auto hidxfb = idx -> Get(i) -> hidx();
					const auto hidx_count = hidxfb -> size();
					idxobj.hidx.resize(hidx_count);

					for(uint64_t hi = 0; hi < hidx_count; hi++)
						idxobj.hidx[hi] = hidxfb -> Get(hi);
				}
			}
		}

		/* WishGroups */ {
			const auto wish = A -> wish();
			const auto wish_size = wish -> size();

			Arf->wish.resize(wish_size);
			for(uint64_t i = 0; i < wish_size; i++) {
				const auto  w = wish -> Get(i);
					  auto& wobj = Arf->wish[i];

				const auto& current_dts = wobj.of_layer2 ? Arf->d1 : Arf->d2;
					  auto& current_dtp = wobj.of_layer2 ? dt_p1 : dt_p2;

				/* Scalars */ {
					const auto winfo = w -> info();
					wobj.max_visitable_distance = (winfo & 0x1fff) * 0.0009765625f;   // As /1024.0f
					wobj.of_layer2 = (bool)((winfo>>13) & 0b1);
				}

				/* PosNodes */ {
					const auto nodesfb = w -> nodes();
					const auto nc = nodesfb -> size();
					wobj.nodes.resize(nc);

					for(uint64_t ni = 0; ni < nc; ni++) {
						const auto	p = nodesfb -> Get(ni);
							  auto& pobj = wobj.nodes[ni];

						// node.x = ((p>>31) & 0x1fff) * 0.0078125f - 16.0f
						// node.y = ((p>>19) & 0xfff) * 0.0078125f - 8.0f
						pobj.ms = p & 0x7ffff;
						pobj.c_dx = ((p>>31) & 0x1fff) * 0.87890625f - 2700.0f;
						pobj.c_dy = ((p>>19) & 0xfff) * 0.87890625f - 1350.0f;
						pobj.easetype = ((p>>44) & 0b11) + 1;   // Old: LINEAR[0], LCIRC, RCIRC, *QUAD
																// New: STATIC, LINEAR[1], LCIRC, RCIRC,
						float ci = (p>>55) * 0.001956947162f; { //      INQUAD, OUTQUAD
							if(ci < 0.0f)		ci = 0.0f;
							else if(ci > 1.0f)	ci = 1.0f;
						}

						float ce = ((p>>46) & 0x1ff) * 0.001956947162f; {
							if(ce < 0.0f)		ce = 0.0f;
							else if(ce > 1.0f)	ce = 1.0f;
						}

						// Process Easetype
						if(ci > ce) {   // OutQuad
							pobj.ci = ce;   // Reversed
							pobj.ce = ci;   // Reversed
							pobj.easetype = OUTQUAD;

							if( ce != 0.0f  ||  ci != 1.0f ) {   // Reversed
								pobj.x_fci = pobj.y_fci = ( ce * (2.0f-ce) );   // Reversed
								pobj.x_fce = pobj.y_fce = ( ci * (2.0f-ci) );   // Reversed
								pobj.x_dnm = pobj.y_dnm = (float)( 1.0 / (pobj.x_fce - pobj.x_fci) );
							}
						}
						else if(pobj.easetype) {
							pobj.ci = ci;
							pobj.ce = ce;

							if( ci != 0.0f  ||  ce != 1.0f ) switch(pobj.easetype) {
								case LCIRC:   /*  x -> ESIN   y -> ECOS  */
									pobj.x_fci = ESIN[ (uint16_t)(1000*ci) ], pobj.y_fci = ECOS[ (uint16_t)(1000*ci) ];
									pobj.x_fce = ESIN[ (uint16_t)(1000*ce) ], pobj.y_fce = ECOS[ (uint16_t)(1000*ce) ];
									pobj.x_dnm = (float)( 1.0 / (pobj.x_fce - pobj.x_fci) );
									pobj.y_dnm = (float)( 1.0 / (pobj.y_fce - pobj.y_fci) );
									break;
								case RCIRC:   /*  x -> ECOS   y -> ESIN  */
									pobj.x_fci = ECOS[ (uint16_t)(1000*ci) ], pobj.y_fci = ESIN[ (uint16_t)(1000*ci) ];
									pobj.x_fce = ECOS[ (uint16_t)(1000*ce) ], pobj.y_fce = ESIN[ (uint16_t)(1000*ce) ];
									pobj.x_dnm = (float)( 1.0 / (pobj.x_fce - pobj.x_fci) );
									pobj.y_dnm = (float)( 1.0 / (pobj.y_fce - pobj.y_fci) );
									break;
								case INQUAD:
									pobj.x_fci = pobj.y_fci = (ci * ci);
									pobj.x_fce = pobj.y_fce = (ce * ce);
									pobj.x_dnm = pobj.y_dnm = (float)( 1.0 / (pobj.x_fce - pobj.x_fci) );
								default:;   // break omitted
							}
						}
					}
				}

				/* WishChilds */ {
					const auto childsfb = w -> childs();
					const auto cc = childsfb -> size();
					wobj.wishchilds.resize(cc);

					for(uint64_t ci = 0; ci < cc; ci++) {
						const auto  c = childsfb -> Get(ci);
							  auto& cobj = wobj.wishchilds[ci];
									cobj.dt = ( c->dt() ) * 0.00002f;

						const auto an = c -> anodes();
						const auto ac = an -> size();
						cobj.anodes.resize(ac);

						for(uint64_t ai = 0; ai < ac; ai++) {
							const auto	a = an -> Get(ai);
							auto& aobj = cobj.anodes[ai];

							aobj.dt = QueryDt( (a & 0x3ffff) * 2, current_dts, current_dtp );
							aobj.degree = (a>>20) - 1800;

							aobj.easetype = (a>>18) & 0b11;
							aobj.easetype = aobj.easetype>LINEAR ? aobj.easetype+2 : aobj.easetype;
						}

						std::stable_sort(cobj.anodes.begin(), cobj.anodes.end(), DTSRT);
						for(uint64_t ai = ac-1; ai > 0; ai--) {
							cobj.anodes[ai-1].rcp = 1.0 / (cobj.anodes[ai].dt - cobj.anodes[ai-1].dt);
						}
					}
				}
			}
		}

		/* Hints */
		const auto hint = A -> hint();
		const auto hint_size = Arf->object_count;
		const uint8_t init_status = lua_toboolean(L, 2) ? AUTO : NONJUDGED;
		Arf->hint.resize(hint_size);

		for(uint16_t i = 0; i < hint_size; i++) {
			auto  h = hint -> Get(i);
			auto& hobj = Arf->hint[i];

			// hint.x = (h & 0x1fff) * 0.0078125f - 16.0f
			// hint.y = ((h>>13) & 0xfff) * 0.0078125f - 8.0f
			hobj.ms = (h>>25) & 0x7ffff;
			hobj.c_dx = (h & 0x1fff) * 0.87890625f - 2700.0f;
			hobj.c_dy = ((h>>13) & 0xfff) * 0.87890625f - 1350.0f;
			hobj.status = init_status;
		}

		lua_pushnumber(L, Arf->before);			lua_pushnumber(L, Arf->object_count);
		lua_pushnumber(L, Arf->wgo_required);	lua_pushnumber(L, Arf->hgo_required);
		return lua_pushnumber(L, 0), (dt_p1 = dt_p2 = 0), 5;
	}
	return 0;
}
#endif