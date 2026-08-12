#pragma once

#include <vector>

// Data-driven description of which fields an XML export writes for a
// table, and in what order -- read from schema/<TableFileName>.schema.json
// at runtime. This does NOT replace the C++ code that actually reads each
// field off the game's struct (sITEM_TBLDAT, etc.) -- there's no
// reflection in those structs, so "how do I read p->szIcon_Name" still has
// to be C++. What the schema *does* let you change without recompiling is
// which fields get exported and in what order, and it makes the field
// list reviewable/diffable as data instead of buried in code.
//
// The JSON reader here is intentionally narrow: it understands exactly
// the shape below, not JSON in general. That's a deliberate scope
// reduction -- these files are only ever produced by us, so a small,
// well-understood parser is lower risk than vendoring a general-purpose
// JSON library sight-unseen into a codebase that can't be compiled here
// to verify it.
//
// Expected shape:
// {
//   "sheetName": "Table_Data_KOR",
//   "fields": [
//     { "name": "Tblidx", "type": "dword" },
//     { "name": "Validity_Able", "type": "bool" },
//     ...
//   ]
// }
//
// "type" is documentation + a light sanity check (see ValidateFieldValue
// in XmlExport.cpp) -- the actual formatting is still chosen by the C++
// extractor for that field name, not derived from this string at runtime.

struct SFieldSchema
{
	CStringW strName;
	CStringW strType;
};

struct STableSchema
{
	CStringW strSheetName;
	std::vector<SFieldSchema> fields;
};

// pszTableFileName is the same name used in STableInfo::pszFileName (e.g.
// "Table_Item_Data") -- the schema file loaded is
// "./schema/<pszTableFileName>.schema.json" relative to the working
// directory, matching the "./data/" convention CreateTableContainer
// already uses. Returns false (with strError set) if the file is
// missing, malformed, or empty.
bool LoadTableSchema(const char* pszTableFileName, STableSchema& outSchema, CString& strError);
