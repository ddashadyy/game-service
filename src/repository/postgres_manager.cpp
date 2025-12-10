#include <repository/postgres_manager.hpp>
#include <userver/storages/postgres/cluster_types.hpp>


template <>
struct userver::storages::postgres::io::CppToUserPg<boost::uuids::uuid> 
{
    static constexpr DBTypeName postgres_name = "uuid";
};

template <>
struct userver::storages::postgres::io::CppToUserPg<std::int32_t> 
{
    static constexpr DBTypeName postgres_name = "integer";
};


namespace {
    const userver::storages::postgres::Query kInsertGame {
        "INSERT INTO playhub.games ("
        "  igdb_id, name, slug, summary, rating, hypes, "
        "  first_release_date, release_dates, cover_url, artwork_urls, screenshots, "
        "  genres, themes, platforms, created_at, updated_at"
        ") "
        "VALUES ("
        "  $1, $2, $3, $4, $5, $6, $7, "
        "  $8, $9, $10, $11, $12, "
        "  $13, $14, $15, $16"
        ") "
        "ON CONFLICT (id) DO UPDATE SET "
        "  igdb_id = EXCLUDED.igdb_id, "
        "  name = EXCLUDED.name, "
        "  slug = EXCLUDED.slug, "
        "  summary = EXCLUDED.summary, "
        "  rating = EXCLUDED.rating, "
        "  hypes = EXCLUDED.hypes, "
        "  first_release_date = EXCLUDED.first_release_date, "
        "  release_dates = EXCLUDED.release_dates, "
        "  cover_url = EXCLUDED.cover_url, "
        "  artwork_urls = EXCLUDED.artwork_urls, "
        "  screenshots = EXCLUDED.screenshots, "
        "  genres = EXCLUDED.genres, "
        "  themes = EXCLUDED.themes, "
        "  platforms = EXCLUDED.platforms, "
        "  updated_at = EXCLUDED.updated_at " 
        "RETURNING "
        "  id, igdb_id, name, slug, summary, rating, hypes, "
        "  first_release_date, release_dates, cover_url, artwork_urls, screenshots, "
        "  genres, themes, platforms, created_at, updated_at"
    };

    const userver::storages::postgres::Query kFindGame {
        "SELECT "
        "  id, igdb_id, name, slug, summary, rating, hypes, "
        "  first_release_date, release_dates, cover_url, artwork_urls, screenshots, "
        "  genres, themes, platforms, created_at, updated_at "
        "FROM playhub.games "
        "WHERE name ILIKE $1"
    };
}


pg::PostgresManager::PostgresManager(userver::storages::postgres::ClusterPtr pg_cluster)
    : pg_cluster_(std::move(pg_cluster)) {}

entities::GamePostgres pg::PostgresManager::CreateGame(const entities::GameInfo& kGameIgdbInfo) const
{
    try
    {
        const auto kResult = pg_cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            kInsertGame,
            kGameIgdbInfo.id, kGameIgdbInfo.name,
            kGameIgdbInfo.slug, kGameIgdbInfo.summary,
            kGameIgdbInfo.rating, kGameIgdbInfo.hypes,
            kGameIgdbInfo.firstReleaseDate, kGameIgdbInfo.releaseDates,
            kGameIgdbInfo.coverUrl, kGameIgdbInfo.artworkUrls,
            kGameIgdbInfo.screenshots, kGameIgdbInfo.genres,
            kGameIgdbInfo.themes, kGameIgdbInfo.platforms
        );

        return kResult.AsSingleRow<entities::GamePostgres>(userver::storages::postgres::kRowTag);
    }
    catch(const std::exception& e)
    {
        LOG_ERROR() << e.what() << '\n';
    }

    return {};
}

pg::PostgresManager::GamesPostgres pg::PostgresManager::FindGame(std::string_view query, std::int32_t limit) const
{
    try 
    {
        const auto kResult = pg_cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            kFindGame,
            query,
            limit
        );

        return kResult.AsContainer<GamesPostgres>(userver::storages::postgres::kRowTag);
    }
    catch(const std::exception& e)
    {
        LOG_ERROR() << e.what() << '\n';
    }

    return {};
}