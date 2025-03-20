/* Aerials Audio System */
#pragma once
#include <dmsdk/sdk.h>
#include <miniaudio/miniaudio_all.h>
#include <unordered_map>

struct PseudoContext {
	dmConfigFile::HConfig       m_ConfigFile;
	dmResource::HFactory        m_ResourceFactory;
};
struct AcAudioSource {
	ma_resource_manager_data_source source;
	uint8_t* pBuf;
};

std::unordered_map<ma_sound*, uint8_t> AcAudioSounds;   // unordered_map with unordered_set-like behaviors
ma_resource_manager AcAudioManager;
ma_engine AcAudioEngine;


/* Lua API Preparations */
static int AcAudioSourceGcMethod(lua_State* L) {
	const auto pSource = (AcAudioSource*)lua_touserdata(L, 1);
	if( pSource->pBuf )
		ma_resource_manager_data_source_uninit(& pSource->source ), free( pSource->pBuf );
	return 0;
}

static int AcAudioUnitGcMethod(lua_State* L) {
	const auto pSound = (ma_sound*)lua_touserdata(L, 1);
	ma_sound_stop(pSound), ma_sound_uninit(pSound);
	if( AcAudioSounds.count(pSound) )
		AcAudioSounds.erase(pSound) ;
	return 0;
}

inline bool AcAudioIsUnit(lua_State* L) {					// (Unit, ···)    Initially
#ifdef ACAUDIO_NOCHECK
	return true;
#else
	if(! lua_getmetatable(L, 1) )					return false;
	const bool isUnit = ( lua_rawgeti(L, -1, 0xACA8D10), lua_toboolean(L, -1) );
	return lua_pop(L, 2), isUnit;							// (Unit, ···)    Finally
#endif
}


/* Lua API Implementations */
static int AcAudioGetDeviceName(lua_State* L) {
	/* Usage:
	 * local device_name = AcAudio.GetDeviceName()
	 */
	char deviceName[1024];
	size_t deviceNameLen = 0;
	ma_device* pDevice = ma_engine_get_device( &AcAudioEngine );
	if( ma_device_get_name(pDevice, ma_device_type_playback, deviceName, 1024, &deviceNameLen) != MA_SUCCESS )
		return lua_pushnil(L), 1;
	lua_pushlstring(L, deviceName, deviceNameLen);
	return 1;
}

static int AcAudioCreateSource(lua_State* L) {
	/* Usage:
	 * local ok, src_or_msg = AcAudio.CreateSource(path)
	 */

	// Args
	lua_pushinteger(L, 2744634527);									// Path -> hash"__script_context"
	lua_gettable(L, LUA_GLOBALSINDEX);								// Path -> context
	const auto pContext = (PseudoContext*)lua_touserdata(L, 2);
	const auto path = luaL_checkstring(L, 1);
	lua_pop(L, 2);

	// Vars
	uint32_t bufSize;
	const auto lSource = (AcAudioSource*)lua_newuserdata( L, sizeof(AcAudioSource) );

	// Load Buffer
	const auto loadResult = dmResource::GetRaw( pContext->m_ResourceFactory, path, (void**)&lSource->pBuf, &bufSize );
	if( loadResult != dmResource::RESULT_OK ) {						/* 2nd Attempt, Load from Disk */
		FILE* pFile = fopen(path, "rb");				// Open
		if( pFile == nullptr) {
			lua_pushboolean(L, false);
			lua_pushstring(L, "[!] Audio file not found");
			return 2;
		}

		fseek(pFile, 0, SEEK_END);									// Size
		bufSize = ftell(pFile);
		fseek(pFile, 0, SEEK_SET);

		lSource->pBuf = (uint8_t*)malloc(bufSize);					// Copying
		if( fread( lSource->pBuf, 1, bufSize, pFile ) != bufSize ) {
			lua_pushboolean(L, false);
			lua_pushstring(L, "[!] Failed to read file");
			lSource->pBuf = ( fclose(pFile), free( lSource->pBuf ), nullptr );
			return 2;
		}
		fclose(pFile);
	}

	// Register Buffer
	char vfsPath[128];			sprintf( vfsPath, "%llu", dmTime::GetTime() );
	ma_resource_manager_register_encoded_data( &AcAudioManager, vfsPath, lSource->pBuf, bufSize );

	// Init Resource
	const auto N = ma_resource_manager_pipeline_notifications_init();
	const auto initResult = ma_resource_manager_data_source_init(
		&AcAudioManager, vfsPath,
		MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_WAIT_INIT | MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_DECODE,
		&N, &lSource->source);
	ma_resource_manager_unregister_data( &AcAudioManager, vfsPath );

	// Failure
	if( initResult != MA_SUCCESS ) {
		lua_pushboolean(L, false);
		lua_pushstring(L, "Audio format not supported by miniaudio");
		lSource->pBuf = ( free( lSource->pBuf ), nullptr );
		return 2;
	}

	// Success
	luaL_getmetatable(L, "AcAudioSource");			// Source -> MetaTable
	lua_setmetatable(L, 1);					// Source
	lua_pushboolean(L, true);						// Source -> True
	lua_insert(L, 1);							// True -> Source
	return 2;
}

static int AcAudioCreateUnit(lua_State* L) {
	/* Usage:
	 * local unit, len = AcAudio.NewUnit( src )   -- Or nil, error_msg
	 */
	const auto lSource = (AcAudioSource*)luaL_checkudata(L, 1, "AcAudioSource");   // Src
	const auto pSound = (ma_sound*)lua_newuserdata( L, sizeof(ma_sound) );				// Src -> Unit

	// Init & Get Length
	const auto initResult = ma_sound_init_from_data_source(
		&AcAudioEngine, &lSource->source,
		MA_SOUND_FLAG_NO_PITCH | MA_SOUND_FLAG_NO_SPATIALIZATION,
		nullptr, pSound
	);
	if( initResult != MA_SUCCESS ) {
		lua_pushnil(L);
		lua_pushstring(L, "[!] Failed to Initialize the Unit Created");
		return 2;
	}
	float len = 0;   // The length getter needs to return a ma_result value
	ma_sound_get_length_in_seconds(pSound, &len);

	// Return
	lua_newtable(L);								// Src -> Unit -> MetaTable
	lua_pushcfunction(L, AcAudioUnitGcMethod);		// Src -> Unit -> MetaTable -> GcMethod
	lua_setfield(L, 3, "__gc");				// Src -> Unit -> MetaTable
	lua_insert(L, 1);							// MetaTable -> Src -> Unit
	lua_insert(L, 1);							// Unit -> MetaTable -> Src
	lua_rawseti(L, 2, 0xACA8D10);				// Unit -> MetaTable
	lua_setmetatable(L, 1);					// Unit
	lua_pushinteger( L, len * 1000.0 );			// Unit -> Len

	AcAudioSounds[pSound] = 1;
	return 2;
}

static int AcAudioPlayUnit(lua_State* L) {
	/* Usage:
	 * local ok = AcAudio.PlayUnit(unit, is_looping)
	 */
	if(! AcAudioIsUnit(L) )
		return lua_pushboolean(L, false), 1;
	const auto pSound = (ma_sound*)lua_touserdata(L, 1);

	// Added a sleep session, trying to let the play method available immediately after setting the playback time progress.
	ma_sound_set_looping( pSound, lua_toboolean(L, 2) );
	return dmTime::Sleep(1000), lua_pushboolean( L, ma_sound_start(pSound)==MA_SUCCESS ), 1;
}

static int AcAudioStopUnit(lua_State* L) {
	/* Usage:
	 * local ok = AcAudio.StopUnit(unit, rewind_to_start)
	 */
	if(! AcAudioIsUnit(L) )
		return lua_pushboolean(L, false), 1;
	const auto pSound = (ma_sound*)lua_touserdata(L, 1);

	if( ma_sound_stop(pSound) == MA_SUCCESS ) {
		if( lua_toboolean(L, 2) )
			ma_sound_seek_to_pcm_frame(pSound, 0);   // Rewind to Start
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
	 * local ok = AcAudio.SetTime(unit, ms)
	 */
	if(! AcAudioIsUnit(L) )
		return lua_pushboolean(L, false), 1;

	// Check if Playing
	const auto pSound = (ma_sound*)lua_touserdata(L, 1);
	if( ma_sound_is_playing(pSound) )
		return lua_pushboolean(L, false), 1;

	// Acquire Sound Length & Attempt Ms
	float len = 0;
	ma_sound_get_length_in_seconds(pSound, &len);		len *= 1000.0f;
	auto targetMs = (int64_t)luaL_checknumber(L, 2);
		 targetMs = (targetMs > 0) ? targetMs : 0;
		 targetMs = (targetMs < len-2.0) ? targetMs : (len-2.0) ;

	// Return the Result
	return lua_pushboolean(L, MA_SUCCESS ==
		ma_sound_seek_to_pcm_frame( pSound, (uint64_t)(targetMs * ma_engine_get_sample_rate(&AcAudioEngine) / 1000.0) )
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
	{"CreateSource", AcAudioCreateSource}, {"CreateUnit", AcAudioCreateUnit},
	{"CheckPlaying", AcAudioCheckPlaying}, {"GetDeviceName", AcAudioGetDeviceName},
	{"PlayUnit", AcAudioPlayUnit}, {"StopUnit", AcAudioStopUnit},
	{"SetTime", AcAudioSetTime}, {"GetTime", AcAudioGetTime},
	{nullptr, nullptr}
};

inline dmExtension::Result AcAudioInit(dmExtension::Params* p) {
	// Init a Pseudo Engine with Default Behaviors & Refer to the Vorbis / Opus Extension
	ma_engine PseudoEngine;
	ma_decoding_backend_vtable* Exts[] = { &mat_libopus, &mat_libvorbis };
	if( ma_engine_init( nullptr, &PseudoEngine ) != MA_SUCCESS ) {
		dmLogFatal("Failed to Init the miniaudio Engine.");
		return dmExtension::RESULT_INIT_ERROR;
	}

	// Init the Player Engine: a custom resource manager
	const auto device = ma_engine_get_device( &PseudoEngine );   // The default device info
	auto rmConfig			= ma_resource_manager_config_init();
		 rmConfig.decodedFormat						= device -> playback.format;
		 rmConfig.decodedChannels					= device -> playback.channels;
		 rmConfig.decodedSampleRate					= device -> sampleRate;
		 rmConfig.customDecodingBackendCount		= sizeof(Exts) / sizeof( Exts[0] );
		 rmConfig.ppCustomDecodingBackendVTables	= Exts;
	ma_resource_manager_init( &rmConfig, &AcAudioManager );
	ma_engine_uninit( &PseudoEngine );

	// Init the Player Engine: a custom engine config
	auto engineConfig			= ma_engine_config_init();
		 engineConfig.pResourceManager			= &AcAudioManager;
	ma_engine_init( &engineConfig, &AcAudioEngine );

	// Lua: Create API Table & tname Metable for AcAudio Source
	const auto L = p->m_L ;
	luaL_register(L, "AcAudio", AcAudioAPIs);			// AcAudioAPIs
	luaL_newmetatable(L, "AcAudioSource");				// AcAudioAPIs -> sMetatable
	lua_pushcfunction(L, AcAudioSourceGcMethod);				// AcAudioAPIs -> sMetatable -> sGcMethod
	lua_setfield(L, 2, "__gc");							// AcAudioAPIs -> sMetatable
	return lua_pop(L, 2), dmExtension::RESULT_OK;
}

inline void AcAudioOnEvent(dmExtension::Params*, const dmExtension::Event* e) {
	switch(e->m_Event) {   // You may want to check the "playing" status manually if needed.
		case dmExtension::EVENT_ID_ICONIFYAPP:
		case dmExtension::EVENT_ID_DEACTIVATEAPP:
			for( const auto it : AcAudioSounds )
				ma_sound_stop( it.first );   // Sounds won't rewind when "stopping"
		default:;   // break omitted
	}
}

inline dmExtension::Result AcAudioFinal(dmExtension::Params*) {
	return dmExtension::RESULT_OK;
}
inline dmExtension::Result AcAudioAPPInit(dmExtension::AppParams*) {
	return dmExtension::RESULT_OK;
}
inline dmExtension::Result AcAudioAPPFinal(dmExtension::AppParams*) {   // We assume this func to be called
	return ma_engine_uninit(&AcAudioEngine), dmExtension::RESULT_OK;	// With the lua_State closed.
}
DM_DECLARE_EXTENSION(AcAudio, "AcAudio", AcAudioAPPInit, AcAudioAPPFinal, AcAudioInit, nullptr, AcAudioOnEvent, AcAudioFinal)