#pragma once

#include <userver/storages/postgres/cluster.hpp>

#include <structs/game_info.hpp>
#include <structs/game_postgres.hpp>

#include <string_view>

namespace pg {

using entities::GameInfo;
using entities::GamePostgres;

class PostgresManager final
{
public:
    using GamesPostgres = std::vector<GamePostgres>;

    explicit PostgresManager(
        userver::storages::postgres::ClusterPtr pg_cluster);

    GamePostgres CreateGame(const GameInfo& kGameIgdbInfo) const;
    GamesPostgres FindGame(std::string_view query,
                           std::int32_t limit = 10) const;
    std::optional<GamePostgres> GetGameBySlug(std::string_view slug) const;
    std::optional<GamePostgres> GetGameByid(std::string_view postgresId) const;
    GamesPostgres GetGamesByGenre(std::string_view genre,
                                  std::int32_t limit) const;
    GamesPostgres GetTopRatedGames(std::int32_t limit) const;
    GamesPostgres GetUpcomingGames(std::int32_t limit) const;

    GamesPostgres GetAllGames(std::int32_t limit, std::int32_t offset) const;

    void UpdateGameRating(std::string_view game_id, double rating) const;
private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

} // namespace pg