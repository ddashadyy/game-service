#pragma once

// project headers
#include <structs/game_info.hpp>

// std
#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

namespace igdb {

class IGDBManager final
{
public:
    using GamesInfo = std::vector<entities::GameInfo>;

    IGDBManager();
    ~IGDBManager() = default;

    std::optional<std::string> GetTwitchToken() const;

    bool Authenticate();

    GamesInfo SearchGames(std::string_view query, std::int32_t limit = 10);
    GamesInfo GetGameBySlug(std::string_view slug);
    GamesInfo GetGamesByGenre(std::string_view genre, std::int32_t limit = 20);
    GamesInfo GetUpcomingGames(std::int32_t limit = 5);

private:
    GamesInfo ParseGamesResponse(std::string_view response) const;

    mutable std::optional<std::string> cachedToken_;
    mutable std::chrono::system_clock::time_point tokenExpiry_;

    std::string clientId_;
    std::string clientSecret_;
    std::string accessToken_;

    static constexpr std::uint32_t kTokenExpiryBufferSeconds = 300;
};

} // namespace igdb
