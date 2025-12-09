#pragma once

#include <string>
#include <optional>
#include <iostream>
#include <nlohmann/json.hpp>

namespace igdb {

class JsonParser 
{
public:
    static std::optional<std::string> ExtractAccessToken(std::string_view jsonResponse);
    static std::optional<std::string> ExtractTokenType(std::string_view jsonResponse);
    static std::optional<std::uint32_t> ExtractExpiresIn(std::string_view jsonResponse);
    
    template<typename T>
    static std::optional<T> ExtractField(std::string_view jsonResponse, std::string_view fieldName);
};

template <typename T>
inline std::optional<T> JsonParser::ExtractField(std::string_view jsonResponse, std::string_view fieldName) 
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

}  // namespace igdb