#include <gtest/gtest.h>
#include <tools/utils.hpp>

#include <cstdlib>
#include <userver/utils/datetime.hpp>

namespace utils::test {

class UtilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        setenv("TZ", "UTC", 1);
        tzset();
    }
};

TEST_F(UtilsTest, ForceOriginalQuality_ThumbToOriginal)
{
    std::string input =
        "//images.igdb.com/igdb/image/upload/t_thumb/co2vcd.jpg";
    std::string expected =
        "https://images.igdb.com/igdb/image/upload/t_original/co2vcd.jpg";

    EXPECT_EQ(utils::ForceOriginalQuality(input), expected);
}

TEST_F(UtilsTest, ForceOriginalQuality_CoverBigToOriginal)
{
    std::string input =
        "//images.igdb.com/igdb/image/upload/t_cover_big/xyz123.jpg";
    std::string expected =
        "https://images.igdb.com/igdb/image/upload/t_original/xyz123.jpg";

    EXPECT_EQ(utils::ForceOriginalQuality(input), expected);
}

TEST_F(UtilsTest, ForceOriginalQuality_AddsHttpsPrefix)
{
    std::string input = "//example.com/image.jpg";
    std::string expected = "https://example.com/image.jpg";

    EXPECT_EQ(utils::ForceOriginalQuality(input), expected);
}

TEST_F(UtilsTest, ForceOriginalQuality_EmptyString)
{
    std::string input = "";
    std::string expected = "https:";

    EXPECT_EQ(utils::ForceOriginalQuality(input), expected);
}

TEST_F(UtilsTest, TimestampToString_ValidDate)
{
    time_t ts = 1672531200;
    EXPECT_EQ(utils::TimestampToString(ts), "2023-01-01");
}

TEST_F(UtilsTest, TimestampToString_Zero)
{
    EXPECT_EQ(utils::TimestampToString(0), "N/A");
}

TEST_F(UtilsTest, TimestampToString_Negative)
{
    EXPECT_EQ(utils::TimestampToString(-100), "N/A");
}

TEST_F(UtilsTest, TimePointToProtobuf_Conversion)
{
    std::string time_str = "2023-10-10T12:00:00+0000";
    auto sys_tp = userver::utils::datetime::Stringtime(time_str);
    userver::storages::postgres::TimePointWithoutTz tp_without_tz(sys_tp);

    auto proto_ts = utils::TimePointToProtobuf(tp_without_tz);
    auto expected_seconds = std::chrono::duration_cast<std::chrono::seconds>(
                                sys_tp.time_since_epoch())
                                .count();

    EXPECT_EQ(proto_ts.seconds(), expected_seconds);
}

TEST_F(UtilsTest, TimePointToProtobuf_Epoch)
{
    auto sys_tp = std::chrono::system_clock::from_time_t(0);
    userver::storages::postgres::TimePointWithoutTz tp(sys_tp);

    auto proto_ts = utils::TimePointToProtobuf(tp);

    EXPECT_EQ(proto_ts.seconds(), 0);
}

} // namespace utils::test