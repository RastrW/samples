#include "IDataBlocksWrappers.h"

template<> eFieldVariantType MapFieldVariantType<FieldVariantData>::Type() noexcept { return eFieldVariantType::Monostate; }
template<> eFieldVariantType MapFieldVariantType<double>::Type() noexcept { return eFieldVariantType::Double; }
template<> eFieldVariantType MapFieldVariantType<bool>::Type() noexcept { return eFieldVariantType::Bool; }
template<> eFieldVariantType MapFieldVariantType<std::string>::Type() noexcept { return eFieldVariantType::String; }
template<> eFieldVariantType MapFieldVariantType<long>::Type() noexcept { return eFieldVariantType::Long; }
template<> eFieldVariantType MapFieldVariantType<uint64_t>::Type() noexcept { return eFieldVariantType::Uint64; }
template<> const std::string_view MapFieldVariantType<FieldVariantData>::VerbalType() noexcept { return "monostate"; }
template<> const std::string_view MapFieldVariantType<double>::VerbalType() noexcept { return "double"; }
template<> const std::string_view MapFieldVariantType<bool>::VerbalType() noexcept { return "bool"; }
template<> const std::string_view MapFieldVariantType<std::string>::VerbalType() noexcept { return "string"; }
template<> const std::string_view MapFieldVariantType<long>::VerbalType() noexcept { return "long"; }
template<> const std::string_view MapFieldVariantType<uint64_t>::VerbalType() noexcept { return "uint64"; }

template<> std::string VariantToString<FieldVariantData>::String(const FieldVariantData& Value)
{
    return std::visit(ToString(), Value);
}
template<> std::string VariantToString<std::string>::String(const std::string& Value)
{
    return Value;
}

const std::string_view MapFieldVariantName(eFieldVariantType Type)
{
    switch (Type)
    {
    case eFieldVariantType::Monostate: return MapFieldVariantType<FieldVariantData>::VerbalType();
    case eFieldVariantType::Long: return MapFieldVariantType<long>::VerbalType();
    case eFieldVariantType::Double: return MapFieldVariantType<double>::VerbalType();
    case eFieldVariantType::Bool: return MapFieldVariantType<bool>::VerbalType();
    case eFieldVariantType::String: return MapFieldVariantType<std::string>::VerbalType();
    case eFieldVariantType::Uint64: return MapFieldVariantType<uint64_t>::VerbalType();
    default: return nullptr;
    }
}
