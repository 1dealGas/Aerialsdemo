#include "miniaudio.h"
#include "miniaudio_libvorbis.h"
#include "miniaudio_libopus.h"


static ma_result ma_decoding_backend_init__libvorbis(void* pUserData, ma_read_proc onRead, ma_seek_proc onSeek, ma_tell_proc onTell, void* pReadSeekTellUserData, const ma_decoding_backend_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_data_source** ppBackend) {
	ma_result result;
	ma_libvorbis* pVorbis;
	(void)pUserData;

	pVorbis = (ma_libvorbis*)ma_malloc(sizeof(*pVorbis), pAllocationCallbacks);
	if (pVorbis == NULL) {
		return MA_OUT_OF_MEMORY;
	}

	result = ma_libvorbis_init(onRead, onSeek, onTell, pReadSeekTellUserData, pConfig, pAllocationCallbacks, pVorbis);
	if (result != MA_SUCCESS) {
		ma_free(pVorbis, pAllocationCallbacks);
		return result;
	}

	*ppBackend = pVorbis;
	return MA_SUCCESS;
}

static ma_result ma_decoding_backend_init_file__libvorbis(void* pUserData, const char* pFilePath, const ma_decoding_backend_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_data_source** ppBackend) {
	ma_result result;
	ma_libvorbis* pVorbis;
	(void)pUserData;

	pVorbis = (ma_libvorbis*)ma_malloc(sizeof(*pVorbis), pAllocationCallbacks);
	if (pVorbis == NULL) {
		return MA_OUT_OF_MEMORY;
	}

	result = ma_libvorbis_init_file(pFilePath, pConfig, pAllocationCallbacks, pVorbis);
	if (result != MA_SUCCESS) {
		ma_free(pVorbis, pAllocationCallbacks);
		return result;
	}

	*ppBackend = pVorbis;
	return MA_SUCCESS;
}

static void ma_decoding_backend_uninit__libvorbis(void* pUserData, ma_data_source* pBackend, const ma_allocation_callbacks* pAllocationCallbacks) {
	ma_libvorbis* pVorbis = (ma_libvorbis*)pBackend;        (void)pUserData;
	ma_libvorbis_uninit(pVorbis, pAllocationCallbacks);
	ma_free(pVorbis, pAllocationCallbacks);
}

static ma_result ma_decoding_backend_get_channel_map__libvorbis(void* pUserData, ma_data_source* pBackend, ma_channel* pChannelMap, size_t channelMapCap) {
	ma_libvorbis* pVorbis = (ma_libvorbis*)pBackend;        (void)pUserData;
	return ma_libvorbis_get_data_format(pVorbis, NULL, NULL, NULL, pChannelMap, channelMapCap);
}

static ma_decoding_backend_vtable mat_libvorbis = {
	ma_decoding_backend_init__libvorbis,
	ma_decoding_backend_init_file__libvorbis,
	NULL, /* onInitFileW() */
	NULL, /* onInitMemory() */
	ma_decoding_backend_uninit__libvorbis
};


static ma_result ma_decoding_backend_init__libopus(void* pUserData, ma_read_proc onRead, ma_seek_proc onSeek, ma_tell_proc onTell, void* pReadSeekTellUserData, const ma_decoding_backend_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_data_source** ppBackend) {
	ma_result result;
	ma_libopus* pOpus;
	(void)pUserData;

	pOpus = (ma_libopus*)ma_malloc(sizeof(*pOpus), pAllocationCallbacks);
	if (pOpus == NULL) {
		return MA_OUT_OF_MEMORY;
	}

	result = ma_libopus_init(onRead, onSeek, onTell, pReadSeekTellUserData, pConfig, pAllocationCallbacks, pOpus);
	if (result != MA_SUCCESS) {
		ma_free(pOpus, pAllocationCallbacks);
		return result;
	}

	*ppBackend = pOpus;
	return MA_SUCCESS;
}

static ma_result ma_decoding_backend_init_file__libopus(void* pUserData, const char* pFilePath, const ma_decoding_backend_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_data_source** ppBackend) {
	ma_result result;
	ma_libopus* pOpus;
	(void)pUserData;

	pOpus = (ma_libopus*)ma_malloc(sizeof(*pOpus), pAllocationCallbacks);
	if (pOpus == NULL) {
		return MA_OUT_OF_MEMORY;
	}

	result = ma_libopus_init_file(pFilePath, pConfig, pAllocationCallbacks, pOpus);
	if (result != MA_SUCCESS) {
		ma_free(pOpus, pAllocationCallbacks);
		return result;
	}

	*ppBackend = pOpus;
	return MA_SUCCESS;
}

static void ma_decoding_backend_uninit__libopus(void* pUserData, ma_data_source* pBackend, const ma_allocation_callbacks* pAllocationCallbacks) {
	ma_libopus* pOpus = (ma_libopus*)pBackend;            (void)pUserData;
	ma_libopus_uninit(pOpus, pAllocationCallbacks);
	ma_free(pOpus, pAllocationCallbacks);
}

static ma_result ma_decoding_backend_get_channel_map__libopus(void* pUserData, ma_data_source* pBackend, ma_channel* pChannelMap, size_t channelMapCap) {
	ma_libopus* pOpus = (ma_libopus*)pBackend;            (void)pUserData;
	return ma_libopus_get_data_format(pOpus, NULL, NULL, NULL, pChannelMap, channelMapCap);
}

static ma_decoding_backend_vtable mat_libopus = {
	ma_decoding_backend_init__libopus,
	ma_decoding_backend_init_file__libopus,
	NULL, /* onInitFileW() */
	NULL, /* onInitMemory() */
	ma_decoding_backend_uninit__libopus
};