#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <parser/json_parser.hpp>

namespace igdb::test {

class JsonParserTest : public ::testing::Test
{
protected:
    const std::string kValidAuthJson = R"({
        "access_token": "def1234567890abcdef",
        "expires_in": 3600,
        "token_type": "bearer"
    })";

    const std::string kEmptyJson = "{}";
    const std::string kMalformedJson = "{ invalid_json: ";
    const std::string kWrongTypesJson = R"({
        "access_token": 12345,
        "expires_in": "not_a_number"
    })";
};


TEST_F(JsonParserTest, ExtractAccessToken_Success)
{
    auto result = JsonParser::ExtractAccessToken(kValidAuthJson);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "def1234567890abcdef");
}

TEST_F(JsonParserTest, ExtractAccessToken_NotFound)
{
    auto result = JsonParser::ExtractAccessToken(kEmptyJson);

    EXPECT_FALSE(result.has_value());
}

TEST_F(JsonParserTest, ExtractAccessToken_MalformedJson)
{
    auto result = JsonParser::ExtractAccessToken(kMalformedJson);

    EXPECT_FALSE(result.has_value());
}

TEST_F(JsonParserTest, ExtractAccessToken_WrongType)
{
    auto result = JsonParser::ExtractAccessToken(kWrongTypesJson);

    EXPECT_FALSE(result.has_value());
}


TEST_F(JsonParserTest, ExtractExpiresIn_Success)
{
    auto result = JsonParser::ExtractExpiresIn(kValidAuthJson);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 3600);
}

TEST_F(JsonParserTest, ExtractExpiresIn_NotFound)
{
    auto result = JsonParser::ExtractExpiresIn(kEmptyJson);

    EXPECT_FALSE(result.has_value());
}

TEST_F(JsonParserTest, ExtractExpiresIn_WrongType)
{
    auto result = JsonParser::ExtractExpiresIn(kWrongTypesJson);

    EXPECT_FALSE(result.has_value());
}

TEST_F(JsonParserTest, ExtractTokenType_Success)
{
    auto result = JsonParser::ExtractTokenType(kValidAuthJson);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "bearer");
}

TEST_F(JsonParserTest, ExtractTokenType_NotFound)
{
    std::string jsonWithoutType = R"({"access_token": "123"})";
    auto result = JsonParser::ExtractTokenType(jsonWithoutType);

    EXPECT_FALSE(result.has_value());
}

TEST_F(JsonParserTest, ExtractField_CustomType)
{
    std::string jsonBool = R"({"is_active": true})";
    
    auto result = JsonParser::ExtractField<bool>(jsonBool, "is_active");

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value());
}

TEST_F(JsonParserTest, ExtractField_NestedNotSupportedButHandled)
{
    std::string nested = R"({ "data": { "id": 1 } })";

    auto result = JsonParser::ExtractField<int>(nested, "id");
    EXPECT_FALSE(result.has_value());
}

} // namespace igdb::test