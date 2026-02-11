#pragma once

#include <list>
#include <fstream>
#include "Exceptions.h"
#include "License2/json.hpp"
#include <filesystem>

namespace fs = std::filesystem;   

//! Класс поля формы
class CUIFormField
{
public:
	//! Перечисление выравнивания контента поля
	enum class eAlignment
	{
		Left = 0,
		Right = 1
	};

protected:
	std::string Name_;
	eAlignment Alignment_ = eAlignment::Left;
	bool ReadOnly_ = false;

public:
	CUIFormField() = default;
	//! Конструктор по имени поля с параметрами
    CUIFormField(std::string_view Name,
                 eAlignment Alignment = eAlignment::Left,
                 bool ReadOnly =  false) :
		Name_(Name),
		Alignment_(Alignment),
		ReadOnly_(ReadOnly){}

	// геттеры атрибутов
	const std::string& Name() const { return Name_; }
	const eAlignment& Alignment() const { return Alignment_; }
	const bool& ReadOnly() const { return ReadOnly_; }

	// сеттеры атрибутов
	void SetName(const std::string_view Name) { Name_ = Name; }
	void SetAlignment(const eAlignment Alignment) { Alignment_ = Alignment; }
	void SetReadOnly(bool ReadOnly) { ReadOnly_ = ReadOnly; }
};

/**
 * @class CUIForm
 * @brief Описание формы для отображения данных
 *
 * Хранит информацию о том, как отображать таблицу:
 * - какие колонки показывать
 * - как называется форма
 * - вертикальная или горизонтальная ориентация
 */
class CUIForm
{
protected:
    std::string Name_;                      // Имя формы (для меню)
    std::string TableName_;                 // Имя таблицы в Rastr
	std::string Collection_;
	std::string Query_;
	std::string DetailAssignment_;
	std::string DetailQueryColumnName_;
    std::string MenuPath_;                  // Путь в меню
	bool HasDetailForm_ = false;
    int AddToMenuIndex_ = 0;                // Индекс в меню
    bool Vertical_ = false;                 // Вертикальная ориентация
    std::list<CUIFormField> Fields_;        // Поля для отображения
public:

	CUIForm() = default;
	//! Конструктор по имени формы
	CUIForm(const std::string_view Name) : Name_(Name) {}
	CUIForm(const CUIForm& Form) = default;

	// геттеры атрибутов
	const std::string& Name() const { return Name_; }
	const std::string& TableName() const { return TableName_; }
	const std::string& Collection() const { return Collection_; }
	const std::string& Query() const { return Query_; }
	const std::string& DetailAssignment() const { return DetailAssignment_; }
	const std::string& DetailQueryColumnName() const { return DetailQueryColumnName_; }
	const std::string& MenuPath() const { return MenuPath_; }
	const bool& HasDetailForm() const { return HasDetailForm_; }
	const int& AddToMenuIndex() const { return AddToMenuIndex_; }
	const bool& Vertical() const { return Vertical_; }
	const auto& Fields() const { return Fields_; }
	auto& Fields() { return Fields_; }

	// сеттеры атрибутов
	void SetName(const std::string_view Name) { Name_ = Name; }
	void SetTableName(const std::string_view TableName) { TableName_ = TableName; }
	void SetCollection(const std::string_view Collection) { Collection_ = Collection; }
	void SetQuery(const std::string_view Query) { Query_ = Query; }
	void SetDetailAssignment(const std::string_view DetailAssignment) { DetailAssignment_ = DetailAssignment; }
	void SetDetailQueryColumnName(const std::string_view DetailQueryColumnName) { DetailQueryColumnName_ = DetailQueryColumnName; }
	void SetMenuPath(const std::string_view MenuPath) { MenuPath_ = MenuPath; }
	void SetHasDetailForm(const bool HasDetailForm) { HasDetailForm_ = HasDetailForm; }
	void SetAddToMenuIndex(const int AddToMenuIndex) { AddToMenuIndex_ = AddToMenuIndex; }
	void SetVertical(const bool Vertical) { Vertical_ = Vertical; }
};

//! Коллекция форм
class CUIFormsCollection
{
protected:
	std::list<CUIForm> Forms_;
public:
	//! Доступ к const коллекции форм 
	const auto& Forms() const { return Forms_; }
	//! Доступ к коллекции форм 
	auto& Forms() { return Forms_; }
	CUIFormsCollection() noexcept = default;
};

//! Базовый класс сериализатора коллекции форм
class CUIFormCollectionSerializerBase
{
public:
	static inline constexpr const char* RCS_UIFormsSerializationError = "RCS_UIFormsSerializationError";
	static inline constexpr const char* RCS_UIFormsDeserializationError = "RCS_UIFormsDeserializationError";
	static inline constexpr const char* RCS_UIFormsBadFormat = "RCS_UIFormsBadFormat";
	static inline constexpr const char* RCS_UIFormsFileNotFound = "RCS_UIFormsFileNotFound";
	static inline constexpr const char* RCS_UIFormsCannotBeOpened = "RCS_UIFormsCannotBeOpened";

	static std::string SerializationErrorText(std::string_view Text)
	{
		return fmt::format(Resources.String(RCS_UIFormsSerializationError), Text);
	}

	static std::string DeserializationErrorText(std::string_view Text)
	{
		return fmt::format(Resources.String(RCS_UIFormsDeserializationError), Text);
	}
};

//! Класс сериализатора коллекции форм в/из файла
class CUIFromCollectionSerializerFile : public CUIFormCollectionSerializerBase
{
protected:
	const fs::path Path_;
	void ThrowExceptionOnFile(const std::string_view What, const fs::path& Path) const
	{
		throw CException(szExceptionOnFile, What, Path.string());
	}
	void ThrowGLEExceptionOnFile(const std::string_view What, const fs::path& Path) const
	{
		throw CExceptionGLE(szExceptionOnFile, What, Path.string());
	}
public:
	//! конструктор принимает путь к файлу
	CUIFromCollectionSerializerFile(const fs::path Path) : Path_(Path) {}
	static inline constexpr const char* szExceptionOnFile = "{} : \"{}\"";
};

//! Класс сериализатора нативного формата RastrWin
class CUIFormCollectionSerializerBinary : public CUIFromCollectionSerializerFile
{
public:
	using CUIFromCollectionSerializerFile::CUIFromCollectionSerializerFile;

	//! Сериализация коллекции форм в файл, заданный в параметре
    const CUIFormsCollection& Serialize(const fs::path& Path,
                                        const CUIFormsCollection& Forms) const
	{
		try
		{
			std::ofstream formstream(Path, std::ios::binary);
			if (!formstream.is_open())
                throw CExceptionGLE(Resources.String
                                (CUIFormCollectionSerializerBase::RCS_UIFormsCannotBeOpened));
			WriteInt32(static_cast<std::int32_t>(Forms.Forms().size()), formstream);
			for (const auto& form : Forms.Forms())
				SerializeForm(form, formstream);
		}
		catch (const std::ofstream::failure& ex)
		{
			ThrowGLEExceptionOnFile(SerializationErrorText(ex.what()), Path);
		}
		catch (const std::exception& ex)
		{
			ThrowExceptionOnFile(SerializationErrorText(ex.what()), Path);
		}
		return Forms;
	}

	//! Сериализация коллекции форм в файл, заданный в конструкторе
	const CUIFormsCollection& Serialize(const CUIFormsCollection& Forms) const
	{
		return Serialize(Path_, Forms);
	}

	//! Десериализация из файла, заданного в конструторе
	/*!
	* Возвращает созданный объект форм
	*/
	CUIFormsCollection Deserialize()
	{
		// создаем формы
		CUIFormsCollection Forms;
		// десериализуем их из файла и возвращаем
		return Deserialize(Path_, Forms);
	}

	//! Десериализация из файла, заданного в параметре
	CUIFormsCollection& Deserialize(const fs::path& Path, CUIFormsCollection& Forms)
	{
		try
		{
			if (!fs::exists(Path))
				throw CExceptionGLE(Resources.String(CUIFormCollectionSerializerBase::RCS_UIFormsFileNotFound));
			std::ifstream formstream(Path, std::ios::binary);
			if (!formstream.is_open())
				throw CExceptionGLE(Resources.String(CUIFormCollectionSerializerBase::RCS_UIFormsCannotBeOpened));

			formstream.exceptions(formstream.exceptions() | std::ios::failbit);
			std::int32_t FormsCount{ ReadInt32(formstream) };

			Forms.Forms().clear();
			while (FormsCount--)
				DeserializeForm(Forms.Forms().emplace_back(), formstream);
		}
		catch (const std::ifstream::failure& ex)
		{
			ThrowGLEExceptionOnFile(DeserializationErrorText(ex.what()), Path);
		}
		catch (const std::exception& ex)
		{
			ThrowExceptionOnFile(DeserializationErrorText(ex.what()), Path);
		}
		return Forms;
	}

protected:

	// функции бинарного I/O
	static std::int32_t ReadInt32(std::ifstream& formstream)
	{
		std::int32_t Value{ 0 };
		formstream.read(reinterpret_cast<char*>(&Value), sizeof(Value));
		if (Value < 0)
			throw CException(Resources.String(CUIFormCollectionSerializerBase::RCS_UIFormsBadFormat));
		return Value;
	}

	static std::string ReadString(std::ifstream& formstream)
	{
		std::string Buffer(ReadInt32(formstream), '\x0');
		formstream.read(reinterpret_cast<char*>(&Buffer[0]), Buffer.size());
		Buffer.resize(Buffer.size() - 1);
		return Buffer;
	}

	static bool ReadBool(std::ifstream& formstream)
	{
		char Bool{ false };
		formstream.read(&Bool, sizeof(Bool));
		return Bool != '\x0';
	}

	static void WriteInt32(std::int32_t Value, std::ofstream& formstream)
	{
		formstream.write(reinterpret_cast<const char*>(&Value), sizeof(Value));
	}

	static void WriteString(const std::string& String, std::ofstream& formstream)
	{
		WriteInt32(static_cast<std::int32_t>(String.size() + 1), formstream);
		formstream.write(reinterpret_cast<const char*>(&String[0]), String.size() + 1);
	}

	static void WriteBool(const bool& Bool, std::ofstream& formstream)
	{
		const char Boolean{ Bool ? '\x1' : '\x0' };
		formstream.write(&Boolean, sizeof(Boolean));
	}

	void SerializeField(const CUIFormField& Field, std::ofstream& formstream) const
	{
		WriteString(Field.Name(), formstream);
		WriteInt32(static_cast<std::int32_t>(Field.Alignment()), formstream);
		WriteInt32(static_cast<std::int32_t>(Field.ReadOnly()), formstream);
	}

	void SerializeForm(const CUIForm& Form, std::ofstream& formstream) const
	{
		WriteString(Form.Name(), formstream);
		WriteString(Form.TableName(), formstream);
		WriteString(Form.Query(), formstream);
		WriteString(Form.DetailAssignment(), formstream);
		WriteString(Form.DetailQueryColumnName(), formstream);
		WriteString(Form.MenuPath(), formstream);
		WriteBool(Form.HasDetailForm(), formstream);
		WriteInt32(static_cast<std::int32_t>(Form.AddToMenuIndex()), formstream);
		WriteInt32(static_cast<std::int32_t>(Form.Vertical()), formstream);
		WriteInt32(static_cast<std::int32_t>(Form.Fields().size()), formstream);

		for (const auto& field : Form.Fields())
			SerializeField(field, formstream);
	}

	void DeserializeField(CUIFormField& Field, std::ifstream& formstream)
	{
		Field.SetName(ReadString(formstream));
		switch (ReadInt32(formstream))
		{
		case 0:
			Field.SetAlignment(CUIFormField::eAlignment::Left);
			break;
		case 1:
			Field.SetAlignment(CUIFormField::eAlignment::Right);
			break;
		default:
			throw CException(Resources.String(CUIFormCollectionSerializerBase::RCS_UIFormsBadFormat));
		}
		Field.SetReadOnly(ReadInt32(formstream));
	}

	void DeserializeForm(CUIForm& Form, std::ifstream& formstream)
	{
		Form.SetName(ReadString(formstream));
		Form.SetTableName(ReadString(formstream));
		Form.SetQuery(ReadString(formstream));
		Form.SetDetailAssignment(ReadString(formstream));
		Form.SetDetailQueryColumnName(ReadString(formstream));
		Form.SetMenuPath(ReadString(formstream));
		Form.SetHasDetailForm(ReadBool(formstream));
		Form.SetAddToMenuIndex(ReadInt32(formstream));
		Form.SetVertical(ReadInt32(formstream));

		std::int32_t FieldsCount{ ReadInt32(formstream) };
		while (FieldsCount--)
			DeserializeField(Form.Fields().emplace_back(), formstream);
	}
};

//! Класс сериализатора в формате json
class CUIFormCollectionSerializerJson : public CUIFromCollectionSerializerFile
{
public:
	using CUIFromCollectionSerializerFile::CUIFromCollectionSerializerFile;

	class JsonSerializerException : public CException
	{
	public:
		using CException::CException;
	};

	//! Сериализация в файл json, заданный в параметрах
	const CUIFormsCollection& Serialize(const fs::path& Path, const CUIFormsCollection& Forms, bool SkipDefault = true) const
	{
		try
		{
			auto jfile = nlohmann::json();
			Serialize(jfile, Forms, SkipDefault);
			std::ofstream formstream(Path, std::ios::binary);
			if (!formstream.is_open())
				throw CExceptionGLE(Resources.String(CUIFormCollectionSerializerBase::RCS_UIFormsCannotBeOpened));
			formstream << jfile.dump(4);
			return Forms;
		}
		catch (const JsonSerializerException& ex)
		{
			ThrowExceptionOnFile(ex.what(), Path);
		}
		catch (const std::ofstream::failure& ex)
		{
			ThrowGLEExceptionOnFile(SerializationErrorText(ex.what()), Path);
		}
		catch (const std::exception& ex)
		{
			ThrowExceptionOnFile(SerializationErrorText(ex.what()), Path);
		}
	}

	//! Сериализация в в файл json, заданный в конструкторе
	const CUIFormsCollection& Serialize(const CUIFormsCollection& Forms, bool SkipDefault = true) const
	{
		return Serialize(Path_, Forms, SkipDefault);
	}

	//! Десериализация из файла, заданного в конструторе
	/*!
	* Возвращает созданный объект форм
	*/
	CUIFormsCollection Deserialize()
	{
		CUIFormsCollection Forms;
		return Deserialize(Path_, Forms);
	}

	//! Десериалиация из файла, заданного в параметре
	CUIFormsCollection& Deserialize(const fs::path& Path, CUIFormsCollection& Forms)
	{
		try
		{
			if (!fs::exists(Path))
				throw CExceptionGLE(Resources.String(CUIFormCollectionSerializerBase::RCS_UIFormsFileNotFound));
			std::ifstream formstream(Path, std::ios::binary);
			if (!formstream.is_open())
				throw CExceptionGLE(Resources.String(CUIFormCollectionSerializerBase::RCS_UIFormsCannotBeOpened));
			auto json = nlohmann::json();
			formstream >> json;
			return Deserialize(Forms, json);
		}
		catch (const JsonSerializerException& ex)
		{
			ThrowExceptionOnFile(ex.what(), Path);
		}
		catch (const std::ifstream::failure& ex)
		{
			ThrowGLEExceptionOnFile(DeserializationErrorText(ex.what()), Path);
		}
		catch (const std::exception& ex)
		{
			ThrowExceptionOnFile(DeserializationErrorText(ex.what()), Path);
		}
        return Forms;
	}

	//! Десериализация из объекта json
	CUIFormsCollection& Deserialize(CUIFormsCollection& Forms, const nlohmann::json& json)
	{
		try
		{
			Forms.Forms().clear();
			for (const auto& form : json.at(CUIFormCollectionSerializerJson::szForms))
				DeserializeForm(Forms.Forms().emplace_back(), form);
			return Forms;
		}
		catch (const std::exception& ex)
		{
			throw JsonSerializerException(DeserializationErrorText(ex.what()));
		}
	}

	//! Сериализация в объект json
	const CUIFormsCollection& Serialize(nlohmann::json& json, const CUIFormsCollection& Forms, bool SkipDefault) const
	{
		try
		{
			auto jforms = nlohmann::json::array();
			for (const auto& form : Forms.Forms())
				SerializeForm(form, jforms, SkipDefault);
			json[CUIFormCollectionSerializerJson::szForms] = jforms;
			return Forms;
		}
		catch (const std::exception& ex)
		{
			throw JsonSerializerException(SerializationErrorText(ex.what()));
		}
	}

	static void ConvertAllFormsToJson(fs::path path)
	{
		for (const auto& file : fs::directory_iterator(path))
			if (file.path().extension() == ".fm")
			{
				CUIFormCollectionSerializerBinary bs(file.path());
				const auto FormsCollection{ bs.Deserialize() };
				CUIFormCollectionSerializerJson js(fs::path(file.path()).replace_extension(".json"));
				js.Serialize(FormsCollection, true);
			}
	}

protected:

	// функции json I/O
	
	void SetStringJsonAttribute(nlohmann::json& json, const std::string& AttributeName, const std::string_view Value, bool SkipDefault) const
	{
		if (SkipDefault && Value.empty()) return;
		json[AttributeName] = stringutils::acp_decode(Value);
	}

	void SerializeField(const CUIFormField& Field, nlohmann::json& jfield, bool SkipDefault) const
	{
		SetStringJsonAttribute(jfield, CUIFormCollectionSerializerJson::szName, Field.Name(), false);
		if (const auto& val{ Field.Alignment() }; val != CUIFormField::eAlignment::Left)
			jfield[CUIFormCollectionSerializerJson::szAlignment] = CUIFormCollectionSerializerJson::szRight;
		else if(!SkipDefault)
			jfield[CUIFormCollectionSerializerJson::szAlignment] = CUIFormCollectionSerializerJson::szLeft;
		if (const auto& val{ Field.ReadOnly() }; !SkipDefault || val)
			jfield[CUIFormCollectionSerializerJson::szReadOnly] = val;
	}

	void SerializeForm(const CUIForm& Form, nlohmann::json& json, bool SkipDefault) const
	{
		auto jform{ nlohmann::json::object() };
		SetStringJsonAttribute(jform, CUIFormCollectionSerializerJson::szName, Form.Name(), false);
		SetStringJsonAttribute(jform, CUIFormCollectionSerializerJson::szTableName, Form.TableName(), false);
		SetStringJsonAttribute(jform, CUIFormCollectionSerializerJson::szQuery, Form.Query(), SkipDefault);
		SetStringJsonAttribute(jform, CUIFormCollectionSerializerJson::szDetailAssignment, Form.DetailAssignment(), SkipDefault);
		SetStringJsonAttribute(jform, CUIFormCollectionSerializerJson::szDetailQueryColumnName, Form.DetailQueryColumnName(), SkipDefault);
		SetStringJsonAttribute(jform, CUIFormCollectionSerializerJson::szMenuPath, Form.MenuPath(), SkipDefault);
		if (const auto& val{ Form.HasDetailForm() }; !SkipDefault || val)
			jform[CUIFormCollectionSerializerJson::szHasDetailForm] = val;
		if (const auto& val{ Form.AddToMenuIndex() }; !SkipDefault || val != 0)
			jform[CUIFormCollectionSerializerJson::szAddToMenuIndex] = val;
		if (const auto& val{ Form.Vertical() }; !SkipDefault || val)
			jform[CUIFormCollectionSerializerJson::szVertical] = val;

		auto jfields = nlohmann::json::array();
		for (const auto& field : Form.Fields())
		{
			auto jfield = nlohmann::json::object();
			SerializeField(field, jfield, SkipDefault);
			jfields.emplace_back(jfield);
		}
		jform[CUIFormCollectionSerializerJson::szFields] = jfields;
		json.emplace_back(jform);
	}

	void DeserializeField(CUIFormField& Field, const nlohmann::json& jfield)
	{
		Field.SetName(stringutils::acp_encode(jfield.at(CUIFormCollectionSerializerJson::szName).get<std::string>()));
		Field.SetReadOnly(jfield.value(CUIFormCollectionSerializerJson::szReadOnly, false));
		if(const auto& alignstring{ jfield.value(CUIFormCollectionSerializerJson::szAlignment, CUIFormCollectionSerializerJson::szLeft) };
			alignstring == CUIFormCollectionSerializerJson::szRight)
			Field.SetAlignment(CUIFormField::eAlignment::Right);
		else if(alignstring == CUIFormCollectionSerializerJson::szLeft)
			Field.SetAlignment(CUIFormField::eAlignment::Left);
		else
			throw CException(Resources.String(CUIFormCollectionSerializerBase::RCS_UIFormsBadFormat));
	}

	void DeserializeForm(CUIForm& Form, const nlohmann::json& jform)
	{
		Form.SetName(stringutils::acp_encode(jform.at(CUIFormCollectionSerializerJson::szName).get<std::string>()));
		Form.SetTableName(stringutils::acp_encode(jform.at(CUIFormCollectionSerializerJson::szTableName).get<std::string>()));
		Form.SetQuery(stringutils::acp_encode(jform.value(CUIFormCollectionSerializerJson::szQuery, {})));
		Form.SetDetailAssignment(stringutils::acp_encode(jform.value(CUIFormCollectionSerializerJson::szDetailAssignment, {})));
		Form.SetDetailQueryColumnName(stringutils::acp_encode(jform.value(CUIFormCollectionSerializerJson::szDetailQueryColumnName, {})));
		Form.SetMenuPath(stringutils::acp_encode(jform.value(CUIFormCollectionSerializerJson::szMenuPath, {})));
		Form.SetHasDetailForm(jform.value(CUIFormCollectionSerializerJson::szHasDetailForm, false));
		Form.SetAddToMenuIndex(jform.value(CUIFormCollectionSerializerJson::szAddToMenuIndex, 0));
		Form.SetVertical(jform.value(CUIFormCollectionSerializerJson::szVertical, false));
		for (const auto& field : jform.at(CUIFormCollectionSerializerJson::szFields))
			DeserializeField(Form.Fields().emplace_back(), field);
	}

	static inline constexpr const char* szTableName = "tableName";
	static inline constexpr const char* szQuery = "query";
	static inline constexpr const char* szDetailAssignment = "detailAssignment";
	static inline constexpr const char* szDetailQueryColumnName = "detailQueryColumnName";
	static inline constexpr const char* szMenuPath = "menuPath";
	static inline constexpr const char* szHasDetailForm = "hasDetailForm";
	static inline constexpr const char* szAddToMenuIndex = "addToMenuIndex";
	static inline constexpr const char* szVertical = "vertical";
	static inline constexpr const char* szFields = "fields";
	static inline constexpr const char* szName = "name";
	static inline constexpr const char* szAlignment = "alignment";
	static inline constexpr const char* szReadOnly = "readOnly";
	static inline constexpr const char* szLeft = "left";
	static inline constexpr const char* szRight = "right";
	static inline constexpr const char* szForms = "forms";
};
