/* Arf3 Common Header */
#pragma once
#define Arf3_API  int Arf3_APIs::
#define Arf3_JUD  JudgeResult Arf3_APIs::

#include <dmsdk/sdk.h>												/* Defold SDK */
#include <dmsdk/dlib/time.h>
#include <dmsdk/dlib/vmath.h>
#include <dmsdk/dlib/buffer.h>
#include <dmsdk/script/script.h>
#include <dmsdk/gameobject/gameobject.h>

#include <unordered_map>											/* Data Structure */
#include <bitsery/bitsery.h>
#include <bitsery/traits/adapter_buffer.h>
#include <bitsery/traits/container_vector.h>						// std::vector included
#include <bitsery/traits/compact_value.h>
#include <bitsery/traits/value_range.h>
#include <ease_constants.h>											/* Constants */


// Arf3 Typedefs & Globals
namespace Arf3 {

	// Constants
	enum TableIndex		{		WGO = 2, HGO, EGO, AGOL, AGOR, WTINT, HTINT, ETINT, ATINT	};
	enum EaseType		{	 STATIC = 0, LINEAR, LCIRC, RCIRC, INQUAD, OUTQUAD,
							   ASIN = 2, ACOS												};
	enum Hint			{ NONJUDGED = 0, NONJUDGED_LIT, JUDGED, JUDGED_LIT, LOST, AUTO,
							 COMMON = 0, EARLY, LATE										};

	// Elemental
	struct DeltaNode {
		double		base_dt = 0.0;
		uint32_t	init_ms = 0;
		float		ratio = 0.0f;									// BPM * Scale / 15000 -> {-32, 32, 1/131072}
	};
	struct AngleNode {
		double		dt = 0.0;
		float		rcp = 0.0f;										// 1.0f / (next_dt - dt)
		int16_t		degree = 0;
		uint8_t		easetype = LINEAR;								// It sucks that sizeof(an_enum_type)==4
	};
	struct PosNode {
		uint32_t	ms = 0;
		float		c_dx = 0.0f,  c_dy = 0.0f;						// {-152, 152, 1/128}; {-76, 76, 1/128}
		/*--------------------------------*/						// Some Partial-Ease calculation results
		float		ci = 0.0f, x_fci = 0.0f, y_fci = 0.0f;			//   are cached here to improve the ease
		float		ce = 0.0f, x_fce = 0.0f, y_fce = 0.0f;			//   performance.
		float		x_dnm = 0.0f, y_dnm = 0.0f;
		uint8_t		easetype = LINEAR;
	};
	struct Object {
		uint32_t	ms = 0;
		float		c_dx = 0.0f,  c_dy = 0.0f;
		/*--------------------------------*/
		uint32_t	judged_ms = 0;
		uint8_t		elstatus = COMMON;
		uint8_t		status = NONJUDGED;
	};

	// Combinative
	struct Index	 { std::vector<uint16_t> widx, hidx, eidx;	};			// Wish/Hint/Echo: Max 65535
	struct WishChild {
		std::vector<AngleNode>		anodes;									// 8*4 bytes
		double						dt = 0.0;
		uint32_t					a_entry = 0;							// AngleNode: Max 2^32-1
	};
	struct EchoChild {
		std::vector<AngleNode>		anodes;									// 8*4 bytes
		double						dt = 0.0;
		uint32_t					a_entry = 0;							// AngleNode: Max 2^32-1
		uint32_t					ms = 0, judged_ms = 0;
		uint8_t						status = NONJUDGED;
	};
	struct Wish {
		std::vector<PosNode>		nodes;									// Node: Max 65535
		std::vector<WishChild>		wishchilds;								// WishChild: Max 32767
		std::vector<EchoChild>		echochilds;								// EchoChild: Max 32767
		std::vector<EchoChild*>		echochilds_ms_order;
		float						max_visitable_distance = 7.0f;			// {0, 16, 1/128}
		/*--------------------------------*/
		float						current_cdx = 0, current_cdy = 0;		// Caches to speed up the judging
		int16_t						wc_entry = -1, ec_dt_entry = -1, ec_ms_entry = -1;
		uint16_t					n_entry = 0;
		/*--------------------------------*/
		bool						of_layer2 = false;						// We put this member at the end
	};																		// to control the Struct size.
	struct Fumen {
		std::vector<Index>			index;
		std::vector<DeltaNode>		d1, d2;
		std::vector<Object>			hint, echo;
		std::vector<Wish>			wish;									// Max 65535 Wish/Hint+Echo
		uint32_t					before = 0;								// in a 512ms Period
		uint16_t					wgo_required = 255, ego_required = 255, hgo_required = 0,
									object_count = 0, special_hint = 0;
	};

	// Misc
	struct  ab			{ float a,b; };										// 2-float Structure
	struct  JudgeResult	{ uint16_t hit,early,late; bool sh_judged; };		// 4-uint Structure
	extern	uint64_t	mstime, systime;									// Shared, OK to Clear
	extern	int8_t		mindt, maxdt, idelta;								// User Settings, Cannot Clear

	extern	uint8_t		judge_range;													/* Runtime Params */
	extern	float		object_size_x, object_size_y;
	extern	float		xscale, yscale, xdelta, ydelta, rotsin, rotcos, SIN, COS;
	extern	bool		daymode;

	extern	uint64_t												dt_p1, dt_p2;		/* Internals */
	extern	std::unordered_map<uint32_t, uint16_t>					last_wgo;
	extern	std::vector<ab>											blocked;

	// Utils
	inline uint16_t mod_degree(uint64_t deg) {
		do {   // Actually while(xxx)···
			if(deg > 7200)  deg -= (deg > 14400) ? 14400 : 7200 ;
			else			deg -= 3600;
		}
		while(deg > 3600);
		return deg;
	}
	inline void GetSINCOS(const double degree) {
		if( degree >= 0 ) {
			auto deg = (uint64_t)(degree*10.0);				deg = (deg>3600) ? mod_degree(deg) : deg ;
			if(deg > 1800) {
				if(deg > 2700)	{ SIN = -DSIN[3600-deg];	COS = DCOS[3600-deg];  }
				else 			{ SIN = -DSIN[deg-1800];	COS = -DCOS[deg-1800]; }
			}
			else {
				if(deg > 900)	{ SIN = DSIN[1800-deg];		COS = -DCOS[1800-deg]; }
				else			{ SIN = DSIN[deg]; 			COS = DCOS[deg];	   }
			}
		}
		else {   // sin(-x) = -sin(x), cos(-x) = cos(x)
			auto deg = (uint64_t)(-degree*10.0);			deg = (deg>3600) ? mod_degree(deg) : deg ;
			if(deg > 1800) {
				if(deg > 2700)	{ SIN = DSIN[3600-deg];		COS = DCOS[3600-deg];  }
				else 			{ SIN = DSIN[deg-1800];		COS = -DCOS[deg-1800]; }
			}
			else {
				if(deg > 900)	{ SIN = -DSIN[1800-deg];	COS = -DCOS[1800-deg]; }
				else			{ SIN = -DSIN[deg]; 		COS = DCOS[deg];	   }
			}
		}
	}
	inline double QueryDt(const uint32_t mstime, const std::vector<DeltaNode>& dt, uint64_t& dtp) {
		const uint64_t	dt_tail	= dt.size() - 1;
		const auto&		node_l	= dt[dt_tail];
		if( mstime >= node_l.init_ms )
			return node_l.base_dt + (mstime - node_l.init_ms) * node_l.ratio;

		while( dtp > 0  &&  mstime < dt[dtp].init_ms )  dtp--;
		while( dtp < dt_tail ) {
			const auto& node_c = dt[dtp];
			const auto& node_n = dt[dtp+1];
			if( mstime < node_n.init_ms )
				return node_c.base_dt + (mstime - node_c.init_ms) * node_c.ratio;
			dtp++;
		}
		return 0;
	}
}
extern Arf3::Fumen* Arf;   // new -> delete -> set nullptr


// Bitsery Settings
namespace bitsery {
	static constexpr auto P = 1.0f/128, HP = 1.0f/131072;								// Precisions
	static constexpr auto ES = ext::ValueRange<float> {0.0f, 1.0f, 1.0f/1024};			// Ease Precalculate
	static constexpr auto PX = ext::ValueRange<float> {-16200.0f, 16200.0f, P};			// Pos X
	static constexpr auto PY = ext::ValueRange<float> {-8100.0f, 8100.0f, P};			// Pos Y
	static constexpr auto MV = ext::ValueRange<float> {0.0f, 16.0f, P};					// Max Visible Distance
	static constexpr auto DR = ext::ValueRange<float> {-32.0f, 32.0f, HP};				// DeltaTime Ratio
	static constexpr auto DT = ext::ValueRange<double> {0.0, 131072.0, (double)HP};		// DeltaTime
	static constexpr auto CV = ext::CompactValueAsObject {};

	template<typename S> void serialize(S& s, Arf3::DeltaNode& dn) {
		s.enableBitPacking(
			[&dn](typename S::BPEnabledType& builder) {
				builder.ext(dn.init_ms, CV);	builder.ext(dn.ratio, DR);
			}
		);
	}

	template<typename S> void serialize(S& s, Arf3::AngleNode& an) {   // Runtime Params won't be Serialized here
		s.enableBitPacking(
			[&an](typename S::BPEnabledType& builder) {
				builder.ext(an.dt, DT);			builder.ext(an.degree, CV);		builder.value1b(an.easetype);
			}
		);
	}

	template<typename S> void serialize(S& s, Arf3::PosNode& pn) {
		s.enableBitPacking(
			[&pn](typename S::BPEnabledType& builder) {
				builder.ext(pn.ms, CV);			builder.ext(pn.c_dx, PX);		builder.ext(pn.c_dy, PY);
				builder.ext(pn.ci, ES);			builder.ext(pn.ce, ES);			builder.value1b(pn.easetype);
			}
		);
	}

	template<typename S> void serialize(S& s, Arf3::Object& o) {   // Runtime Params will be Serialized here
		s.enableBitPacking(
			[&o](typename S::BPEnabledType& builder) {
				builder.ext(o.ms, CV);				builder.ext(o.c_dx, PX);		builder.ext(o.c_dy, PY);
				builder.ext(o.judged_ms, CV);		builder.value1b(o.elstatus);	builder.value1b(o.status);
			}
		);
	}

	template<typename S> void serialize(S& s, Arf3::Index& idx) {
		s.enableBitPacking(
			[&idx](typename S::BPEnabledType& builder) {
				builder.container2b(idx.widx, 65535);
				builder.container2b(idx.hidx, 65535);
				builder.container2b(idx.eidx, 65535);
			}
		);
	}

	template<typename S> void serialize(S& s, Arf3::WishChild& wc) {   // Runtime Params won't be Serialized here
		s.enableBitPacking(
			[&wc](typename S::BPEnabledType& builder) {
				builder.container(wc.anodes, 4294967295);			builder.ext(wc.dt, DT);
			}
		);
	}

	template<typename S> void serialize(S& s, Arf3::EchoChild& ec) {   // Runtime Params will be Serialized here except a_entry
		s.enableBitPacking(
			[&ec](typename S::BPEnabledType& builder) {
				builder.container(ec.anodes, 4294967295);			builder.ext(ec.dt, DT);
				builder.ext(ec.ms, CV);								builder.ext(ec.judged_ms, CV);
				builder.value1b(ec.status);
			}
		);
	}

	template<typename S> void serialize(S& s, Arf3::Wish& w) {
		s.enableBitPacking(
			[&w](typename S::BPEnabledType& builder) {   // Runtime Params won't be Serialized here
				builder.container(w.nodes, 65535);
				builder.container(w.wishchilds, 32767);				builder.container(w.echochilds, 32767);
				builder.ext(w.max_visitable_distance, MV);			builder.boolValue(w.of_layer2);
			}
		);
	}

	template<typename S> void serialize(S& s, Arf3::Fumen& fm) {
		s.enableBitPacking(
			[&fm](typename S::BPEnabledType& builder) {
				builder.container(fm.index, 8388608);
				builder.container(fm.d1, 0xffffffffffffffff);		builder.container(fm.d2, 0xffffffffffffffff);
				builder.container(fm.hint, 65535);
				builder.container(fm.echo, 65535);
				builder.container(fm.wish, 65535);
				builder.ext(fm.before, CV);							builder.ext(fm.wgo_required, CV);
				builder.ext(fm.ego_required, CV);					builder.ext(fm.hgo_required, CV);
				builder.ext(fm.object_count, CV);					builder.ext(fm.special_hint, CV);
			}
		);
	}
}


// APIs
namespace Arf3_APIs {
	int					InitArf3			(lua_State*);
	int					FinalArf			(lua_State*);
	int					SetDaymode			(lua_State*);
	int					SetCam				(lua_State*);
	int					UpdateArf			(lua_State*);
#if defined(AR_BUILD_VIEWER)
	int					MakeArf				(lua_State*);
	int					NewTable			(lua_State*);
#endif
#if defined(AR_BUILD_VIEWER) || defined(AR_WITH_EXPORTER)
	int					DumpArf				(lua_State*);
#endif
#if defined(AR_BUILD_VIEWER) || defined(AR_COMPATIBILITY)
	int					InitArf2			(lua_State*);
#endif
#ifndef AR_BUILD_VIEWER
	Arf3::JudgeResult	JudgeArf			(const Arf3::ab*, uint8_t, bool, bool);
	int					SetObjectSize		(lua_State*);
	int					SetJudgeRange		(lua_State*);
	int					SetIDelta			(lua_State*);
	int					JudgeArf			(lua_State*);
#endif
}
namespace Arf3 {
	struct BSCFG {
		static constexpr bitsery::EndiannessType Endianness = bitsery::DefaultConfig::Endianness;
		static constexpr bool CheckAdapterErrors = false, CheckDataErrors = false;
	};
	using Encoder = bitsery::Serializer< bitsery::OutputBufferAdapter< std::vector<uint8_t>, BSCFG > >;
	using Decode  = bitsery::Deserializer< bitsery::InputBufferAdapter<const uint8_t*, BSCFG> >;
}