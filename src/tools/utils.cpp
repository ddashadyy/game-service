// project headers
#include <tools/utils.hpp>

// std
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <regex>

namespace fs = std::filesystem;
namespace ssl = net::ssl;

const std::unordered_map<std::string, std::string> utils::LoadEnv(std::string_view fileName)
{
    std::unordered_map<std::string, std::string> envVars;
    
    fs::path filePath(fileName);
    
    if (!fs::exists(filePath)) 
        throw std::runtime_error("File not found: " + filePath.string());
    
    if (!fs::is_regular_file(filePath)) 
        throw std::runtime_error("Not a regular file: " + filePath.string());
    
    std::ifstream file(filePath);
    
    if (!file.is_open()) 
        throw std::runtime_error("Cannot open file: " + filePath.string());
    

    std::string line;
    while (std::getline(file, line)) 
    {
        if (line.empty() || line[0] == '#') continue;

        auto equalsPos = line.find('=');
        if (equalsPos != std::string::npos) 
        {
            std::string key = line.substr(0, equalsPos);
            std::string value = line.substr(equalsPos + 1);

            envVars[key] = value;
        }
    }
    return envVars;
}

const std::string utils::PerformHttpRequest(
    std::string_view host, std::string_view port, std::string_view target,
    http::verb method, std::string_view body,
    const std::vector<std::pair<std::string_view, std::string_view>>& headers) 
{
    try 
    {
        net::io_context ioc;
        ssl::context ctx(ssl::context::tlsv12_client);
        ctx.set_default_verify_paths();
        
        ssl::stream<tcp::socket> stream(ioc, ctx);
        tcp::resolver resolver(ioc);
        
        const auto results = resolver.resolve(host, port);
        
        if (!SSL_set_tlsext_host_name(stream.native_handle(), host.data())) 
        {
            beast::error_code ec{
                static_cast<int>(::ERR_get_error()), 
                net::error::get_ssl_category()
            };
            throw beast::system_error{ec};
        }
        
        auto& lowest_layer = beast::get_lowest_layer(stream);
        net::connect(lowest_layer, results.begin(), results.end());
        stream.handshake(ssl::stream_base::client);
        
        http::request<http::string_body> req{method, target, 11};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, "IGDB-CPP-Client/1.0");
        
        for (const auto& [key, value] : headers) 
        {
            req.set(key, value);
        }
        
        if (!body.empty()) 
        {
            req.set(http::field::content_type, "application/json");
            req.body() = body;
            req.prepare_payload();
        }
        
        http::write(stream, req);
        
        beast::flat_buffer buffer;
        http::response<http::dynamic_body> res;
        http::read(stream, buffer, res);
        
        beast::error_code ec;
        stream.shutdown(ec);
        
        if (ec == net::error::eof || ec == ssl::error::stream_truncated) 
            ec = {};
        if (ec) 
            throw beast::system_error{ec};
        
        return beast::buffers_to_string(res.body().data());
        
    } 
    catch (const std::exception& e) 
    {
        std::cerr << "HTTP Request Error: " << e.what() << std::endl;
        return "";
    }
}

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
    return std::regex_replace(url, size_pattern, "/t_original/");
}