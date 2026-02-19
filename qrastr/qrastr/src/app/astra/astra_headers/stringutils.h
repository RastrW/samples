#pragma once
#include <list>
#include <set>
#include <algorithm>
#include <string>
#include <string_view>
#include <locale>
#include <map>
#include <array>
#include <optional>
#include <stdexcept>

#ifdef _MSC_VER
	#undef V2  //  defined in "Parser.h" and <windows.h>->"<winioctl.h>"  
	#include <windows.h>
	#include "comip.h"
	#include <stringapiset.h>
#endif

using STRINGLIST = std::list<std::string>;
#include <cstdint>

class stringutils
{
public:
	static inline void removecrlf(std::string& s)
	{
		s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
		s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
	}

	static inline std::string removecrlf(const std::string& s)
	{
		std::string ret;
		ret.reserve(s.size());
		std::copy_if(s.begin(), s.end(), std::back_inserter(ret), 
			[](const std::string::value_type& c) -> bool { return c != '\n' && c != '\r'; });
		return ret;
	}

	static inline void ltrim(std::string& s)
	{
        s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                [](unsigned char ch) noexcept { return !std::isspace(ch); }));
	}

	static inline void rtrim(std::string& s)
	{
        s.erase(std::find_if(s.rbegin(), s.rend(),
                [](unsigned char ch) noexcept { return !std::isspace(ch); }).base(), s.end());
	}

	static inline void tolower(std::string& s)
	{
        std::transform(s.begin(), s.end(), s.begin(),
                [](unsigned char c) { return static_cast<unsigned char>(std::tolower(c)); });
	}

	static inline void toupper(std::string& s)
	{
        std::transform(s.begin(), s.end(), s.begin(),
                [](unsigned char c) { return static_cast<unsigned char>(std::toupper(c)); });
	}

	static inline void trim(std::string& s) { ltrim(s);  rtrim(s); }

	static inline void shrink_asciiz(std::string& s)
	{
		if (const auto pos{ s.find('\0') }; pos != std::string::npos)
			s.erase(pos);
	}

	static inline void fix_decimal_separator(std::string& s)
	{
        std::transform(s.begin(), s.end(), s.begin(),
                [](unsigned char c) { return c == ',' ? stringutils::decimal_separator : c;  });
	}

	template<typename T>
	static std::string join(const T& container, const std::string::value_type Delimiter = ',')
	{
		std::string result;
		for (auto it = container.begin(); it != container.end(); it++)
		{
			if (it != container.begin())
				result.push_back(Delimiter);
			result.append(*it);
		}
		return result;
	}
	
	template<typename T>
	static size_t split(std::string_view str, T& result, std::string_view Delimiters = ",;")
	{
		result.clear();
		for(size_t nPos(0); nPos < str.length() ; )
		{
            if (const auto nMinDelPos{ str.find_first_of(Delimiters, nPos) };
                nMinDelPos == std::string::npos)
			{	
				stringutils::PushContainer(result, str.substr(nPos));
				break;
			}
			else
			{
				stringutils::PushContainer(result, str.substr(nPos, nMinDelPos - nPos));
				nPos = nMinDelPos + 1;
			}
		}
		return result.size();
	}
	
	static std::string utf8_encode(const std::wstring_view& wstr)
	{
#ifdef _MSC_VER
		if (wstr.empty()) return std::string();
        const auto size_needed{ WideCharToMultiByte(CP_UTF8, 0, &wstr[0],
                            static_cast<int>(wstr.size()), NULL, 0, NULL, NULL) };
		std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0],
                            static_cast<int>(wstr.size()), &strTo[0], size_needed, NULL, NULL);
		return strTo;
#else
		return std::string(); // nothing to convert on linux
#endif
	}

#ifdef _MSC_VER
	static std::string COM_decode(BSTR wstr)
	{
		if (wstr)
			return COM_decode(std::wstring_view(wstr));
		else
			return std::string();
	}
	static std::string COM_decode(const std::wstring_view wstr)
	{
		if (wstr.empty()) return std::string();
        const auto size_needed{ WideCharToMultiByte(CP_ACP, 0, &wstr[0],
                            static_cast<int>(wstr.size()), NULL, 0, NULL, NULL) };
		std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_ACP, 0, &wstr[0],
                            static_cast<int>(wstr.size()), &strTo[0], size_needed, NULL, NULL);
		return strTo;
	}
#else
	static std::string COM_decode(const std::string_view str)
	{
		return std::string(str);
	}
#endif 

#ifdef _MSC_VER
	static std::wstring COM_encode(const std::string_view str)
	{
		if (str.empty()) return std::wstring();
		const auto size_needed{ MultiByteToWideChar(CP_ACP, 0, &str[0], static_cast<int>(str.size()), NULL, 0) };
		std::wstring wstrTo(size_needed, 0);
		MultiByteToWideChar(CP_ACP, 0, &str[0], static_cast<int>(str.size()), &wstrTo[0], size_needed);
		return wstrTo;
	}
#else
	static std::string COM_encode(const std::string_view str)
	{
		return std::string(str);
	}
#endif

#ifdef _MSC_VER
#ifndef CMAKE_PLAIN
	static BSTR BSTR_encode(const std::string_view str)
	{
		return ::SysAllocString(COM_encode(str).c_str());
	}

	static _bstr_t _bstr_t_encode(const std::string_view str)
	{
		return _bstr_t(COM_encode(str).c_str());
	}

	static BSTR Utf8ToBSTR(const std::string_view str)
	{
		return ::SysAllocString(utf8_decode(str).c_str());
	}
#endif
#endif

	static std::string utf8_encode(const wchar_t *wstr)
	{
#ifdef _MSC_VER
		return stringutils::utf8_encode(std::wstring_view(wstr));
#else
		return std::string(); // nothing to convert on linux
#endif
	}
#ifndef _MSC_VER	
	static std::string utf8_encode(const char *str)
	{
		return std::string(str);
	}

	static std::string utf8_encode(const std::string& str)
	{
		return std::string(str);
	}
#endif	

	static std::string acp_decode(const std::string_view str)
	{
#ifdef _MSC_VER
		if (str.empty()) return std::string();
		const auto size_needed{ MultiByteToWideChar(CP_ACP, 0, &str[0], static_cast<int>(str.size()), NULL, 0) };
		std::wstring wstrTo(size_needed, 0);
		MultiByteToWideChar(CP_ACP, 0, &str[0], static_cast<int>(str.size()), &wstrTo[0], size_needed);
		return utf8_encode(wstrTo);
#else
		return std::string(str); // nothing to convert on linux
#endif
	}

	static std::string acp_encode(const std::string_view utf8string)
	{
#ifdef _MSC_VER
		if (utf8string.empty()) return std::string();
		auto size_needed{ MultiByteToWideChar(CP_UTF8, 0, &utf8string[0], static_cast<int>(utf8string.size()), NULL, 0) };
		std::wstring wstrTo(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, &utf8string[0], static_cast<int>(utf8string.size()), &wstrTo[0], size_needed);
		size_needed = WideCharToMultiByte(CP_ACP, 0, &wstrTo[0], static_cast<int>(wstrTo.size()), NULL, 0, NULL, NULL);
		std::string strTo(size_needed, 0);
		WideCharToMultiByte(CP_ACP, 0, &wstrTo[0], static_cast<int>(wstrTo.size()), &strTo[0], size_needed, NULL, NULL);
		return strTo;
#else
		return std::string(utf8string);
#endif

	}

#ifdef _MSC_VER		
	static std::wstring utf8_decode(const std::string_view str)
	{
		if (str.empty()) return std::wstring();
		const auto size_needed{ MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), NULL, 0) };
		std::wstring wstrTo(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), &wstrTo[0], size_needed);
		return wstrTo;
	}
#else
	// на linux функция ничего не делает и возвращает тот же std::string
	static std::string utf8_decode(const std::string_view str)
	{
		return std::string(str);
	}
#endif

	// возвращает строку из массива строк strArray,
	// соответствующую e из перечисления T или "???" если
	// e >= size(strArray)
	template <typename T, std::size_t N>
	static const char* enum_text(const T e, const char* const (&strArray)[N])
	{
		const auto nx{ static_cast<typename std::underlying_type<T>::type>(e) };
		if (nx >= 0 && nx < N)
			return strArray[nx];
		else
			return "???";
	}

	// возвращает std::stod если value не пустое, в противном случае - 0.0
	static double estod(const std::string& value)
	{
		return value.empty() || is_whitespace(value) ? 0.0 : std::stod(value);
	}


	// возвращает optional c double, если заданную строку удалось преобразовать корректно
	static std::optional<double> as_double(const std::string& value) noexcept
	{
		std::size_t pos{ 0 };
		try
		{
			const double ret{ std::stod(value, &pos) };
			if (pos < value.length())
				throw std::invalid_argument({});
			return ret;
		}
		catch (const std::invalid_argument&)
		{
			try
			{
				std::string fix{ value };
				stringutils::trim(fix);
				stringutils::fix_decimal_separator(fix);
				const double ret{ std::stod(fix, &pos) };
				if (pos < fix.length())
					throw std::exception({});
				return ret;
			}
			catch (const std::exception&) { return {}; }
		}
		catch (const std::out_of_range&) { return {}; }
		catch (const std::exception&) { return {}; }
	}

	// возвращает std::stol если value не пустое, в противном случае - 0.0
	static long estol(const std::string& value)
	{
		return value.empty() || is_whitespace(value) ? 0 : std::stol(value);
	}

	static bool is_whitespace(const std::string& value) 
	{
		return std::all_of(value.begin(), value.end(), [](std::string::value_type c){ return std::isspace(c); });
	}

#ifdef _MSC_VER		
	static std::string cp1251ToUtf8(const std::string_view cp1251string)
	{
		return IConvLite::cp2utf(cp1251string);
	}
#else
	// на linux функция ничего не делает и возвращает тот же std::string
	static std::string cp1251ToUtf8(const std::string_view cp1251string)
	{
		return std::string(cp1251string);
	}
#endif 

	//! Функция конвертирует utf-8 в windows cp-1251 
	//! вне зависимости от платформы
	static std::string Utf8ToMk(const std::string_view utf8string)
	{
		return Utf8Tocp1251_(utf8string);
	}

	//! Функция конвертирует windows cp-1251 в utf-8
	//! вне зависимости от платформы
	static std::string MkToUtf8(const std::string_view cp1251string)
	{
		return IConvLite::cp2utf(cp1251string);
	}

#ifdef _MSC_VER		
	static std::string Utf8Tocp1251(const std::string_view utf8string)
	{
		// если что-то плохо - просто свапните строки
		// return Utf8Tocp1251(utf8string);
		return Utf8Tocp1251_(utf8string);
	}
#else
	// на linux функция ничего не делает и возвращает тот же std::string
	static std::string Utf8Tocp1251(const std::string_view utf8string)
	{
		return std::string(utf8string);
	}
#endif 

	template<typename T>
	static void PushContainer(T& container, std::string_view item);
	static inline const unsigned char decimal_separator = std::use_facet< std::numpunct<char> >(std::locale()).decimal_point();

protected:

	class IConvLite
	{
	protected:
		using Letter = struct ConvLetter
		{
			char    win1251;
			int     unicode;
		};

		static constexpr std::array<int, 128> utftable =
		{
			0x82D0,0x83D0,0x9A80E2,0x93D1,0x9E80E2,0xA680E2,0xA080E2,0xA180E2,
			0xAC82E2,0xB080E2,0x89D0,0xB980E2,0x8AD0,0x8CD0,0x8BD0,0x8FD0,
			0x92D1,0x9880E2,0x9980E2,0x9C80E2,0x9D80E2,0xA280E2,0x9380E2,0x9480E2,
			0,0xA284E2,0x99D1,0xBA80E2,0x9AD1,0x9CD1,0x9BD1,0x9FD1,
			0xA0C2,0x8ED0,0x9ED1,0x88D0,0xA4C2,0x90D2,0xA6C2,0xA7C2,
			0x81D0,0xA9C2,0x84D0,0xABC2,0xACC2,0xADC2,0xAEC2,0x87D0,
			0xB0C2,0xB1C2,0x86D0,0x96D1,0x91D2,0xB5C2,0xB6C2,0xB7C2,
			0x91D1,0x9684E2,0x94D1,0xBBC2,0x98D1,0x85D0,0x95D1,0x97D1,
			0x90D0,0x91D0,0x92D0,0x93D0,0x94D0,0x95D0,0x96D0,0x97D0,
			0x98D0,0x99D0,0x9AD0,0x9BD0,0x9CD0,0x9DD0,0x9ED0,0x9FD0,
			0xA0D0,0xA1D0,0xA2D0,0xA3D0,0xA4D0,0xA5D0,0xA6D0,0xA7D0,
			0xA8D0,0xA9D0,0xAAD0,0xABD0,0xACD0,0xADD0,0xAED0,0xAFD0,
			0xB0D0,0xB1D0,0xB2D0,0xB3D0,0xB4D0,0xB5D0,0xB6D0,0xB7D0,
			0xB8D0,0xB9D0,0xBAD0,0xBBD0,0xBCD0,0xBDD0,0xBED0,0xBFD0,
			0x80D1,0x81D1,0x82D1,0x83D1,0x84D1,0x85D1,0x86D1,0x87D1,
			0x88D1,0x89D1,0x8AD1,0x8BD1,0x8CD1,0x8DD1,0x8ED1,0x8FD1
		};

		static const char* cp2utf1(char* out, const char* in)
		{
			const char* RetOut{ out };
			while (*in)
				if (*in & 0x80) {
					int v = utftable[(int)(0x7f & *in++)];
					if (!v)
						continue;
					*out++ = (char)v;
					*out++ = (char)(v >> 8);
					if (v >>= 16)
						*out++ = (char)v;
				}
				else
					*out++ = *in++;
			*out = '\x0';
			return RetOut;
		}

		static bool convert_utf8_to_windows1251(std::string_view utf8string, char* windows1251)
		{
			long j{ 0 };
			bool Ok{ true };
			for (long i = 0; i < static_cast<long>(utf8string.size()); ++i, ++j)
			{
				char prefix{ utf8string[i] };
				if ((prefix & 0x80) == 0)
					windows1251[j] = prefix;
				else
				{
					if ((~prefix) & 0x20 && i + 1 < static_cast<long>(utf8string.size()))
					{
						char suffix{ utf8string[i + 1] };
						int first5bit{ prefix & 0x1F };
						first5bit <<= 6;
						const int sec6bit{ suffix & 0x3F };
						const int unicode_char{ first5bit + sec6bit };

						if (unicode_char >= 0x410 && unicode_char <= 0x44F)
							windows1251[j] = (char)(unicode_char - 0x350);
						else if (unicode_char >= 0x80 && unicode_char <= 0xFF)
							windows1251[j] = (char)(unicode_char);
						else if (unicode_char >= 0x402 && unicode_char <= 0x403)
							windows1251[j] = (char)(unicode_char - 0x382);
						else
							if (const auto itutf{ g_letters.find(unicode_char) }; itutf != g_letters.end())
								windows1251[j] = itutf->second;
							else
							{
								windows1251[j] = '\x7f';
								Ok = false;
							}
					}
					else
					{
						windows1251[j] = '\x7f';
						Ok = false;
					}
					++i;
				}
			}
			return Ok;
		}

		public:

			static const inline std::map<int, const char> g_letters =
			{
				{0x201A, char(0x82)}, // SINGLE LOW-9 QUOTATION MARK
				{0x0453, char(0x83)}, // CYRILLIC SMALL LETTER GJE
				{0x201E, char(0x84)}, // DOUBLE LOW-9 QUOTATION MARK
				{0x2026, char(0x85)}, // HORIZONTAL ELLIPSIS
				{0x2020, char(0x86)}, // DAGGER
				{0x2021, char(0x87)}, // DOUBLE DAGGER
				{0x20AC, char(0x88)}, // EURO SIGN
				{0x2030, char(0x89)}, // PER MILLE SIGN
				{0x0409, char(0x8A)}, // CYRILLIC CAPITAL LETTER LJE
				{0x2039, char(0x8B)}, // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
				{0x040A, char(0x8C)}, // CYRILLIC CAPITAL LETTER NJE
				{0x040C, char(0x8D)}, // CYRILLIC CAPITAL LETTER KJE
				{0x040B, char(0x8E)}, // CYRILLIC CAPITAL LETTER TSHE
				{0x040F, char(0x8F)}, // CYRILLIC CAPITAL LETTER DZHE
				{0x0452, char(0x90)}, // CYRILLIC SMALL LETTER DJE
				{0x2018, char(0x91)}, // LEFT SINGLE QUOTATION MARK
				{0x2019, char(0x92)}, // RIGHT SINGLE QUOTATION MARK
				{0x201C, char(0x93)}, // LEFT DOUBLE QUOTATION MARK
				{0x201D, char(0x94)}, // RIGHT DOUBLE QUOTATION MARK
				{0x2022, char(0x95)}, // BULLET
				{0x2013, char(0x96)}, // EN DASH
				{0x2014, char(0x97)}, // EM DASH
				{0x2122, char(0x99)}, // TRADE MARK SIGN
				{0x0459, char(0x9A)}, // CYRILLIC SMALL LETTER LJE
				{0x203A, char(0x9B)}, // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
				{0x045A, char(0x9C)}, // CYRILLIC SMALL LETTER NJE
				{0x045C, char(0x9D)}, // CYRILLIC SMALL LETTER KJE
				{0x045B, char(0x9E)}, // CYRILLIC SMALL LETTER TSHE
				{0x045F, char(0x9F)}, // CYRILLIC SMALL LETTER DZHE
				{0x00A0, char(0xA0)}, // NO-BREAK SPACE
				{0x040E, char(0xA1)}, // CYRILLIC CAPITAL LETTER SHORT U
				{0x045E, char(0xA2)}, // CYRILLIC SMALL LETTER SHORT U
				{0x0408, char(0xA3)}, // CYRILLIC CAPITAL LETTER JE
				{0x00A4, char(0xA4)}, // CURRENCY SIGN
				{0x0490, char(0xA5)}, // CYRILLIC CAPITAL LETTER GHE WITH UPTURN
				{0x00A6, char(0xA6)}, // BROKEN BAR
				{0x00A7, char(0xA7)}, // SECTION SIGN
				{0x0401, char(0xA8)}, // CYRILLIC CAPITAL LETTER IO
				{0x00A9, char(0xA9)}, // COPYRIGHT SIGN
				{0x0404, char(0xAA)}, // CYRILLIC CAPITAL LETTER UKRAINIAN IE
				{0x00AB, char(0xAB)}, // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
				{0x00AC, char(0xAC)}, // NOT SIGN
				{0x00AD, char(0xAD)}, // SOFT HYPHEN
				{0x00AE, char(0xAE)}, // REGISTERED SIGN
				{0x0407, char(0xAF)}, // CYRILLIC CAPITAL LETTER YI
				{0x00B0, char(0xB0)}, // DEGREE SIGN
				{0x00B1, char(0xB1)}, // PLUS-MINUS SIGN
				{0x0406, char(0xB2)}, // CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I
				{0x0456, char(0xB3)}, // CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I
				{0x0491, char(0xB4)}, // CYRILLIC SMALL LETTER GHE WITH UPTURN
				{0x00B5, char(0xB5)}, // MICRO SIGN
				{0x00B6, char(0xB6)}, // PILCROW SIGN
				{0x00B7, char(0xB7)}, // MIDDLE DOT
				{0x0451, char(0xB8)}, // CYRILLIC SMALL LETTER IO
				{0x2116, char(0xB9)}, // NUMERO SIGN
				{0x0454, char(0xBA)}, // CYRILLIC SMALL LETTER UKRAINIAN IE
				{0x00BB, char(0xBB)}, // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
				{0x0458, char(0xBC)}, // CYRILLIC SMALL LETTER JE
				{0x0405, char(0xBD)}, // CYRILLIC CAPITAL LETTER DZE
				{0x0455, char(0xBE)}, // CYRILLIC SMALL LETTER DZE
				{0x0457, char(0xBF)} // CYRILLIC SMALL LETTER YI
			};



		static std::string utf2cp(std::string_view utf8string)
		{
			std::string cp1251string(utf8string.size() + 1, '\x0');
			convert_utf8_to_windows1251(utf8string, &cp1251string.data()[0]);
			return cp1251string;
		}

		static std::string cp2utf(std::string_view cp1251string)
		{
			std::string utf8string;
			// rough estimation to russian text
			utf8string.reserve(2 * cp1251string.size());
			char buf[4];
			for (size_t i = 0; i < cp1251string.size(); i++)
			{
				char in[2] = { cp1251string[i] , '\x0' };
				utf8string.append(cp2utf1(buf, in));
			}
			return utf8string;
		}

	};

	// https://bjoern.hoehrmann.de/utf-8/decoder/dfa/
	class UTF8CodePointDecoder
	{
	protected:
		static constexpr uint8_t utf8d[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
											 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
											 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
											 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
											 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
											 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
											 8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
											 0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
											 0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
											 0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
											 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
											 1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
											 1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
											 1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
		};

	public:
		static constexpr uint32_t Accept = 0;
		static constexpr uint32_t Reject = 1;

		static uint32_t decode(uint32_t* state, uint32_t* codep, unsigned char byte)
		{
			uint32_t type = utf8d[byte]; *codep = (*state != Accept) ? (byte & 0x3fu) | (*codep << 6) : (0xff >> type) & (byte); *state = utf8d[256 + *state * 16 + type];
			return *state;
		};
	};

	// в реализации iconvlite есть проблема : она заканчивает работу на первом неправильном символе.
	// И даже если попытаться в вариантe iconvlite возобновлять работу с входным потоком после неправильного
	// символа, в ней нет способа синхронизироваться с первым правильным, и она будет генерировать квадраты
	// или кривые символы до того как не встретит правильный однобайтный codepoint
	// В этом варианте используется декодер, который знает размеры codepoint. Мы можем пропускать некорректные
	// или недопустимые в 1251 символы пропуская codepoint

	static std::string Utf8Tocp1251_(std::string_view utf8string)
	{
		std::string out;
		out.reserve(utf8string.size());
		std::string_view::const_iterator byte{ utf8string.begin() };
		while (byte != utf8string.end())
		{
			uint32_t state{ UTF8CodePointDecoder::Accept };
			uint32_t codepoint0{ 0 };
			uint32_t codepoint1{ 0 };
			// grab 1st byte
			switch (UTF8CodePointDecoder::decode(&state, &codepoint0, *byte))
			{
			case UTF8CodePointDecoder::Accept:
				// single byte codepoint - convert as is
				out.push_back(codepoint0);
				++byte;
				break;
			case UTF8CodePointDecoder::Reject:
				// single byte rejected (?) - wrong symbol - move to next
				out.push_back(0x7f);
				++byte;
				break;
			default:
				// multiple bytes codepoint
				// grab 2nd byte
				if (++byte == utf8string.end())
				{
					out.push_back(0x7f);
					break; // no more bytes in the input
				}
				switch (UTF8CodePointDecoder::decode(&state, &codepoint1, *byte))
				{
				case UTF8CodePointDecoder::Accept:
						{
							// two byte codepoint, try to use original iconv cp1251 magic
							uint32_t first5bit{ codepoint0 & 0x1F };
							first5bit <<= 6;
							const uint32_t sec6bit{ codepoint1 & 0x3F };
							const uint32_t unicode_char{ first5bit + sec6bit };

							if (unicode_char >= 0x410 && unicode_char <= 0x44F)
								out.push_back(unicode_char - 0x350);
							else if (unicode_char >= 0x80 && unicode_char <= 0xFF)
								out.push_back(unicode_char);
							else if (unicode_char >= 0x402 && unicode_char <= 0x403)
								out.push_back(unicode_char - 0x382);
							else
								if (const auto itutf{ IConvLite::g_letters.find(unicode_char) }; itutf != IConvLite::g_letters.end())
									out.push_back(itutf->second);
								else
								{
									// two byte codepoint does not fit into cp1251
									out.push_back(0x7f);
								}
						}
						++byte;
						break;
					case UTF8CodePointDecoder::Reject:
						// wrong two byte codepoint - move to next
						out.push_back(0x7f);
						++byte;
						break;
					default:
						// more than two byte codepoint - unsupported
						out.push_back(0x7f);
						// skip to next codepoint
						// walk until input end
						while (++byte != utf8string.end())
						{
							const auto res{ UTF8CodePointDecoder::decode(&state, &codepoint0, *byte) };
							if (res == UTF8CodePointDecoder::Accept ||
								res == UTF8CodePointDecoder::Reject)
							{
								// or multibyte codepoint either accepted or rejected
								++byte;
								break;
							}
						}
				}
			}
		}
		return out;
	}
};


template<typename T> inline void stringutils::PushContainer(T& container, std::string_view item)
{
	container.push_back(std::string(item));
}

template<> inline void stringutils::PushContainer<std::set<std::string, std::less<void>>>(std::set<std::string, std::less<void>>& container, std::string_view item)
{
	container.insert(std::string(item));
}

template<> inline void stringutils::PushContainer<std::set<std::string>>(std::set<std::string>& container, std::string_view item)
{
	container.insert(std::string(item));
}










