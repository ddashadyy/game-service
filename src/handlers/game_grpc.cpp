#include <handlers/game_grpc.hpp>

#include <boost/uuid/uuid_io.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/database.hpp>
namespace {

template <typename Source, typename Destination>
void MoveToProto(Source& src, Destination* dst)
{
    dst->Reserve(src.size());
    for (auto& item : src)
        *dst->Add() = std::move(item);
}

} // namespace

game_service::GameService::GameService(std::string prefix,
                                       pg::PostgresManager manager,
                                       igdb::IGDBManager igdb_manager)
    : prefix_(std::move(prefix)), pg_manager_(std::move(manager)),
      igdb_manager_(std::move(igdb_manager))
{}

::games::GameServiceBase::SearchGamesResult
game_service::GameService::SearchGames(CallContext& context,
                                       ::games::SearchGamesRequest&& request)
{
    if (request.query().empty())
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                            "Query cannot be empty");

    ::games::GamesListResponse response;

    try
    {
        auto pg_games = pg_manager_.FindGame(request.query(), request.limit());

        if (!pg_games.empty())
        {
            response.mutable_games()->Reserve(pg_games.size());
            for (auto& game : pg_games)
                FillResponseWithPgData(response, std::move(game));

            return response;
        }

        const auto kIgdbResults =
            igdb_manager_.SearchGames(request.query(), request.limit());

        if (kIgdbResults.empty())
            return response;

        response.mutable_games()->Reserve(kIgdbResults.size());

        for (const auto& igdb_game : kIgdbResults)
        {
            auto saved_game = pg_manager_.CreateGame(igdb_game);
            FillResponseWithPgData(response, std::move(saved_game));
        }

        return response;
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR() << "Database query failed: " << ex.what();
        return grpc::Status(grpc::StatusCode::CANCELLED, "Games not found");
    }

    return grpc::Status(grpc::StatusCode::UNKNOWN, "Unexpected error");
}

::games::GameServiceBase::GetGameResult
game_service::GameService::GetGame(CallContext& context,
                                   ::games::GetGameRequest&& request)
{
    if (!request.has_slug())
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                            "Query cannot be empty");

    std::optional<entities::GamePostgres> pg_game;

    try
    {
        if (request.has_game_id())
        {
            pg_game = pg_manager_.GetGameByid(request.game_id());

            if (!pg_game)
                return grpc::Status(grpc::StatusCode::NOT_FOUND,
                                    "Game not found by postgres ID");
        }

        else if (request.has_slug())
        {
            const auto& kSlug = request.slug();
            if (kSlug.empty())
                return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                                    "Slug cannot be empty");

            pg_game = pg_manager_.GetGameBySlug(kSlug);

            if (pg_game)
            {
                auto igdb_game = igdb_manager_.GetGameBySlug(kSlug);

                pg_game = pg_manager_.CreateGame(std::move(igdb_game.at(0)));

                LOG_INFO() << "Game found in IGDB and saved to DB: " << kSlug;
            }

            else
                return grpc::Status(grpc::StatusCode::NOT_FOUND,
                                    "Game not found in DB or IGDB");
        }

        else
            return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                                "Request must have game_id or slug");

        ::games::GetGameResponse response;
        FillGameProto(response.mutable_game(), std::move(*pg_game));
        return response;
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR() << "Database query failed: " << ex.what();
        return grpc::Status(grpc::StatusCode::CANCELLED, "Games not found");
    }
}

::games::GameServiceBase::GetGamesByGenreResult
game_service::GameService::GetGamesByGenre(
    CallContext& context, ::games::GetGamesByGenreRequest&& request)
{
    if (request.genre_name().empty())
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                            "Genre name cannot be empty");

    const uint32_t kLimit = request.limit() > 0 ? request.limit() : 10;

    ::games::GamesListResponse response;

    try
    {
        auto pg_games =
            pg_manager_.GetGamesByGenre(request.genre_name(), kLimit);

        if (!pg_games.empty())
        {
            response.mutable_games()->Reserve(pg_games.size());
            for (auto& game : pg_games)
            {
                FillResponseWithPgData(response, std::move(game));
            }
            return response;
        }

        const auto kIgdbResults =
            igdb_manager_.GetGamesByGenre(request.genre_name(), kLimit);

        if (kIgdbResults.empty())
            return response;

        response.mutable_games()->Reserve(kIgdbResults.size());

        for (const auto& igdb_game : kIgdbResults)
        {
            auto saved_game = pg_manager_.CreateGame(igdb_game);
            FillResponseWithPgData(response, std::move(saved_game));
        }

        return response;
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR() << "GetGamesByGenre failed: " << ex.what();
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Internal database error");
    }
}

::games::GameServiceBase::GetTopRatedGamesResult
game_service::GameService::GetTopRatedGames(
    CallContext& context, ::games::GetDiscoveryRequest&& request)
{

    const uint32_t kLimit = request.limit() > 0 ? request.limit() : 10;

    ::games::GamesListResponse response;

    try
    {
        auto pg_games = pg_manager_.GetTopRatedGames(kLimit);

        if (!pg_games.empty())
        {
            response.mutable_games()->Reserve(pg_games.size());
            for (auto& game : pg_games)
            {
                FillResponseWithPgData(response, std::move(game));
            }
            return response;
        }

        const auto kIgdbResults = igdb_manager_.GetTopRatedGames(kLimit);

        if (kIgdbResults.empty())
            return response;

        response.mutable_games()->Reserve(kIgdbResults.size());

        for (const auto& igdb_game : kIgdbResults)
        {
            auto saved_game = pg_manager_.CreateGame(igdb_game);
            FillResponseWithPgData(response, std::move(saved_game));
        }

        return response;
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR() << "GetGamesByGenre failed: " << ex.what();
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Internal database error");
    }
}

::games::GameServiceBase::GetUpcomingGamesResult
game_service::GameService::GetUpcomingGames(
    CallContext& context, ::games::GetDiscoveryRequest&& request)
{
    const uint32_t kLimit = request.limit() > 0 ? request.limit() : 10;

    ::games::GamesListResponse response;

    try
    {
        auto pg_games = pg_manager_.GetUpcomingGames(kLimit);

        if (!pg_games.empty())
        {
            response.mutable_games()->Reserve(pg_games.size());
            for (auto& game : pg_games)
            {
                FillResponseWithPgData(response, std::move(game));
            }
            return response;
        }

        const auto kIgdbResults = igdb_manager_.GetUpcomingGames(kLimit);

        if (kIgdbResults.empty())
            return response;

        response.mutable_games()->Reserve(kIgdbResults.size());

        for (const auto& igdb_game : kIgdbResults)
        {
            auto saved_game = pg_manager_.CreateGame(igdb_game);
            FillResponseWithPgData(response, std::move(saved_game));
        }

        return response;
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR() << "GetGamesByGenre failed: " << ex.what();
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Internal database error");
    }
}

::games::GameServiceBase::ListGamesResult
game_service::GameService::ListGames(CallContext& context,
                                     ::games::ListGamesRequest&& request)
{
    const uint32_t kLimit = request.limit() > 0 ? request.limit() : 10;
    const uint32_t kOffset = request.offset() > 0 ? request.offset() : 10;

    ::games::GamesListResponse response;

    try
    {
        auto pg_games = pg_manager_.GetAllGames(kLimit, kOffset);

        if (pg_games.empty())
            return response;

        response.mutable_games()->Reserve(pg_games.size());
        for (auto& game : pg_games)
            FillResponseWithPgData(response, std::move(game));

        return response;
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR() << "ListGames failed: " << ex.what();
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Internal database error");
    }
}

::games::GameServiceBase::CalculateRatingResult
game_service::GameService::CalculateRating(
    CallContext& context, ::social::GetGameReviewsResponse&& request)
{
    if (request.reviews().empty())
        return google::protobuf::Empty{};

    if (request.game_id().empty())
        return google::protobuf::Empty{};

    double total_score = 0.0;
    std::uint32_t count = 0;
    std::string game_id = request.game_id();

    for (const auto& review : request.reviews())
    {
        total_score += review.rating();
        count++;
    }

    if (count == 0 || game_id.empty())
        return google::protobuf::Empty{};

    double average_rating = total_score / count;

    try
    {
        pg_manager_.UpdateGameRating(game_id, average_rating);

        LOG_INFO() << "Updated rating for game " << game_id << ": "
                   << average_rating << " (based on " << count << " reviews)";

        return grpc::Status(grpc::StatusCode::OK, "Updated");
    }
    catch (const std::runtime_error& ex)
    {
        LOG_ERROR() << "Invalid UUID in reviews: " << game_id;
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                            "Invalid game_id in reviews");
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR() << "Failed to update rating: " << ex.what();
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Database update failed");
    }
}

void game_service::GameService::FillResponseWithPgData(
    ::games::GamesListResponse& response, entities::GamePostgres&& pgData) const
{
    FillGameProto(response.add_games(), std::move(pgData));
}

void game_service::GameService::FillGameProto(
    ::games::Game* game, entities::GamePostgres&& pgData) const
{
    game->set_id(boost::uuids::to_string(pgData.id));
    game->set_igdb_id(std::move(pgData.igdb_id));

    game->set_name(std::move(pgData.name));
    game->set_slug(std::move(pgData.slug));
    game->set_summary(std::move(pgData.summary));

    game->set_igdb_rating(pgData.igdb_rating);
    game->set_hypes(pgData.hypes);

    game->set_first_release_date(std::move(pgData.firstReleaseDate));
    game->set_cover_url(std::move(pgData.coverUrl));

    MoveToProto(pgData.releaseDates, game->mutable_release_dates());
    MoveToProto(pgData.artworkUrls, game->mutable_artwork_urls());
    MoveToProto(pgData.screenshots, game->mutable_screenshots());
    MoveToProto(pgData.genres, game->mutable_genres());
    MoveToProto(pgData.themes, game->mutable_themes());
    MoveToProto(pgData.platforms, game->mutable_platforms());
}

game_service::GameServiceComponent::GameServiceComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::ugrpc::server::ServiceComponentBase(config, context),
      service_(
          config["game-prefix"].As<std::string>(),
          pg::PostgresManager(context
                                  .FindComponent<userver::components::Postgres>(
                                      "playhub-games-db")
                                  .GetCluster()),
          igdb::IGDBManager())
{
    RegisterService(service_);
}

userver::yaml_config::Schema
game_service::GameServiceComponent::GetStaticConfigSchema()
{
    return userver::yaml_config::MergeSchemas<
        userver::ugrpc::server::ServiceComponentBase>(
        R"(
            type: object
            description: Game gRPC service component
            additionalProperties: false
            properties:
                game-prefix:
                    type: string
                    description: game prefix
                database:
                    type: object
                    description: Database connection settings
                    additionalProperties: false  
                    properties:
                        host:
                            type: string
                            description: Hostname or IP of the database server
                        port:
                            type: integer
                            description: Port of the database server
                        user:
                            type: string
                            description: Database user
                        password:
                            type: string
                            description: Database password
        )");
}