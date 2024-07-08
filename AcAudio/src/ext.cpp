/* Aerials Audio System */
#pragma once
#include <dmsdk/sdk.h>
#include <dmsdk/dlib/time.h>
#include <dmsdk/dlib/buffer.h>
#include <dmsdk/script/script.h>
#include <miniaudio/miniaudio_all.h>
#include <dmsdk/dlib/log.h>
#include <unordered_map>

using AcAudioBufAddr = void*;
std::unordered_map<ma_sound*, uint8_t> acaudio_sounds;   // unordered_map with unordered_set-like behaviors
ma_resource_manager acaudio_manager;
ma_engine acaudio_engine;


/* Lua API Preparations & Implementations */
static int AcAudioSourceGcMethod(lua_State* L) {
	return
		ma_resource_manager_data_source_uninit(
			(ma_resource_manager_data_source*)lua_touserdata(L, 1)
		),
	0;
}

static int AcAudioUnitGcMethod(lua_State* L) {
	const auto sound_ptr = (ma_sound*)lua_touserdata(L, 1);
	ma_sound_stop(sound_ptr), ma_sound_uninit(sound_ptr);
	if( acaudio_sounds.count(sound_ptr) )
		acaudio_sounds.erase(sound_ptr) ;
	return 0;
}

inline void AcAudioMakeMetatableForUnit(lua_State* L) {		// Buffer -> AcAudioUnit    Initially
	/* The metatable of an unit should include:
	 * A. A ref of the original Defold Buffer, to prevent the inappropriate GC;  ->  Also function as a tag
	 * B. __gc metamethod.
	 */
	/*T*/	lua_insert(L, 1);							// AcAudioUnit -> Buffer
			lua_newtable(L);								// AcAudioUnit -> Buffer -> uMetatable
	/*A*/	lua_insert(L, 2);							// AcAudioUnit -> uMetatable -> Buffer
			lua_rawseti(L, 2, 0xACA8D10);				// AcAudioUnit -> uMetatable
			lua_rawgeti(L, 2, 0xACA8D10);				// AcAudioUnit -> uMetatable -> Buffer
			lua_insert(L, 1);							// Buffer -> AcAudioUnit -> uMetatable
	/*B*/	lua_pushcfunction(L, AcAudioUnitGcMethod);		// Buffer -> AcAudioUnit -> uMetatable -> uGcMethod
			lua_setfield(L, 3, "__gc");				// Buffer -> AcAudioUnit -> uMetatable
	/*M*/	lua_setmetatable(L, 2);					// Buffer -> AcAudioUnit    Finally
}

inline bool AcAudioIsUnit(lua_State* L) {					// (Unit, ···)    Initially
#ifdef ACAUDIO_NOCHECK
	return true;
#else
	if(! lua_getmetatable(L, 1) )					return false;
	const bool is_unit = ( lua_rawgeti(L, -1, 0xACA8D10), lua_toboolean(L, -1) );
	return lua_pop(L, 2), is_unit;							// (Unit, ···)    Finally
#endif
}

static int AcAudioCreateUnit(lua_State* L) {
	/* Usage:
	 * local buf = sys.load_buffer(some_path)
	 * local ok, unit, len = AcAudio.NewUnit(buf)   -- Or false, error_msg
	 */
	ma_resource_manager_data_source* source_ptr;

	// Prepare the Buffer if needed
	lua_getmetatable(L, 1);													// Buffer -> bMetatable
	if( lua_rawgeti(L, 2, 0xACA8D10), lua_isuserdata(L, 3) ) {				// Buffer -> bMetatable -> Source?
		source_ptr = (ma_resource_manager_data_source*)lua_touserdata(L, 3);
		lua_pop(L, 2);																// Buffer
	}
	else {
		lua_pop(L, 1);																// Buffer -> bMetatable
		source_ptr = (ma_resource_manager_data_source*)								// Buffer -> bMetatable -> Source
					 lua_newuserdata( L, sizeof(ma_resource_manager_data_source) );

		// Acquire Buffer
		AcAudioBufAddr addr;		uint32_t size;
		dmBuffer::GetBytes( dmScript::CheckBuffer(L,1)->m_Buffer, &addr, &size );

		// Register the Buffer into the miniaudio Resource Manager
		char vfs_path[128];			sprintf( vfs_path, "%llu", (unsigned long long)dmTime::GetTime() );
		ma_resource_manager_register_encoded_data( &acaudio_manager, vfs_path, addr, size );

		// Create Resource
		const auto N = ma_resource_manager_pipeline_notifications_init();
		const auto buf_prep_result = ma_resource_manager_data_source_init(
			&acaudio_manager, vfs_path,
			MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_WAIT_INIT | MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_DECODE,
			&N, source_ptr);
		ma_resource_manager_unregister_data( &acaudio_manager, vfs_path );

		// Process Failure
		if( buf_prep_result != MA_SUCCESS ) {
			lua_pushboolean(L, false), lua_pushstring(L, "[!] Audio format not supported by miniaudio");
			return 2;
		}

		// Set Source Metatable
		luaL_getmetatable(L, "AcAudioSource");			// Buffer -> bMetatable -> Source -> sMetatable
		lua_setmetatable(L, 3);					// Buffer -> bMetatable -> Source

		// Monkeypatch Buffer Metatble
		lua_newtable(L);								// Buffer -> bMetatable -> Source -> mMetatable
		lua_pushnumber(L, 2177270482);				// Buffer -> bMetatable -> Source -> mMetatable -> hash"buffer"
		lua_rawseti(L, 4, 2296269367);				// Buffer -> bMetatable -> Source -> mMetatable
		lua_getfield(L, 2, "__gc");				// Buffer -> bMetatable -> Source -> mMetatable -> bGcMethod
		lua_setfield(L, 4, "__gc");				// Buffer -> bMetatable -> Source -> mMetatable
		lua_insert(L, 3);							// Buffer -> bMetatable -> mMetatable -> Source
		lua_rawseti(L, 3, 0xACA8D10);				// Buffer -> bMetatable -> mMetatable
		lua_setmetatable(L, 1);					// Buffer -> bMetatable
		lua_pop(L, 1);									// Buffer
	}

	const auto sound_ptr = (ma_sound*)lua_newuserdata( L, sizeof(ma_sound) );	// Buffer -> AcAudioUnit
	const auto unit_init_result = ma_sound_init_from_data_source(
		&acaudio_engine, source_ptr,
		MA_SOUND_FLAG_NO_PITCH | MA_SOUND_FLAG_NO_SPATIALIZATION,
		nullptr, sound_ptr
	);

	// Do Returns
	if(unit_init_result != MA_SUCCESS) {
		lua_pushboolean(L, false), lua_pushstring(L, "[!] Failed to Initialize the Unit Created");
		return 2;
	}

	float len = 0;   // The length getter needs to return a ma_result value
	ma_sound_get_length_in_seconds(sound_ptr, &len);
	acaudio_sounds[sound_ptr] = 1;

	AcAudioMakeMetatableForUnit(L);						// Should be:  Buffer -> AcAudioUnit
	lua_pushboolean(L, true), lua_insert(L, 2);				// Buffer -> true -> AcAudioUnit
	lua_pushnumber( L, (uint64_t)(len * 1000.0) );					// Buffer -> true -> AcAudioUnit -> len
	return 3;
}

static int AcAudioPlayUnit(lua_State* L) {
	/* Usage:
	 * local ok = AcAudio.PlayUnit(unit, is_looping)
	 */
	if(! AcAudioIsUnit(L) )
		return lua_pushboolean(L, false), 1;
	const auto unit_handle = (ma_sound*)lua_touserdata(L, 1);

	ma_sound_set_looping( unit_handle, lua_toboolean(L, 2) );
	return lua_pushboolean( L, ma_sound_start(unit_handle)==MA_SUCCESS ), 1;
}

static int AcAudioStopUnit(lua_State* L) {
	/* Usage:
	 * local ok = AcAudio.StopUnit(unit, rewind_to_start)
	 */
	if(! AcAudioIsUnit(L) )
		return lua_pushboolean(L, false), 1;
	const auto unit_handle = (ma_sound*)lua_touserdata(L, 1);

	if( ma_sound_stop(unit_handle) == MA_SUCCESS ) {
		if( lua_toboolean(L, 2) )
			ma_sound_seek_to_pcm_frame(unit_handle, 0);   // Rewind to Start
		return lua_pushboolean(L, true), 1;
	}
	return lua_pushboolean(L, false), 1;
}

static int AcAudioCheckPlaying(lua_State* L) {
	/* Usage:
	 * local is_playing = AcAudio.CheckPlaying(unit)   -- Boolean Value, or nil when the unit is invalid
	 */
	if(! AcAudioIsUnit(L) )
		return lua_pushnil(L), 1;
	return lua_pushboolean(L, ma_sound_is_playing( (ma_sound*)lua_touserdata(L,1) )), 1;
}

static int AcAudioSetTime(lua_State* L) {
	/* Usage:
	 * local ok = AcAudio.SetTime(unit, ms)   -- Keep in mind that this is NOT A SYNC API.
	 */
	if(! AcAudioIsUnit(L) )
		return lua_pushboolean(L, false), 1;

	// Check if Playing
	const auto unit_handle = (ma_sound*)lua_touserdata(L, 1);
	if( ma_sound_is_playing(unit_handle) )
		return lua_pushboolean(L, false), 1;

	// Acquire Sound Length & Attempt Ms
	float len = 0;
	ma_sound_get_length_in_seconds(unit_handle, &len);		len *= 1000.0f;
	auto attempt_ms = (int64_t)luaL_checknumber(L, 2);
		 attempt_ms = (attempt_ms > 0) ? attempt_ms : 0;
		 attempt_ms = (attempt_ms < len-2.0) ? attempt_ms : (len-2.0) ;

	// Return the Result
	return lua_pushboolean(L, MA_SUCCESS ==
		ma_sound_seek_to_pcm_frame( unit_handle, (uint64_t)(attempt_ms * ma_engine_get_sample_rate(&acaudio_engine) / 1000.0) )
	), 1;
}

static int AcAudioGetTime(lua_State* L) {
	/* Usage:
	 * local ms_or_nil = AcAudio.GetTime(unit)
	 */
	if(! AcAudioIsUnit(L) )
		return lua_pushnil(L), 1;
	return lua_pushnumber(L, ma_sound_get_time_in_milliseconds( (ma_sound*)lua_touserdata(L,1) )), 1;
}


/* Binding Stuff */
constexpr luaL_reg AcAudioAPIs[] = {
	{"CreateUnit", AcAudioCreateUnit}, {"CheckPlaying", AcAudioCheckPlaying},
	{"PlayUnit", AcAudioPlayUnit}, {"StopUnit", AcAudioStopUnit},
	{"SetTime", AcAudioSetTime}, {"GetTime", AcAudioGetTime},
	{nullptr, nullptr}
};

inline dmExtension::Result AcAudioInit(dmExtension::Params* p) {
	// Init a Pseudo Engine with Default Behaviors & Refer to the Vorbis / Opus Extension
	ma_engine PseudoEngine;
	ma_decoding_backend_vtable* vo_binding[] = { &mat_libopus, &mat_libvorbis };
	if( ma_engine_init(nullptr, &PseudoEngine) != MA_SUCCESS ) {
		dmLogFatal("Failed to Init the miniaudio Engine.");
		return dmExtension::RESULT_INIT_ERROR;
	}

	// Init the Player Engine: a custom resource manager
	const auto device = ma_engine_get_device(&PseudoEngine);   // The default device info
	auto rm_config			= ma_resource_manager_config_init();
		 rm_config.decodedFormat					= device -> playback.format;
		 rm_config.decodedChannels					= device -> playback.channels;
		 rm_config.decodedSampleRate				= device -> sampleRate;
		 rm_config.customDecodingBackendCount		= sizeof(vo_binding) / sizeof(vo_binding[0]);
		 rm_config.ppCustomDecodingBackendVTables	= vo_binding;
	if( ma_resource_manager_init(&rm_config, &acaudio_manager) != MA_SUCCESS) {
		dmLogFatal("Failed to Init the miniaudio Engine.");
		return dmExtension::RESULT_INIT_ERROR;
	}

	// Init the Player Engine: a custom engine config
	auto engine_config			= ma_engine_config_init();
		 engine_config.pResourceManager			= &acaudio_manager;
	if( ma_engine_init(&engine_config, &acaudio_engine) != MA_SUCCESS ) {
		dmLogFatal("Failed to Init the miniaudio Engine.");
		return dmExtension::RESULT_INIT_ERROR;
	}
	ma_engine_uninit(&PseudoEngine);

	// Lua: Create API Table & tname Metable for AcAudio Source
	const auto L = p->m_L ;
	luaL_register(L, "AcAudio", AcAudioAPIs);			// AcAudioAPIs
	luaL_newmetatable(L, "AcAudioSource");				// AcAudioAPIs -> sMetatable
	lua_pushcfunction(L, AcAudioSourceGcMethod);				// AcAudioAPIs -> sMetatable -> sGcMethod
	lua_setfield(L, 2, "__gc");							// AcAudioAPIs -> sMetatable
	return lua_pop(L,2), dmExtension::RESULT_OK;
}

inline void AcAudioOnEvent(dmExtension::Params* p, const dmExtension::Event* e) {
	switch(e->m_Event) {   // You may want to check the "playing" status manually if needed.
		case dmExtension::EVENT_ID_ICONIFYAPP:
		case dmExtension::EVENT_ID_DEACTIVATEAPP:
			for( const auto it : acaudio_sounds )
				ma_sound_stop( it.first );   // Sounds won't rewind when "stopping"
		default:;   // break omitted
	}
}

inline dmExtension::Result AcAudioFinal(dmExtension::Params* p)		  { return dmExtension::RESULT_OK; }
inline dmExtension::Result AcAudioAPPInit(dmExtension::AppParams* p)  { return dmExtension::RESULT_OK; }
inline dmExtension::Result AcAudioAPPFinal(dmExtension::AppParams* p) { // We assume this func to be called
	return ma_engine_uninit(&acaudio_engine), dmExtension::RESULT_OK; } // With the lua_State closed.
DM_DECLARE_EXTENSION(AcAudio, "AcAudio", AcAudioAPPInit, AcAudioAPPFinal, AcAudioInit, nullptr, AcAudioOnEvent, AcAudioFinal)