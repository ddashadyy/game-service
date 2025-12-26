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
    "  igdb_id, name, slug, summary, igdb_rating, hypes, "
    "  first_release_date, release_dates, cover_url, artwork_urls, "
    "  screenshots, "
    "  genres, themes, platforms, "
    "  playhub_rating"
    ") "
    "VALUES ("
    "  $1, $2, $3, $4, $5, $6, $7, "
    "  $8, $9, $10, $11, $12, "
    "  $13, $14, "
    "  0.0"
    ") "
    "ON CONFLICT (id) DO UPDATE SET "
    "  igdb_id = EXCLUDED.igdb_id, "
    "  name = EXCLUDED.name, "
    "  slug = EXCLUDED.slug, "
    "  summary = EXCLUDED.summary, "
    "  igdb_rating = EXCLUDED.igdb_rating, "
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
    "  id, igdb_id, name, slug, summary, igdb_rating, playhub_rating, hypes, "
    "  first_release_date, release_dates, cover_url, artwork_urls, "
    "  screenshots, "
    "  genres, themes, platforms, created_at, updated_at"
};

const userver::storages::postgres::Query kFindGame{
    "SELECT "
    "  id, igdb_id, name, slug, summary, igdb_rating, playhub_rating, hypes, "
    "  first_release_date, release_dates, cover_url, artwork_urls, "
    "screenshots, "
    "  genres, themes, platforms, created_at, updated_at "
    "FROM playhub.games "
    "WHERE name ILIKE $1 "
    "LIMIT $2"
};

const userver::storages::postgres::Query kGetGameBySlug{
    "SELECT "
    "  id, igdb_id, name, slug, summary, igdb_rating, playhub_rating, hypes, "
    "  first_release_date, release_dates, cover_url, artwork_urls, "
    "screenshots, "
    "  genres, themes, platforms, created_at, updated_at "
    "FROM playhub.games "
    "WHERE slug = $1"
};

const userver::storages::postgres::Query kGetGameByPostgresId{
    "SELECT "
    "  id, igdb_id, name, slug, summary, igdb_rating, playhub_rating, hypes, "
    "  first_release_date, release_dates, cover_url, artwork_urls, "
    "screenshots, "
    "  genres, themes, platforms, created_at, updated_at "
    "FROM playhub.games "
    "WHERE id = $1::uuid"
};

const userver::storages::postgres::Query kGetGamesByGenre{
    "SELECT "
    "  id, igdb_id, name, slug, summary, igdb_rating, playhub_rating, hypes, "
    "  first_release_date, release_dates, cover_url, artwork_urls, "
    "screenshots, "
    "  genres, themes, platforms, created_at, updated_at "
    "FROM playhub.games "
    "WHERE $1 = ANY(genres) "
    "ORDER BY igdb_rating DESC NULLS LAST "
    "LIMIT $2"
};

const userver::storages::postgres::Query kGetTopRatedGames{
    "SELECT "
    "  id, igdb_id, name, slug, summary, igdb_rating, playhub_rating, hypes, "
    "  first_release_date, release_dates, cover_url, artwork_urls, "
    "screenshots, "
    "  genres, themes, platforms, created_at, updated_at "
    "FROM playhub.games "
    "WHERE playhub_rating IS NOT NULL "      
    "ORDER BY playhub_rating DESC NULLS LAST " 
    "LIMIT $1"
};

const userver::storages::postgres::Query kGetUpcomingGames{
    "SELECT "
    "  id, igdb_id, name, slug, summary, igdb_rating, playhub_rating, hypes, "
    "  first_release_date, release_dates, cover_url, artwork_urls, "
    "screenshots, "
    "  genres, themes, platforms, created_at, updated_at "
    "FROM playhub.games "
    "WHERE first_release_date IS NOT NULL "
    "  AND CAST(first_release_date AS TIMESTAMP) > NOW() "
    "ORDER BY CAST(first_release_date AS TIMESTAMP) ASC "
    "LIMIT $1"
};

const userver::storages::postgres::Query kGetAllGames{
    "SELECT "
    "  id, igdb_id, name, slug, summary, igdb_rating, playhub_rating, hypes, "
    "  first_release_date, release_dates, cover_url, artwork_urls, "
    "screenshots, "
    "  genres, themes, platforms, created_at, updated_at "
    "FROM playhub.games "
    "ORDER BY created_at DESC "
    "LIMIT $1 OFFSET $2"
};

const userver::storages::postgres::Query kUpdateGameRating{
    "UPDATE playhub.games "
    "SET playhub_rating = $2, updated_at = NOW() "
    "WHERE id = $1::uuid"
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
            kGameIgdbInfo.summary, kGameIgdbInfo.igdb_rating,
            kGameIgdbInfo.hypes, kGameIgdbInfo.firstReleaseDate,
            kGameIgdbInfo.releaseDates, kGameIgdbInfo.coverUrl,
            kGameIgdbInfo.artworkUrls, kGameIgdbInfo.screenshots,
            kGameIgdbInfo.genres, kGameIgdbInfo.themes,
            kGameIgdbInfo.platforms);

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

std::optional<GamePostgres>
PostgresManager::GetGameBySlug(std::string_view slug) const
{
    try
    {
        const auto kResult = pg_cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            pg::kGetGameBySlug, slug);

        return kResult.AsOptionalSingleRow<entities::GamePostgres>(
            userver::storages::postgres::kRowTag);
    }
    catch (const std::exception& e)
    {
        LOG_ERROR() << "Error getting game by slug: " << e.what() << '\n';
    }

    return {};
}

std::optional<GamePostgres>
PostgresManager::GetGameByid(std::string_view postgresId) const
{
    try
    {
        const auto kResult = pg_cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            pg::kGetGameByPostgresId, postgresId);

        return kResult.AsOptionalSingleRow<entities::GamePostgres>(
            userver::storages::postgres::kRowTag);
    }
    catch (const std::exception& e)
    {
        LOG_ERROR() << "Error getting game by uuid: " << e.what() << '\n';
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

PostgresManager::GamesPostgres
PostgresManager::GetAllGames(std::int32_t limit, std::int32_t offset) const
{
    try
    {
        const auto kResult = pg_cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            pg::kGetAllGames, limit, offset);

        return kResult.AsContainer<GamesPostgres>(
            userver::storages::postgres::kRowTag);
    }
    catch (const std::exception& e)
    {
        LOG_ERROR() << "Error getting upcoming games: " << e.what() << '\n';
    }
    return {};
}

void PostgresManager::UpdateGameRating(std::string_view game_id,
                                       double rating) const
{
    try
    {
        pg_cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            kUpdateGameRating, game_id, rating);
    }
    catch (const std::exception& e)
    {
        LOG_ERROR() << "Error getting upcoming games: " << e.what() << '\n';
    }
}

} // namespace pg