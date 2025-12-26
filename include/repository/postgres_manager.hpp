#pragma once

#include <userver/storages/postgres/cluster.hpp>

#include <structs/game_info.hpp>
#include <structs/game_postgres.hpp>

#include <repository/repository.hpp>

#include <string_view>

namespace pg {

using entities::GameInfo;
using entities::GamePostgres;

class PostgresManager final : public IGameRepository
{
public:

    explicit PostgresManager(
        userver::storages::postgres::ClusterPtr pg_cluster);

    GamePostgres CreateGame(const GameInfo& kGameIgdbInfo) const override;
    GamesPostgres FindGame(std::string_view query,
                           std::int32_t limit = 10) const override;
    std::optional<GamePostgres> GetGameBySlug(std::string_view slug) const override;
    std::optional<GamePostgres> GetGameById(std::string_view postgresId) const override;
    GamesPostgres GetGamesByGenre(std::string_view genre,
                                  std::int32_t limit) const override;
    GamesPostgres GetTopRatedGames(std::int32_t limit) const override;
    GamesPostgres GetUpcomingGames(std::int32_t limit) const override;

    GamesPostgres GetAllGames(std::int32_t limit, std::int32_t offset) const override;

    void UpdateGameRating(std::string_view game_id, double rating) const override;
private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

} // namespace pg