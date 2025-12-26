#include <gmock/gmock.h>
#include <userver/ugrpc/tests/service.hpp>
#include <userver/ugrpc/tests/service_fixtures.hpp>

#include <handlers/game_grpc.hpp>
#include <managers/igdb_manager.hpp>
#include <repository/repository.hpp>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <games/games_client.usrv.pb.hpp>

namespace test {

using OptionalGamePostgres = std::optional<entities::GamePostgres>;

class MockGameRepository : public pg::IGameRepository
{
public:
    MOCK_METHOD(entities::GamePostgres, CreateGame,
                (const entities::GameInfo& kGameIgdbInfo), (const, override));
    MOCK_METHOD(GamesPostgres, FindGame,
                (std::string_view query, std::int32_t limit),
                (const, override));
    MOCK_METHOD(OptionalGamePostgres, GetGameBySlug, (std::string_view slug),
                (const, override));
    MOCK_METHOD(OptionalGamePostgres, GetGameById,
                (std::string_view postgresId), (const, override));
    MOCK_METHOD(GamesPostgres, GetGamesByGenre,
                (std::string_view genre, std::int32_t limit),
                (const, override));
    MOCK_METHOD(GamesPostgres, GetTopRatedGames, (std::int32_t limit),
                (const, override));
    MOCK_METHOD(GamesPostgres, GetUpcomingGames, (std::int32_t limit),
                (const, override));
    MOCK_METHOD(GamesPostgres, GetAllGames,
                (std::int32_t limit, std::int32_t offset), (const, override));
    MOCK_METHOD(void, UpdateGameRating,
                (std::string_view game_id, double rating), (const, override));
};

class MockIGDBManager : public igdb::IIGDBManager
{
public:
    using GamesInfo = igdb::IIGDBManager::GamesInfo;

    MOCK_METHOD(GamesInfo, SearchGames,
                (std::string_view query, std::int32_t limit), (override));

    MOCK_METHOD(GamesInfo, GetGameBySlug, (std::string_view slug), (override));

    MOCK_METHOD(GamesInfo, GetGamesByGenre,
                (std::string_view genre, std::int32_t limit), (override));

    MOCK_METHOD(GamesInfo, GetUpcomingGames, (std::int32_t limit), (override));
};

class GameServiceTest : public userver::ugrpc::tests::ServiceFixtureBase
{
protected:
    GameServiceTest()
        : prefix_("game-prefix"), mock_igdb_{},
          service_(prefix_, mock_repo_, mock_igdb_)
    {
        RegisterService(service_);
        StartServer();
    }

    MockGameRepository mock_repo_{};
    std::string prefix_{ "game-prefix" };
    MockIGDBManager mock_igdb_{};
    game_service::GameService service_;
};


entities::GamePostgres CreateFakePostgresGame(std::string_view name) {
    entities::GamePostgres game;
    game.id = boost::uuids::random_generator()();
    game.name = name;
    game.slug = name; 
    return game;
}

UTEST_F(GameServiceTest, SearchGamesFindsInIgdb)
{
    ::games::SearchGamesRequest request;
    request.set_query("God of War");
    request.set_limit(5);

    EXPECT_CALL(mock_repo_, FindGame(testing::_, testing::_))
        .WillOnce(testing::Return(std::vector<entities::GamePostgres>{}));

    std::vector<entities::GameInfo> fake_igdb_response;
    entities::GameInfo game_info;
    game_info.name = "God of War";
    game_info.slug = "god-of-war";
    game_info.id = "12345";

    fake_igdb_response.push_back(game_info);

    EXPECT_CALL(mock_igdb_, SearchGames(testing::HasSubstr("God of War"), testing::_))
        .WillOnce(testing::Return(fake_igdb_response));

    entities::GamePostgres fake_saved_game;
    fake_saved_game.id = boost::uuids::random_generator()();
    fake_saved_game.name = "God of War";
    
    EXPECT_CALL(mock_repo_, CreateGame(testing::_))
        .WillOnce(testing::Return(fake_saved_game));

    auto client = MakeClient<::games::GameServiceClient>();
    auto response = client.SearchGames(request);

    EXPECT_EQ(response.games_size(), 1);
    EXPECT_EQ(response.games(0).name(), "God of War");
}

UTEST_F(GameServiceTest, GetGameByIdFound)
{
    // Подготовка данных
    auto fake_game = CreateFakePostgresGame("Cyberpunk 2077");
    std::string str_id = boost::uuids::to_string(fake_game.id);

    ::games::GetGameRequest request;
    request.set_game_id(str_id);

    // Настройка мока: БД находит игру по ID
    EXPECT_CALL(mock_repo_, GetGameById(testing::Eq(str_id)))
        .WillOnce(testing::Return(std::optional<entities::GamePostgres>{fake_game}));

    // Вызов
    auto client = MakeClient<::games::GameServiceClient>();
    auto response = client.GetGame(request);

    // Проверка
    EXPECT_EQ(response.game().name(), "Cyberpunk 2077");
    EXPECT_EQ(response.game().id(), str_id);
}

UTEST_F(GameServiceTest, GetGameBySlugFound)
{
    auto fake_game = CreateFakePostgresGame("Elden Ring");
    fake_game.slug = "elden-ring";

    ::games::GetGameRequest request;
    request.set_slug("elden-ring");

    // Настройка мока: БД находит игру по Slug
    EXPECT_CALL(mock_repo_, GetGameBySlug(testing::Eq("elden-ring")))
        .WillOnce(testing::Return(std::optional<entities::GamePostgres>{fake_game}));

    auto client = MakeClient<::games::GameServiceClient>();
    auto response = client.GetGame(request);

    EXPECT_EQ(response.game().name(), "Elden Ring");
}


} // namespace test
