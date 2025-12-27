#pragma once

#include <games/games_service.usrv.pb.hpp>

#include <structs/game_info.hpp>
#include <structs/game_postgres.hpp>

#include <optional>

namespace pg {

using entities::GameInfo;
using entities::GamePostgres;

class IGameRepository
{
public:
    using GamesPostgres = std::vector<GamePostgres>;

    virtual ~IGameRepository() = default;

    virtual GamePostgres CreateGame(const GameInfo& kGameIgdbInfo) const = 0;
    virtual GamesPostgres FindGame(std::string_view query,
                                   std::int32_t limit = 10) const = 0;
    virtual std::optional<GamePostgres>
    GetGameBySlug(std::string_view slug) const = 0;
    virtual std::optional<GamePostgres>
    GetGameById(std::string_view postgresId) const = 0;
    virtual GamesPostgres GetGamesByGenre(std::string_view genre,
                                          std::int32_t limit) const = 0;
    virtual GamesPostgres GetTopRatedGames(std::int32_t limit) const = 0;
    virtual GamesPostgres GetUpcomingGames(std::int32_t limit) const = 0;

    virtual GamesPostgres GetAllGames(std::int32_t limit, std::int32_t offset,
                                      ::games::SortingType filter) const = 0;

    virtual void UpdateGameRating(std::string_view game_id,
                                  double rating) const = 0;
};

} // namespace pg