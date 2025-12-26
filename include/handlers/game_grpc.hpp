#pragma once

#include <games/games_service.usrv.pb.hpp>
#include <userver/ugrpc/server/service_component_base.hpp>

#include <managers/igdb_manager.hpp>
#include <repository/postgres_manager.hpp>

namespace game_service {

class GameService final : public ::games::GameServiceBase
{
public:
    explicit GameService(std::string prefix, pg::PostgresManager manager,
                         igdb::IGDBManager igdb_manager);

    SearchGamesResult
    SearchGames(CallContext& context,
                ::games::SearchGamesRequest&& request) override;

    GetGameResult GetGame(CallContext& context,
                          ::games::GetGameRequest&& request) override;

    GetGamesByGenreResult
    GetGamesByGenre(CallContext& context,
                    ::games::GetGamesByGenreRequest&& request) override;

    GetTopRatedGamesResult
    GetTopRatedGames(CallContext& context,
                     ::games::GetDiscoveryRequest&& request) override;

    GetUpcomingGamesResult
    GetUpcomingGames(CallContext& context,
                     ::games::GetDiscoveryRequest&& request) override;

    ListGamesResult ListGames(CallContext& context,
                              ::games::ListGamesRequest&& request) override;

    SetRatingResult SetRating(CallContext& context,
                              ::games::RatingRequest&& request) override;

        private
        : void FillResponseWithPgData(::games::GamesListResponse& response,
                                      entities::GamePostgres&& pgData) const;
    void FillGameProto(::games::Game* game,
                       entities::GamePostgres&& pgData) const;

    std::string prefix_;
    pg::PostgresManager pg_manager_;
    igdb::IGDBManager igdb_manager_;
};

class GameServiceComponent final
    : public userver::ugrpc::server::ServiceComponentBase
{
public:
    static constexpr std::string_view kName = "game-service";

    GameServiceComponent(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context);

    static userver::yaml_config::Schema GetStaticConfigSchema();

private:
    GameService service_;
};

} // namespace game_service