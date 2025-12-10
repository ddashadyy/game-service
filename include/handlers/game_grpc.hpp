#pragma once

#include <userver/ugrpc/server/service_component_base.hpp>
#include <games_service.usrv.pb.hpp>

#include <repository/postgres_manager.hpp>
#include <managers/igdb_manager.hpp>

// std

namespace game_service {

class GameService final : public ::games::GameServiceBase 
{
public:

    explicit GameService(std::string prefix, pg::PostgresManager manager, igdb::IGDBManager igdb_manager);

    SearchGamesResult SearchGames(CallContext& context, ::games::SearchGamesRequest&& request) override;
    // GetGameResult GetGame(CallContext& context, ::games::GetGameRequest&& request) override;
    // GetGamesByGenreResult GetGamesByGenre(CallContext& context, ::games::GetGamesByGenreRequest&& request) override;
    // GetTopRatedGamesResult GetTopRatedGames(CallContext& context, ::games::GetDiscoveryRequest&& request) override;
    // GetUpcomingGamesResult GetUpcomingGames(CallContext& context, ::games::GetDiscoveryRequest&& request) override;

private:

    void FillResponseWithPgData(::games::GamesListResponse& response, entities::GamePostgres&& pgData) const;

    std::string prefix_;
    pg::PostgresManager pg_manager_;
    igdb::IGDBManager igdb_manager_;
};

class GameServiceComponent final : public userver::ugrpc::server::ServiceComponentBase
{
public: 
    static constexpr std::string_view kName = "game-service";

    GameServiceComponent(const userver::components::ComponentConfig& config, const userver::components::ComponentContext& context);

    static userver::yaml_config::Schema GetStaticConfigSchema();

private:
    GameService service_;
};


} // namespace game_service