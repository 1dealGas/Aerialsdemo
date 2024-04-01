// Singleton Class for Decoded Aerials Chart[Fumen] Data
#include <includes.h>
#pragma once


// Compressible Nodes
struct ArDeltaNode {
	double base = 0.0;
	float ratio = 0.0f;
	uint32_t init_ms = 0;
};

struct ArAngleNode {
	uint32_t ms = 0;
	int16_t degree = 0;
	uint8_t easetype = 0;
};

struct ArPosNode {
	float c_dx = 0.0f,  c_dy = 0.0f;
	uint32_t ms = 0;
/*--------------------------------*/
	float ci = 0.0f, x_fci = 0.0f, y_fci = 0.0f;
	float ce = 0.0f, x_fce = 0.0f, y_fce = 0.0f;
	float x_dnm = 0.0f, y_dnm = 0.0f;
	uint8_t easetype = 0;
};

struct ArHint {
	float c_dx = 0.0f,  c_dy = 0.0f;
	uint32_t ms = 0;
/*--------------------------------*/
	uint32_t judged_ms = 0;
	uint8_t elstatus = 0;   // To Utilize the Memory Alignment Padding Better
	uint8_t status = 0;
};


// Incompressible Nodes
// We call "new sth[size]" on InitArf(),
// And "delete[] sth" will be recursively called using "Arf.clear()"
struct ArWishChild {
	ArAngleNode* anodes = nullptr;
	float dt = 0.0f;

	uint8_t ac = 0;   // AngleNodes Count
	uint8_t ap = 0;   // Progress of AngleNodes Iteration

	~ArWishChild() { delete[] anodes; }
};

struct ArEchoChild {
	ArAngleNode* anodes = nullptr;
	float dt = 0.0f;
	uint32_t ms = 0;

	uint8_t ac = 0;   // AngleNodes Count
	uint8_t ap = 0;   // Progress of AngleNodes Iteration

	~ArEchoChild() { delete[] anodes; }
};

struct ArWishGroup {
	ArPosNode* nodes = nullptr;
	ArWishChild* childs = nullptr;
	ArEchoChild* echilds = nullptr;

	float mvb = 0.0f;   // Max Visible Distance

	uint16_t cc = 0;   // WishChilds Count
	int16_t cp = -1;   // Progress of WishChilds Iteration

	uint16_t ec = 0;   // EchoChilds Count
	int16_t ep = -1;   // Progress of EchoChilds Iteration

	uint8_t nc = 0;   // PosNodes Count
	uint8_t np = 0;   // Progress of PosNodes Iteration
	bool ofl2 = false;    // Instance belonging to Layer 2 or Not

	~ArWishGroup() {
		delete[] nodes;
		delete[] childs;
		delete[] echilds;
	}
};

struct ArIndex {
	uint16_t* widx = nullptr;
	uint16_t* hidx = nullptr;
	uint16_t* eidx = nullptr;

	uint16_t widx_count = 0;
	uint8_t hidx_count = 0;
	uint8_t eidx_count = 0;

	~ArIndex() {
		delete[] widx;
		delete[] hidx;
		delete[] eidx;
	}
};


// Root Type
// Scalar vars are saved internally
struct Arf {
	static ArIndex* index;
	static ArDeltaNode* d1;   // DeltaNodes in Layer 1
	static ArDeltaNode* d2;   // DeltaNodes in Layer 2
	static ArWishGroup* wish;
	static ArHint *hint, *echo;

	static uint16_t d1c, d2c, ic;
	static void clear() {
		if(d1)		{ delete[] d1;		d1 = nullptr;		d1c = 0; }
		if(d2)		{ delete[] d2;		d2 = nullptr;		d2c = 0; }
		if(index)	{ delete[] index;	index = nullptr;	ic = 0; }
		if(wish)	{ delete[] wish;	wish = nullptr; }
		if(hint)	{ delete[] hint;	hint = nullptr; }
		if(echo)	{ delete[] echo;	echo = nullptr; }
	}
};

// Link Symbols
ArIndex *Arf::index;			ArDeltaNode *Arf::d1, *Arf::d2;
ArWishGroup *Arf::wish;			ArHint *Arf::hint, *Arf::echo;
uint16_t Arf::d1c, Arf::d2c, Arf::ic;
using namespace std;