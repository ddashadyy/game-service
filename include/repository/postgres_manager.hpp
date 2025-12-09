#pragma once


#include <userver/storages/postgres/cluster.hpp>

#include <structs/game_postgres.hpp>

#include <string_view>

namespace pg {

class PostgresManager final 
{
public:
    explicit PostgresManager(userver::storages::postgres::ClusterPtr pg_cluster);

    entities::GamePostgres CreateGame(
        std::string_view igdb_id,                     std::string_view name,
        std::string_view slug,                        std::string_view summary,
        double rating,                                std::uint32_t hypes,
        std::string_view firstReleaseDate,            const std::vector<std::string>& kReleaseDates,
        std::string_view coverUrl,                    const std::vector<std::string>& kArtworkUrls,
        const std::vector<std::string>& kScreenshots, const std::vector<std::string>& kGenres,
        const std::vector<std::string>& kThemes,      const std::vector<std::string>& kPlatforms
    ) const;

    entities::GamePostgres FindGame(std::string_view query, std::uint32_t limit) const;

private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

}