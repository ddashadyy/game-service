// project headers
#include <tools/utils.hpp>

// std
#include <iostream>
#include <regex>

//userver
#include <userver/utils/datetime.hpp>


const std::string utils::TimestampToString(time_t timestamp)
{
    if (timestamp <= 0)
        return "N/A";

    std::tm* tm = std::localtime(&timestamp);
    std::array<char, 11> buffer{};
    std::strftime(buffer.data(), buffer.size(), "%Y-%m-%d", tm);
    return std::string(buffer.data());
}

std::string utils::ForceOriginalQuality(const std::string& url)
{
    std::regex size_pattern("/t_[a-zA-Z0-9_]+/");
    return "https:" + std::regex_replace(url, size_pattern, "/t_original/");
}

::google::protobuf::Timestamp utils::TimePointToProtobuf(
    const userver::storages::postgres::TimePointWithoutTz& time_point)
{
    const auto time_string = userver::utils::datetime::Timestring(time_point);
    const auto system_time = userver::utils::datetime::Stringtime(time_string);

    const auto seconds_since_epoch =
        std::chrono::duration_cast<std::chrono::seconds>(
            system_time.time_since_epoch())
            .count();

    ::google::protobuf::Timestamp timestamp;
    timestamp.set_seconds(seconds_since_epoch);

    return timestamp;
}
