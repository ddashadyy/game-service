#pragma once

// std
#include <string>
#include <string_view>

// boost
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>

// userver
#include <google/protobuf/timestamp.pb.h>
#include <userver/storages/postgres/io/chrono.hpp>


namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

using tcp = net::ip::tcp;

namespace utils {

const std::string PerformHttpRequest(
    std::string_view host, std::string_view port, std::string_view target,
    http::verb method, std::string_view body = "",
    const std::vector<std::pair<std::string_view, std::string_view>>&
        headers = {});

const std::string TimestampToString(time_t timestamp);

std::string ForceOriginalQuality(const std::string& url);

::google::protobuf::Timestamp TimePointToProtobuf(
    const userver::storages::postgres::TimePointWithoutTz& time_point);

} // namespace utils
