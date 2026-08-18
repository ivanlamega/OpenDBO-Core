#ifndef __SERVER_HLS_SLOT_MACHINE_MANAGER__
#define __SERVER_HLS_SLOT_MACHINE_MANAGER__


#include "NtlSingleton.h"
#include "HlsSlotMachineItemTable.h"
#include "NtlSharedDef.h"
#include "NtlMutex.h"
#include <list>

class CPlayer;
struct sHLS_ITEM_TBLDAT;
struct sHLS_SLOT_MACHINE_TBLDAT;

struct sHLS_SLOT_ITEM
{
	WORD							wCountLeft;
	BYTE							byRank; //top 10
	float							fPercent;
	sHLS_ITEM_TBLDAT*				pHlsItem;
	sHLS_SLOT_MACHINE_ITEM_TBLDAT*	pSlotItem;
};

struct sHLS_SLOT_WINNER_INFO
{
	WCHAR			wszPlayer[NTL_MAX_SIZE_CHAR_NAME + 1];
	WORD			wWinCount;
	DBOTIME			nExtractTime;
	QWORD			winnerIndex;
	UINT			byRank;
};

struct sSLOT_MACHINE
{
	sHLS_SLOT_MACHINE_TBLDAT*	pTbldat;
	WORD						wMaxCapsule;
	WORD						wCurrentCapsule;
};

class CHlsSlotMachine : public CNtlSingleton<CHlsSlotMachine>
{

public:

	CHlsSlotMachine();
	virtual ~CHlsSlotMachine();

public:

	void							Init();

	void							GetSlotItems(TBLIDX slotIdx, std::vector<sHLS_SLOT_ITEM*>* pVec);

	void							SetWaguItemCount(TBLIDX slotIdx, BYTE Count, TBLIDX tblidx);

	void							AddWinner(TBLIDX slotId, TBLIDX itemTblidx, CPlayer* pPlayer);

	void							GetWinnerInfo(TBLIDX wSlot, CPlayer * pPlayer);

	void							LoadSlotMachines(CPlayer* pPlayer, BYTE byType);

	sSLOT_MACHINE*					GetSlotMachine(TBLIDX tblidx);

	void							DebugDumpSlotMachines(TBLIDX requestedIdx);

	// this singleton's machine/item state is shared across every IOCP worker
	// thread; callers that need to make several calls appear atomic (e.g. a
	// GetSlotMachine + GetSlotItems + ... + Init sequence spanning a single
	// extract request) must hold this for the whole sequence, since Init()
	// deletes and rebuilds the very objects other threads may be mid-read on
	CNtlMutex*						GetMutex() { return &m_mutex; }

private:

	CNtlMutex						m_mutex;

	typedef std::multimap<TBLIDX, sHLS_SLOT_ITEM*> SLOTMACHINEGROUP;
	typedef SLOTMACHINEGROUP::iterator SLOTMACHINEGROUP_IT;
	typedef SLOTMACHINEGROUP::value_type SLOTMACHINEGROUP_VAL;

	SLOTMACHINEGROUP							m_slotMachineGroup;

	std::map<TBLIDX, QWORD>							m_mapWinnerIndex;
	std::map<TBLIDX, std::list<sHLS_SLOT_WINNER_INFO*>>	m_mapSlotWinnerInfo;

	std::map<TBLIDX, sSLOT_MACHINE*>			m_mapSlotMachine;
};

#define GetHlsSlotMachine()			CHlsSlotMachine::GetInstance()
#define g_pHlsSlotMachine			GetHlsSlotMachine()

#endif