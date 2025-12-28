#pragma once

#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace igdb {

class JsonParser
{
public:
    using OptString = std::optional<std::string>;
    using OptUint32 = std::optional<std::uint32_t>;

    
    static OptString ExtractAccessToken(std::string_view jsonResponse);
    static OptString ExtractTokenType(std::string_view jsonResponse);
    static OptUint32 ExtractExpiresIn(std::string_view jsonResponse);

    template <typename T>
    static std::optional<T> ExtractField(std::string_view jsonResponse,
                                         std::string_view fieldName);
};

template <typename T>
inline std::optional<T> JsonParser::ExtractField(std::string_view jsonResponse,
                                                 std::string_view fieldName)
{
    try
    {
        auto json = nlohmann::json::parse(jsonResponse);

        if (json.contains(fieldName))
            return json[fieldName].get<T>();
    }
    catch (const nlohmann::json::exception& e)
    {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
    }

    return std::nullopt;
}

} // namespace igdb