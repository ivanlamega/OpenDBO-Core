#include "pch.h"
#include "TableSchema.h"

#include <vector>

namespace {

// A small generic JSON value tree -- object/array/string/number/bool/null.
// Deliberately minimal: no streaming, no error recovery beyond "parsing
// stops and we report failure." That's fine here because these files are
// short (a few hundred fields at most) and only ever hand-edited or
// produced by us.
struct SJsonValue
{
	enum EType { Null, Bool, Number, String, Array, Object } type = Null;

	bool							b = false;
	double							num = 0;
	CStringW						str;
	std::vector<SJsonValue>			arr;
	std::vector<std::pair<CStringW, SJsonValue>> obj;

	const SJsonValue* Find(const wchar_t* pszKey) const
	{
		for (const auto& kv : obj)
		{
			if (kv.first == pszKey)
			{
				return &kv.second;
			}
		}
		return nullptr;
	}
};

class CJsonParser
{
public:
	explicit CJsonParser(const wchar_t* psz) : m_p(psz) {}

	bool Parse(SJsonValue& out)
	{
		SkipWs();
		return ParseValue(out);
	}

private:
	const wchar_t* m_p;

	void SkipWs()
	{
		while (*m_p == L' ' || *m_p == L'\t' || *m_p == L'\r' || *m_p == L'\n')
		{
			++m_p;
		}
	}

	bool ParseValue(SJsonValue& out)
	{
		SkipWs();

		if (*m_p == L'{') return ParseObject(out);
		if (*m_p == L'[') return ParseArray(out);
		if (*m_p == L'"')
		{
			out.type = SJsonValue::String;
			return ParseString(out.str);
		}
		if (0 == wcsncmp(m_p, L"true", 4))
		{
			out.type = SJsonValue::Bool; out.b = true; m_p += 4; return true;
		}
		if (0 == wcsncmp(m_p, L"false", 5))
		{
			out.type = SJsonValue::Bool; out.b = false; m_p += 5; return true;
		}
		if (0 == wcsncmp(m_p, L"null", 4))
		{
			out.type = SJsonValue::Null; m_p += 4; return true;
		}

		// number
		const wchar_t* pStart = m_p;
		if (*m_p == L'-') ++m_p;
		while (iswdigit(*m_p) || *m_p == L'.' || *m_p == L'e' || *m_p == L'E' || *m_p == L'+' || *m_p == L'-')
		{
			++m_p;
		}
		if (m_p == pStart)
		{
			return false;
		}

		out.type = SJsonValue::Number;
		out.num = _wtof(CStringW(pStart, (int)(m_p - pStart)));
		return true;
	}

	bool ParseObject(SJsonValue& out)
	{
		out.type = SJsonValue::Object;
		++m_p; // '{'
		SkipWs();

		if (*m_p == L'}')
		{
			++m_p;
			return true;
		}

		for (;;)
		{
			SkipWs();
			if (*m_p != L'"')
			{
				return false;
			}

			CStringW key;
			if (!ParseString(key))
			{
				return false;
			}

			SkipWs();
			if (*m_p != L':')
			{
				return false;
			}
			++m_p;

			SJsonValue val;
			if (!ParseValue(val))
			{
				return false;
			}

			out.obj.push_back(std::make_pair(key, val));

			SkipWs();
			if (*m_p == L',')
			{
				++m_p;
				continue;
			}
			if (*m_p == L'}')
			{
				++m_p;
				break;
			}
			return false;
		}

		return true;
	}

	bool ParseArray(SJsonValue& out)
	{
		out.type = SJsonValue::Array;
		++m_p; // '['
		SkipWs();

		if (*m_p == L']')
		{
			++m_p;
			return true;
		}

		for (;;)
		{
			SJsonValue val;
			if (!ParseValue(val))
			{
				return false;
			}
			out.arr.push_back(val);

			SkipWs();
			if (*m_p == L',')
			{
				++m_p;
				continue;
			}
			if (*m_p == L']')
			{
				++m_p;
				break;
			}
			return false;
		}

		return true;
	}

	bool ParseString(CStringW& out)
	{
		if (*m_p != L'"')
		{
			return false;
		}
		++m_p;

		out.Empty();

		while (*m_p != L'"')
		{
			if (*m_p == L'\0')
			{
				return false; // unterminated string
			}

			if (*m_p == L'\\')
			{
				++m_p;
				switch (*m_p)
				{
				case L'"':  out += L'"';  break;
				case L'\\': out += L'\\'; break;
				case L'/':  out += L'/';  break;
				case L'n':  out += L'\n'; break;
				case L't':  out += L'\t'; break;
				case L'r':  out += L'\r'; break;
				case L'b':  out += L'\b'; break;
				case L'f':  out += L'\f'; break;
				case L'u':
				{
					WCHAR code = 0;
					for (int i = 0; i < 4; ++i)
					{
						++m_p;
						WCHAR c = *m_p;
						code = (WCHAR)(code << 4);
						if (c >= L'0' && c <= L'9')      code = (WCHAR)(code | (c - L'0'));
						else if (c >= L'a' && c <= L'f')  code = (WCHAR)(code | (c - L'a' + 10));
						else if (c >= L'A' && c <= L'F')  code = (WCHAR)(code | (c - L'A' + 10));
						else return false;
					}
					out += code;
					break;
				}
				default:
					return false;
				}
				++m_p;
			}
			else
			{
				out += *m_p;
				++m_p;
			}
		}

		++m_p; // closing quote
		return true;
	}
};

} // namespace


bool LoadTableSchema(const char* pszTableFileName, STableSchema& outSchema, CString& strError)
{
	// pszTableFileName is always narrow (see STableInfo::pszFileName), but
	// CString::Format's "%s" width has to match the format string's own
	// width (from _T() below) or it reads the argument as the wrong char
	// type -- going through CString's real converting constructor first
	// (instead of handing pszTableFileName to Format directly) keeps this
	// correct under both character-set builds.
	CString strFileName(pszTableFileName);

	CString strPath;
	strPath.Format(_T("./schema/%s.schema.json"), (LPCTSTR)strFileName);

	CFile file;
	if (!file.Open(strPath, CFile::modeRead | CFile::typeBinary))
	{
		strError.Format(_T("Schema file not found: %s"), (LPCTSTR)strPath);
		return false;
	}

	ULONGLONG nLen64 = file.GetLength();
	size_t nLen = (size_t)nLen64;

	std::vector<BYTE> buf(nLen + 1, 0);
	if (nLen > 0)
	{
		file.Read(buf.data(), (UINT)nLen);
	}
	file.Close();

	size_t nStart = 0;
	if (nLen >= 3 && buf[0] == 0xEF && buf[1] == 0xBB && buf[2] == 0xBF)
	{
		nStart = 3; // skip UTF-8 BOM, if present
	}

	int nWideLen = ::MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)(buf.data() + nStart), (int)(nLen - nStart), nullptr, 0);
	if (nWideLen <= 0)
	{
		strError.Format(_T("Schema file is not valid UTF-8 text: %s"), (LPCTSTR)strPath);
		return false;
	}

	CStringW strContent;
	::MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)(buf.data() + nStart), (int)(nLen - nStart), strContent.GetBuffer(nWideLen), nWideLen);
	strContent.ReleaseBuffer(nWideLen);

	SJsonValue root;
	CJsonParser parser((LPCWSTR)strContent);
	if (!parser.Parse(root) || root.type != SJsonValue::Object)
	{
		strError.Format(_T("Failed to parse schema file (invalid JSON): %s"), (LPCTSTR)strPath);
		return false;
	}

	const SJsonValue* pSheetName = root.Find(L"sheetName");
	const SJsonValue* pFields = root.Find(L"fields");

	if (!pSheetName || pSheetName->type != SJsonValue::String ||
		!pFields || pFields->type != SJsonValue::Array)
	{
		strError.Format(_T("Schema file is missing \"sheetName\" or \"fields\": %s"), (LPCTSTR)strPath);
		return false;
	}

	outSchema.strSheetName = pSheetName->str;
	outSchema.fields.clear();

	for (const SJsonValue& fieldVal : pFields->arr)
	{
		if (fieldVal.type != SJsonValue::Object)
		{
			strError.Format(_T("Schema file has a non-object entry in \"fields\": %s"), (LPCTSTR)strPath);
			return false;
		}

		const SJsonValue* pName = fieldVal.Find(L"name");
		const SJsonValue* pType = fieldVal.Find(L"type");

		if (!pName || pName->type != SJsonValue::String || pName->str.IsEmpty())
		{
			strError.Format(_T("Schema file has a field with no \"name\": %s"), (LPCTSTR)strPath);
			return false;
		}

		SFieldSchema field;
		field.strName = pName->str;
		field.strType = (pType && pType->type == SJsonValue::String) ? pType->str : CStringW();

		outSchema.fields.push_back(field);
	}

	if (outSchema.fields.empty())
	{
		strError.Format(_T("Schema file has no fields: %s"), (LPCTSTR)strPath);
		return false;
	}

	return true;
}
