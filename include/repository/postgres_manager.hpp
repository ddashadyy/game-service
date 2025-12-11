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
    GamesPostgres GetGameBySlug(std::string_view slug);
    GamesPostgres GetGameByid(std::string_view postgresId);

private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

} // namespace pg