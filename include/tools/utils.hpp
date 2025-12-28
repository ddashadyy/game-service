#pragma once

// std
#include <string>

// userver
#include <google/protobuf/timestamp.pb.h>
#include <userver/storages/postgres/io/chrono.hpp>


namespace utils {


const std::string TimestampToString(time_t timestamp);

std::string ForceOriginalQuality(const std::string& url);

::google::protobuf::Timestamp TimePointToProtobuf(
    const userver::storages::postgres::TimePointWithoutTz& time_point);

} // namespace utils
