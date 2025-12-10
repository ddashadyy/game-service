#include <handlers/game_grpc.hpp>

#include <boost/uuid/uuid_io.hpp>
#include <userver/storages/postgres/database.hpp>
#include <userver/storages/postgres/component.hpp>
namespace { 

template <typename Source, typename Destination>
void MoveToProto(Source& src, Destination* dst) 
{
    dst->Reserve(src.size());
    for (auto& item : src) 
        *dst->Add() = std::move(item);
}

}

game_service::GameService::GameService(std::string prefix, pg::PostgresManager manager, igdb::IGDBManager igdb_manager)
    : prefix_(std::move(prefix)), pg_manager_(std::move(manager)), igdb_manager_(std::move(igdb_manager)) {}


::games::GameServiceBase::SearchGamesResult game_service::GameService::SearchGames(
    CallContext& context, ::games::SearchGamesRequest&& request
) 
{
    if (request.query().empty()) 
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Query cannot be empty");
    

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

    
        const auto kIgdbResults = igdb_manager_.SearchGames(request.query(), request.limit());
        
        if (kIgdbResults.empty()) return response; 
        
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

void game_service::GameService::FillResponseWithPgData(
    ::games::GamesListResponse& response, entities::GamePostgres&& pgData
) const
{
    auto* game = response.add_games();
    
    game->set_id(boost::uuids::to_string(pgData.id));
    game->set_igdb_id(std::move(pgData.igdb_id));

    game->set_name(std::move(pgData.name));
    game->set_slug(std::move(pgData.slug));
    game->set_summary(std::move(pgData.summary));

    game->set_rating(pgData.rating);
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
    const userver::components::ComponentConfig& config, const userver::components::ComponentContext& context
) : userver::ugrpc::server::ServiceComponentBase(config, context), 
    service_(
        config["game-prefix"].As<std::string>(),
        pg::PostgresManager(
            context.FindComponent<userver::components::Postgres>("playhub-games-db").GetCluster()
        ),
        igdb::IGDBManager(config["env-file"].As<std::string>())
    ) { RegisterService(service_); }

userver::yaml_config::Schema game_service::GameServiceComponent::GetStaticConfigSchema()
{
    return userver::yaml_config::MergeSchemas<userver::ugrpc::server::ServiceComponentBase>(
        R"(
            type: object
            description: Game gRPC service component
            additionalProperties: false
            properties:
                game-prefix:
                    type: string
                    description: game prefix
                env-file:
                    type: string
                    description: Path to the .env file
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
        )"
    );
}