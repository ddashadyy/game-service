#pragma once

// project headers
#include <managers/manager.hpp>
#include <structs/game_info.hpp>
// std
#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

// boost
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>

namespace igdb {

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

using tcp = net::ip::tcp;

class IGDBManager final : public IIGDBManager
{
public:
    IGDBManager();
    ~IGDBManager() = default;

    std::optional<std::string> GetTwitchToken() const;

    bool Authenticate();

    GamesInfo SearchGames(std::string_view query,
                          std::int32_t limit = 10) override;
    GamesInfo GetGameBySlug(std::string_view slug) override;
    GamesInfo GetGamesByGenre(std::string_view genre,
                              std::int32_t limit = 20) override;
    GamesInfo GetUpcomingGames(std::int32_t limit = 5) override;

private:
    GamesInfo ParseGamesResponse(std::string_view response) const;

    const std::string PerformHttpRequest(
        std::string_view host, std::string_view port, std::string_view target,
        http::verb method, std::string_view body = "",
        const std::vector<std::pair<std::string_view, std::string_view>>&
            headers = {}) const;

    mutable std::optional<std::string> cachedToken_;
    mutable std::chrono::system_clock::time_point tokenExpiry_;

    std::string clientId_;
    std::string clientSecret_;
    std::string accessToken_;

    static constexpr std::uint32_t kTokenExpiryBufferSeconds = 300;
};

} // namespace igdb
