#pragma once

#include "TableContainer.h"

// Hand-written writer for the Access-style XML format CTable::LoadFromXml
// reads (see DboShared/NtlGameTable/Table.cpp) -- there is no writer
// anywhere in the shared table engine to reuse, only a reader. This is
// intentionally kept in the tool rather than the shared engine.
//
// IMPORTANT LIMITATION: a handful of fields on some tables have no XML
// column at all in the current reader (CTable::SetTableData), so they
// can never be restored from an XML file no matter what gets written
// here -- reloading will leave them as uninitialized memory, same as it
// already would for any XML file, including ones the engine itself might
// produce elsewhere. Exporting warns about this per table; see
// GetXmlExportCaveat().
bool	SaveSingleTableXml(CTableContainer::eTABLE eTable, const CString& strFolder);

// Returns a caveat string describing which fields (if any) will be lost
// on an XML round-trip for this table, or an empty string if none.
CString	GetXmlExportCaveat(CTableContainer::eTABLE eTable);
