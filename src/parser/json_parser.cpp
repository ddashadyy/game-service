#include <parser/json_parser.hpp>

std::optional<std::string>
igdb::JsonParser::ExtractAccessToken(std::string_view jsonResponse)
{
    return ExtractField<std::string>(jsonResponse, "access_token");
}

std::optional<std::string>
igdb::JsonParser::ExtractTokenType(std::string_view jsonResponse)
{
    return ExtractField<std::string>(jsonResponse, "token_type");
}

std::optional<std::uint32_t>
igdb::JsonParser::ExtractExpiresIn(std::string_view jsonResponse)
{
    return ExtractField<std::uint32_t>(jsonResponse, "expires_in");
}
