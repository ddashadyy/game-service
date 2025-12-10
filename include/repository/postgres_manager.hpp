#pragma once


#include <userver/storages/postgres/cluster.hpp>

#include <structs/game_postgres.hpp>
#include <structs/game_info.hpp>

#include <string_view>

namespace pg {

using entities::GamePostgres;
using entities::GameInfo;

class PostgresManager final 
{
public:

    using GamesPostgres = std::vector<GamePostgres>;

    explicit PostgresManager(userver::storages::postgres::ClusterPtr pg_cluster);

    GamePostgres CreateGame(const GameInfo& kGameIgdbInfo) const;
    GamesPostgres FindGame(std::string_view query, std::int32_t limit = 10) const;

private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

}