#pragma once


#include "Table.h"
#include "TableContainer.h"

// One entry per table this tool currently knows how to display/edit.
// FileView builds its tab strip from this list, and Util.cpp uses it to
// register/load/save the matching tables in CTableContainer.
struct STableInfo
{
	CTableContainer::eTABLE	eType;
	const char*					pszFileName;	// on-disk file name, without extension
	LPCTSTR						pszDisplayName;	// tab label
	bool						bBrowsable;		// false for support tables with no per-row list (e.g. TextAll)
};

const STableInfo*	GetRegisteredTables(int& nCount);

// Display name for CTextAllTable::TABLETYPE category nCategory (see
// TextAllTable.h) -- e.g. "Action", "ChatCommand". Used to tag which
// category a row came from when TextAll's 28 sub-tables are merged into
// one combined XML export (see XmlExport.cpp's SaveTextAllXml). Returns an
// empty string if nCategory is out of range.
LPCTSTR				GetTextAllCategoryName(int nCategory);

// Tool-only sentinel nTableType values for Spawn rows. Each individual
// per-World spawn table (e.g. "spawn_npc_dungeon_001") gets its own
// CFileView nav tab -- see CFileView::RefreshTables/AddSpawnTab -- but none
// of them is kept in one CTableContainer::eTABLE slot like every other
// table: each World row names its own spawn file, and CTableContainer
// loads them into maps keyed by World tblidx (see
// CTableContainer::BeginNpcSpawnTable/BeginMobSpawnTable/BeginObjectTable).
// These sentinels route a row's display the same way a real eTABLE value
// would for CPropertiesWnd::LoadTableData. NPC and Mob spawn rows share one
// sentinel because they share one row shape (sSPAWN_TBLDAT); Object spawn
// rows (sOBJECT_TBLDAT) get their own. Never pass these into
// CreateTableContainer/SaveTableContainer/SaveSingleTableRdf/
// GetRegisteredTables.
const int SPAWN_ROW_NPC_OR_MOB = -1000;
const int SPAWN_ROW_OBJECT = -1001;

bool				CreateTableContainer(const char* pPath, CTable::eLOADING_METHOD eLoadingMethod);
bool				SaveTableContainer(const char* pPath, bool bNeedToEncrypt);

// Saves just one table's .rdf/.edf (binary) file, leaving the others on
// disk untouched. There's no XML-writer counterpart anywhere in the table
// engine (CTable only has a reader, LoadFromXml) -- see the note on
// CMainFrame::OnTableSaveXml.
bool				SaveSingleTableRdf(CTableContainer::eTABLE eTable, const char* pPath, bool bNeedToEncrypt);

void				DeleteTableContainer();

CTableContainer*	GetTableContainer();
