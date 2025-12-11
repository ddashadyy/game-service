#include <parser/json_parser.hpp>

namespace igdb {

JsonParser::OptString
JsonParser::ExtractAccessToken(std::string_view jsonResponse)
{
    return ExtractField<std::string>(jsonResponse, "access_token");
}

JsonParser::OptString
JsonParser::ExtractTokenType(std::string_view jsonResponse)
{
    return ExtractField<std::string>(jsonResponse, "token_type");
}

JsonParser::OptUint32
JsonParser::ExtractExpiresIn(std::string_view jsonResponse)
{
    return ExtractField<std::uint32_t>(jsonResponse, "expires_in");
}

} // namespace igdb