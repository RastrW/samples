#pragma once
#include <string>
#include <exception>
#include <astra/stringutils.h>
#include "ResourceManager.h"

#ifdef _MSC_VER
#ifndef CMAKE_PLAIN
#include <Windows.h>
#endif
#endif

#undef VIEW_FUNCTION
#if defined(__GNUC__) || defined(__MINGW__) || defined(__MINGW32__)
#define VIEW_FUNCTION __PRETTY_FUNCTION__
#else
#ifdef _MSC_VER
#define __VIEW_FUNCTION__ __FUNCSIG__
#else
#define VIEW_FUNCTION __FUNCTION__
#endif
#endif

//Макро для сокращения пути, возвращаемого макросом __FILE__
//вместо C:\rastr\RastrWin\astra\Col.cpp получаем astra\Col.cpp
inline std::string get_short_file(const char* file) {
	std::filesystem::path path = std::filesystem::path(file);
	std::filesystem::path short_path = path.parent_path().filename() / path.filename();
	return short_path.string();
}

//Макро для создания исключения с информацией о файле, строке и функции, в которых оно будет создано
#define RASTR_EXCEPTION_SOURCE(Format, ...) CException::make_source_error_exception(Format, get_short_file(__FILE__), __VIEW_FUNCTION__, __LINE__, __VA_ARGS__)
//Аналогичный функционал можно реализовать без макросов, если использовать std::source_location из c++20

// Я прям настаиваю что все исключения должны наследоваться от std::exception
// чтобы не париться с catch и иметь нормальный what

// простое исключение с вариадиком
class CException : public std::runtime_error
{
public:
	template <typename... Args>
	CException(std::string_view Format, Args&&... args) : std::runtime_error(fmt::format(Format, args...)) {}

	template <typename... Args>
	static CException make_source_error_exception(std::string_view Format,
		const std::string& short_file, const char* function, std::size_t line, Args&&... args) {
		using namespace std::literals;
		return CException(fmt::format("file: {}, line: {}, function: {}; "sv, short_file, line, function) + std::string(Format), args...);
	}
};

// исключение вместо Cmyex
// Cmyex явно же использовался для исключения
// доступа к кривым реквизитам БД

// Ну так и сделали бы его для этого как следует
class CBadDBAccess : public std::runtime_error
{
protected:
	std::string Field_;		// Поле и Таблица вместо всяких nam1, nam2
	std::string Table_;
	int Type_;				// Тип хз зачем нужен (подозреваю что для НЕ исключений БД)
public:
	// поскольку наследуемся от std::exception сразу формируем what()
	// вообще исключение должно формироваться полностью в конструкторе
	// так как у нас исключение это строка - формируем ее сразу
	// по идее всякое гавно типа полей и таблиц не нужно,
	// но если вам нравится чето еще формировать из ресурсов - вперед
	CBadDBAccess(int Type, std::string_view Field, std::string_view Table) :
		std::runtime_error(CBadDBAccess::GetWhat(Field, Table)),
		Field_(Field),
		Table_(Table),
		Type_(Type) { };

	// геттеры
	const char* Field() const { return Field_.c_str(); }
	const char* Table() const { return Table_.c_str(); }
	const int Type() const { return Type_; }

	// формирование what для конструктора
	static std::string GetWhat(std::string_view Field, std::string_view Table)
	{
		if (Field.empty())	// если поле пустое - пишем про таблицу
			return fmt::format(Resources.String(RCS_TableNotFound), Table);
		else // если поле и таблица не пустые - пишем про поле и таблицу
			return fmt::format(Resources.String(RCS_FieldInTableNotFound), Field, Table);
	}

	static constexpr const char* RCS_TableNotFound = "RCS_TableNotFound";
	static constexpr const char* RCS_FieldInTableNotFound  = "RCS_FieldInTableNotFound";
};

// добавляет к строке исключение информацию от GetLastError
class CExceptionGLE : public CException
{
public:
	template <typename... Args>
	CExceptionGLE(std::string_view Format, Args&&... args) : CException(MessageFormat(fmt::format(Format, args...))) {}

	static std::string MessageFormat(std::string_view Message)
	{
		std::string message;
		int Code{ errno };
#ifdef _MSC_VER
		const DWORD dwError{ ::GetLastError() };

		// Описание ошибки от code приходит в CP_ACP, поэтому его декодируем с помощью
		// утилиты acp_decode
		// https://blogs.msmvps.com/gdicanio/2017/08/16/what-is-the-encoding-used-by-the-error_code-message-string/

		if (dwError != 0)
		{
			LPWSTR messageBuffer{ nullptr };
			const size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);
			std::wstring wmessage(messageBuffer, size);
			LocalFree(messageBuffer);
			message = stringutils::acp_encode(stringutils::utf8_encode(wmessage));
			Code = static_cast<int>(dwError);
		}
		else
		{
			const std::error_code code(Code, std::system_category());
			message = code.message();
		}
#else
		std::error_code code(Code, std::system_category());
		message = code.message();
#endif 
		stringutils::removecrlf(message);
		return fmt::format(Resources.String(RCS_GLESystemError), Message, Code, message);
	}

	static inline constexpr const char* RCS_GLESystemError = "RCS_GLESystemError";
};
