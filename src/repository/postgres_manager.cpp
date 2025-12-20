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

namespace pg {

const userver::storages::postgres::Query kInsertGame{
    "INSERT INTO playhub.games ("
    "  igdb_id, name, slug, summary, rating, hypes, "
    "  first_release_date, release_dates, cover_url, artwork_urls, "
    "screenshots, "
    "  genres, themes, platforms"
    ") "
    "VALUES ("
    "  $1, $2, $3, $4, $5, $6, $7, "
    "  $8, $9, $10, $11, $12, "
    "  $13, $14"
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
    "  updated_at = NOW() "
    "RETURNING "
    "  id, igdb_id, name, slug, summary, rating, hypes, "
    "  first_release_date, release_dates, cover_url, artwork_urls, "
    "screenshots, "
    "  genres, themes, platforms, created_at, updated_at"
};

const userver::storages::postgres::Query kFindGame{
    "SELECT "
    "  id, igdb_id, name, slug, summary, rating, hypes, "
    "  first_release_date, release_dates, cover_url, artwork_urls, "
    "screenshots, "
    "  genres, themes, platforms, created_at, updated_at "
    "FROM playhub.games "
    "WHERE name ILIKE $1 "
    "LIMIT $2"
};

const userver::storages::postgres::Query kGetGameBySlug{
    "SELECT "
    "  id, igdb_id, name, slug, summary, rating, hypes, "
    "  first_release_date, release_dates, cover_url, artwork_urls, "
    "screenshots, "
    "  genres, themes, platforms, created_at, updated_at "
    "FROM playhub.games "
    "WHERE slug = $1"
};

const userver::storages::postgres::Query kGetGamesByGenre{
    "SELECT "
    "  id, igdb_id, name, slug, summary, rating, hypes, "
    "  first_release_date, release_dates, cover_url, artwork_urls, "
    "screenshots, "
    "  genres, themes, platforms, created_at, updated_at "
    "FROM playhub.games "
    "WHERE $1 = ANY(genres) " 
    "ORDER BY rating DESC NULLS LAST "
    "LIMIT $2"
};

const userver::storages::postgres::Query kGetTopRatedGames{
    "SELECT "
    "  id, igdb_id, name, slug, summary, rating, hypes, "
    "  first_release_date, release_dates, cover_url, artwork_urls, "
    "screenshots, "
    "  genres, themes, platforms, created_at, updated_at "
    "FROM playhub.games "
    "WHERE rating >= 75 "
    "ORDER BY rating DESC NULLS LAST "
    "LIMIT $1"
};

const userver::storages::postgres::Query kGetUpcomingGames{
    "SELECT "
    "  id, igdb_id, name, slug, summary, rating, hypes, "
    "  first_release_date, release_dates, cover_url, artwork_urls, "
    "screenshots, "
    "  genres, themes, platforms, created_at, updated_at "
    "FROM playhub.games "
    "WHERE first_release_date IS NOT NULL "
    "  AND CAST(first_release_date AS TIMESTAMP) > NOW() "
    "ORDER BY CAST(first_release_date AS TIMESTAMP) ASC "
    "LIMIT $1"
};

PostgresManager::PostgresManager(
    userver::storages::postgres::ClusterPtr pg_cluster)
    : pg_cluster_(std::move(pg_cluster))
{}

entities::GamePostgres
PostgresManager::CreateGame(const entities::GameInfo& kGameIgdbInfo) const
{
    try
    {
        const auto kResult = pg_cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster, kInsertGame,
            kGameIgdbInfo.id, kGameIgdbInfo.name, kGameIgdbInfo.slug,
            kGameIgdbInfo.summary, kGameIgdbInfo.rating, kGameIgdbInfo.hypes,
            kGameIgdbInfo.firstReleaseDate, kGameIgdbInfo.releaseDates,
            kGameIgdbInfo.coverUrl, kGameIgdbInfo.artworkUrls,
            kGameIgdbInfo.screenshots, kGameIgdbInfo.genres,
            kGameIgdbInfo.themes, kGameIgdbInfo.platforms);

        return kResult.AsSingleRow<entities::GamePostgres>(
            userver::storages::postgres::kRowTag);
    }
    catch (const std::exception& e)
    {
        LOG_ERROR() << e.what() << '\n';
    }

    return {};
}

PostgresManager::GamesPostgres
PostgresManager::FindGame(std::string_view query, std::int32_t limit) const
{
    try
    {
        const auto kResult = pg_cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster, kFindGame,
            query, limit);

        return kResult.AsContainer<GamesPostgres>(
            userver::storages::postgres::kRowTag);
    }
    catch (const std::exception& e)
    {
        LOG_ERROR() << e.what() << '\n';
    }

    return {};
}

PostgresManager::GamesPostgres
PostgresManager::GetGameBySlug(std::string_view slug) const
{
    try
    {
        const auto kResult = pg_cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            pg::kGetGameBySlug, slug);

        return kResult.AsContainer<GamesPostgres>(
            userver::storages::postgres::kRowTag);
    }
    catch (const std::exception& e)
    {
        LOG_ERROR() << "Error getting game by slug: " << e.what() << '\n';
    }

    return {};
}

PostgresManager::GamesPostgres
PostgresManager::GetGamesByGenre(std::string_view genre,
                                 std::int32_t limit) const
{
    try
    {
        const auto kResult = pg_cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            pg::kGetGamesByGenre, genre, limit);

        return kResult.AsContainer<GamesPostgres>(
            userver::storages::postgres::kRowTag);
    }
    catch (const std::exception& e)
    {
        LOG_ERROR() << "Error getting games by genre: " << e.what() << '\n';
    }
    return {};
}

PostgresManager::GamesPostgres
PostgresManager::GetTopRatedGames(std::int32_t limit) const
{
    try
    {
        const auto kResult = pg_cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            pg::kGetTopRatedGames, limit);

        return kResult.AsContainer<GamesPostgres>(
            userver::storages::postgres::kRowTag);
    }
    catch (const std::exception& e)
    {
        LOG_ERROR() << "Error getting top rated games: " << e.what() << '\n';
    }
    return {};
}

PostgresManager::GamesPostgres
PostgresManager::GetUpcomingGames(std::int32_t limit) const
{
    try
    {
        const auto kResult = pg_cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            pg::kGetUpcomingGames, limit);

        return kResult.AsContainer<GamesPostgres>(
            userver::storages::postgres::kRowTag);
    }
    catch (const std::exception& e)
    {
        LOG_ERROR() << "Error getting upcoming games: " << e.what() << '\n';
    }
    return {};
}

} // namespace pg