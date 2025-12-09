#pragma once


#include <service_service.usrv.pb.hpp>
#include <repository/postgres_manager.hpp>


namespace game_service {

class GameService final : public ::games::GameServiceBase 
{
public:
    explicit GameService(std::string prefix, pg::PostgresManager manager);

    SearchGamesResult SearchGames(CallContext& context, ::games::SearchGamesRequest&& request) override;
    GetGameResult GetGame(CallContext& context, ::games::GetGameRequest&& request) override;
    GetGamesByGenreResult GetGamesByGenre(CallContext& context, ::games::GetGamesByGenreRequest&& request) override;
    GetTopRatedGamesResult GetTopRatedGames(CallContext& context, ::games::GetDiscoveryRequest&& request) override;
    GetUpcomingGamesResult GetUpcomingGames(CallContext& context, ::games::GetDiscoveryRequest&& request) override;

private:
    std::string prefix_;
    pg::PostgresManager manager_;
};


} // namespace game_service