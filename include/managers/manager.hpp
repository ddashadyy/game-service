#pragma once

// project headers
#include <structs/game_info.hpp>

// std
#include <cstdint>


namespace igdb {

class IIGDBManager 
{
public:
    using GamesInfo = std::vector<entities::GameInfo>;

    virtual ~IIGDBManager() = default;

    virtual GamesInfo SearchGames(std::string_view query, std::int32_t limit = 10) = 0;
    virtual GamesInfo GetGameBySlug(std::string_view slug) = 0;
    virtual GamesInfo GetGamesByGenre(std::string_view genre, std::int32_t limit = 20) = 0;
    virtual GamesInfo GetUpcomingGames(std::int32_t limit = 5) = 0;
};

} // namespace igdb
