#pragma once
#include <string>
#include <map>
#include <variant>
#include <array>
#include <mutex>
#include <filesystem>
#include <fmt/core.h>

namespace RCM
{
	using ResourceDataT = std::variant<std::string>;

	class ResourceItem : protected ResourceDataT
	{
		static inline const std::string NonText_ = "non-text resource";
	public:
		ResourceItem(std::string_view data) : ResourceDataT{ std::string(data) } {};
		virtual ~ResourceItem() = default;

		operator const std::string& () const
		{
			switch (index())
			{
			case 0:
				return std::get<std::string>(*this);
			default:
				return NonText_;
			}
		}

		bool operator == (const ResourceItem& rhs) const
		{
			if (index() != rhs.index())
				return false;
			return static_cast<const std::string&>(*this) == static_cast<const std::string&>(rhs);
		}

		bool operator != (const ResourceItem& rhs) const
		{ 
			return !(*this == rhs); 
		}

		std::size_t index() const noexcept
		{
			return ResourceDataT::index();
		}
	};

	enum class LanguageId
	{
		Russian,
		English,
		Last
	};

	using ResourceKeyT = std::pair<std::string, LanguageId>;

	class ResourceId : protected ResourceKeyT
	{
	protected:
		static inline constexpr std::array<std::string_view,
			static_cast<std::underlying_type<LanguageId>::type>(LanguageId::Last)> LanguageNames_ = { {"Russian", "English"} };
	public:
		ResourceId(std::string_view Id, const LanguageId& Language = LanguageId::Last) : ResourceKeyT{ Id, Language } {}
		virtual ~ResourceId() = default;
		bool operator<(const ResourceId& rhs) const
		{
			return *static_cast<const ResourceKeyT*>(this) < static_cast<const ResourceKeyT>(rhs);
		}
		inline const LanguageId Language() const { return second; }
		inline const std::string& Id() const { return first; }
		inline const std::string_view& LanguageName() const { return LanguageName(second); }
		static const std::string_view& LanguageName(LanguageId Id) { return LanguageNames_[static_cast<std::underlying_type<LanguageId>::type>(Id)]; }
		std::string Key() const { return fmt::format("\"{}\" in {}", Id(), LanguageName()); }
	};

	class ResourceManager
	{
		using ResourceMapT = std::map<ResourceId, ResourceItem>;

	public:
		struct LanguageResourceT
		{
			LanguageId LangId_;
			const ResourceItem& data_;
		};

		static ResourceManager& GetInstance()
		{
			static ResourceManager instance;
			return instance;
		}

	private:
		ResourceMapT ResourceMap_;
		mutable ResourceMapT WrongRequests_;
		mutable LanguageId LanguageId_ = LanguageId::Russian;
		mutable std::mutex mapmutex_;

		void Add(const ResourceId& Id, const ResourceItem& data)
		{
			auto Ins{ ResourceMap_.emplace(Id, data) };
			if (!Ins.second)
			{
				const auto& dup{ Ins.first->second };
				if (ResourceItem(data) != dup)
					throw std::runtime_error(fmt::format("ResourceManager::Add - different resources for key {} : \"{}\" and \"{}\"",
						Id.Key(),
						static_cast<const std::string&>(data),
						static_cast<const std::string&>(dup)));
			}
		}

		void Add(const std::string_view Key, std::initializer_list<LanguageResourceT> LanguageList)
		{
			for (const auto& lang : LanguageList)
				Add({ Key, lang.LangId_ }, lang.data_);
		}

        //ResourceManager();
        ResourceManager()
        {}

	public:

		ResourceManager(ResourceManager const&) = delete;
		void operator=(ResourceManager const&) = delete;

		const ResourceItem& operator()(const ResourceId& Id) const
		{
			ResourceId LanguageResourceId{ Id.Id(), Id.Language() == LanguageId::Last ? Language() : Id.Language() };
			auto ResourceIt{ ResourceMap_.find(LanguageResourceId) };
			if (ResourceIt != ResourceMap_.end())
				return ResourceIt->second;

			std::lock_guard<std::mutex> guard(mapmutex_);

			for (const auto& LangId : { LanguageId::English, LanguageId::Russian })
			{
				if (LangId != LanguageResourceId.Language())
				{
					ResourceIt = ResourceMap_.find(ResourceId{ LanguageResourceId.Id(), LangId });
					if (ResourceIt != ResourceMap_.end())
					{
						return WrongRequests_.emplace(LanguageResourceId, fmt::format("{} (Translation of {} not found)",
							static_cast<const std::string&>(ResourceIt->second),
							LanguageResourceId.Key())).first->second;
					}
				}
			}
			return WrongRequests_.emplace(LanguageResourceId, fmt::format("Resource \"{}\" not found", LanguageResourceId.Id())).first->second;
		}

		const std::string& String(const ResourceId& Id) const
		{
			return static_cast<const std::string&>(operator()(Id));
		}

		const std::string& String(std::string_view Id) const
		{
			return static_cast<const std::string&>(operator()(ResourceId{ Id, Language() }));
		}

        //void Deserialize(std::filesystem::path path = cszLanguage);
        //void Serialize(std::filesystem::path path = cszLanguage);
        void Deserialize(std::filesystem::path path = cszLanguage)
        {}
        void Serialize(std::filesystem::path path = cszLanguage)
        {}

		void SetLanguage(const LanguageId& LanguageId) const
		{
			LanguageId_ = LanguageId;
		}

		inline LanguageId Language() const { return LanguageId_; }
	
		static inline constexpr const char* cszStrings = "strings";
		static inline constexpr const char* cszLanguage = "language.json";

	};
}

static inline const RCM::ResourceManager& Resources = RCM::ResourceManager::GetInstance();
