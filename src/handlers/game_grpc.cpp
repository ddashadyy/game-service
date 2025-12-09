#include <handlers/game_grpc.hpp>


game_service::GameService::GameService(std::string prefix, pg::PostgresManager manager, igdb::IGDBManager igdb_manager)
    : prefix_(std::move(prefix)), pg_manager_(std::move(manager)), igdb_manager_(igdb_manager) {}


::games::GameServiceBase::SearchGamesResult game_service::GameService::SearchGames(
    CallContext& context, ::games::SearchGamesRequest&& request
) 
{
    if (request.query().empty())
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Query for searching is required");

    const auto kPgResult = pg_manager_.FindGame(request.query(), request.limit());
    
    if (kPgResult.id.is_nil()) 
    {
        // если не нашли в бд, лезем в igdb
    }
}