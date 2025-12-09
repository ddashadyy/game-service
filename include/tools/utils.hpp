#pragma once


// std 
#include <unordered_map>
#include <string>
#include <string_view>

// boost
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

using tcp = net::ip::tcp;


namespace utils {

const std::unordered_map<std::string, std::string> LoadEnv(std::string_view fileName);

const std::string PerformHttpRequest(
    std::string_view host,
    std::string_view port,
    std::string_view target,
    http::verb method,
    std::string_view body = "",
    const std::vector<std::pair<std::string_view, std::string_view>>& headers = {}
);

const std::string TimestampToString(time_t timestamp);

std::string ForceOriginalQuality(const std::string& url);

} // namespace utils
