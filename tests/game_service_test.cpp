#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <userver/ugrpc/client/exceptions.hpp>
#include <userver/ugrpc/tests/service_fixtures.hpp>

#include <games/games_client.usrv.pb.hpp>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <handlers/game_grpc.hpp>
#include <managers/manager.hpp>
#include <repository/repository.hpp>
#include <structs/game_info.hpp>
#include <structs/game_postgres.hpp>

using namespace testing;

namespace test {

class MockGameRepository : public pg::IGameRepository
{
public:
    MOCK_METHOD(entities::GamePostgres, CreateGame, (const entities::GameInfo&),
                (const, override));
    MOCK_METHOD(std::vector<entities::GamePostgres>, FindGame,
                (std::string_view, std::int32_t), (const, override));
    MOCK_METHOD(std::optional<entities::GamePostgres>, GetGameBySlug,
                (std::string_view), (const, override));
    MOCK_METHOD(std::optional<entities::GamePostgres>, GetGameById,
                (std::string_view), (const, override));
    MOCK_METHOD(std::vector<entities::GamePostgres>, GetGamesByGenre,
                (std::string_view, std::int32_t), (const, override));
    MOCK_METHOD(std::vector<entities::GamePostgres>, GetTopRatedGames,
                (std::int32_t), (const, override));
    MOCK_METHOD(std::vector<entities::GamePostgres>, GetUpcomingGames,
                (std::int32_t), (const, override));
    MOCK_METHOD(std::vector<entities::GamePostgres>, GetAllGames,
                (std::int32_t, std::int32_t, ::games::SortingType),
                (const, override));
    MOCK_METHOD(void, UpdateGameRating, (std::string_view, double),
                (const, override));
};

class MockIGDBManager : public igdb::IIGDBManager
{
public:
    MOCK_METHOD(std::vector<entities::GameInfo>, SearchGames,
                (std::string_view, std::int32_t), (override));
    MOCK_METHOD(std::vector<entities::GameInfo>, GetGameBySlug,
                (std::string_view), (override));
    MOCK_METHOD(std::vector<entities::GameInfo>, GetGamesByGenre,
                (std::string_view, std::int32_t), (override));
    MOCK_METHOD(std::vector<entities::GameInfo>, GetUpcomingGames,
                (std::int32_t), (override));
};

entities::GamePostgres CreateFakePostgresGame(std::string_view name)
{
    entities::GamePostgres game;
    game.id = boost::uuids::random_generator()();
    game.name = name;
    game.slug = name;
    game.igdb_id = "123";
    game.hypes = 100;
    game.playhub_rating = 8.5;
    return game;
}

} // namespace test

class GameServiceTest : public userver::ugrpc::tests::ServiceFixtureBase
{
protected:
    std::string prefix_{ "game-prefix" };

    test::MockGameRepository mock_repo_;
    test::MockIGDBManager mock_igdb_;

    game_service::GameService service_;

    GameServiceTest() : service_(prefix_, mock_repo_, mock_igdb_)
    {
        RegisterService(service_);
        StartServer();
    }
};

// --- 1. SEARCH GAMES ---
UTEST_F(GameServiceTest, SearchGames_FoundInDb)
{
    ::games::SearchGamesRequest request;
    request.set_query("Witcher");
    request.set_limit(5);

    std::vector<entities::GamePostgres> db_games;
    db_games.push_back(test::CreateFakePostgresGame("The Witcher 3"));

    EXPECT_CALL(mock_repo_, FindGame(testing::Eq("Witcher"), testing::Eq(5)))
        .WillOnce(testing::Return(db_games));

    EXPECT_CALL(mock_igdb_, SearchGames(_, _)).Times(0);

    auto client = MakeClient<::games::GameServiceClient>();
    auto response = client.SearchGames(request);

    EXPECT_EQ(response.games_size(), 1);
    EXPECT_EQ(response.games(0).name(), "The Witcher 3");
}

UTEST_F(GameServiceTest, SearchGames_FallbackToIgdb)
{
    ::games::SearchGamesRequest request;
    request.set_query("Cyberpunk");
    request.set_limit(5);

    EXPECT_CALL(mock_repo_, FindGame(_, _))
        .WillOnce(testing::Return(std::vector<entities::GamePostgres>{}));

    std::vector<entities::GameInfo> igdb_games;
    entities::GameInfo info;
    info.name = "Cyberpunk 2077";
    igdb_games.push_back(info);

    EXPECT_CALL(mock_igdb_,
                SearchGames(testing::Eq("Cyberpunk"), testing::Eq(5)))
        .WillOnce(testing::Return(igdb_games));

    EXPECT_CALL(mock_repo_, CreateGame(_))
        .WillOnce(
            testing::Return(test::CreateFakePostgresGame("Cyberpunk 2077")));

    auto client = MakeClient<::games::GameServiceClient>();
    auto response = client.SearchGames(request);

    EXPECT_EQ(response.games_size(), 1);
}

UTEST_F(GameServiceTest, SearchGames_DbError)
{
    ::games::SearchGamesRequest request;
    request.set_query("Doom");

    EXPECT_CALL(mock_repo_, FindGame(_, _))
        .WillOnce(testing::Throw(std::runtime_error("DB Connection Lost")));

    auto client = MakeClient<::games::GameServiceClient>();

    try
    {
        client.SearchGames(request);
        FAIL() << "Expected RpcError";
    }
    catch (const userver::ugrpc::client::ErrorWithStatus& e)
    {
        EXPECT_EQ(e.GetStatus().error_code(), grpc::StatusCode::CANCELLED);
    }
}

// --- 2. GET GAME ---
UTEST_F(GameServiceTest, GetGame_ById)
{
    auto fake_game = test::CreateFakePostgresGame("Doom");
    std::string str_id = boost::uuids::to_string(fake_game.id);

    ::games::GetGameRequest request;
    request.set_game_id(str_id);

    EXPECT_CALL(mock_repo_, GetGameById(testing::Eq(str_id)))
        .WillOnce(testing::Return(
            std::optional<entities::GamePostgres>{ fake_game }));

    auto client = MakeClient<::games::GameServiceClient>();
    auto response = client.GetGame(request);

    EXPECT_EQ(response.game().id(), str_id);
    EXPECT_EQ(response.game().name(), "Doom");
}

UTEST_F(GameServiceTest, GetGame_BySlug)
{
    auto fake_game = test::CreateFakePostgresGame("Zelda");
    fake_game.slug = "zelda";

    ::games::GetGameRequest request;
    request.set_slug("zelda");

    EXPECT_CALL(mock_repo_, GetGameBySlug(testing::Eq("zelda")))
        .WillOnce(testing::Return(
            std::optional<entities::GamePostgres>{ fake_game }));

    auto client = MakeClient<::games::GameServiceClient>();
    auto response = client.GetGame(request);

    EXPECT_EQ(response.game().slug(), "zelda");
}

UTEST_F(GameServiceTest, GetGame_NotFound)
{
    ::games::GetGameRequest request;
    request.set_game_id("unknown-id");

    EXPECT_CALL(mock_repo_, GetGameById(_))
        .WillOnce(testing::Return(std::nullopt));

    auto client = MakeClient<::games::GameServiceClient>();

    try
    {
        client.GetGame(request);
        FAIL() << "Expected NOT_FOUND";
    }
    catch (const userver::ugrpc::client::ErrorWithStatus& e)
    {
        EXPECT_EQ(e.GetStatus().error_code(), grpc::StatusCode::NOT_FOUND);
    }
}

UTEST_F(GameServiceTest, GetGame_Validation)
{
    auto client = MakeClient<::games::GameServiceClient>();

    try
    {
        ::games::GetGameRequest request;
        client.GetGame(request);
        FAIL() << "Expected error for empty request";
    }
    catch (const userver::ugrpc::client::ErrorWithStatus& e)
    {
        EXPECT_EQ(e.GetStatus().error_code(),
                  grpc::StatusCode::INVALID_ARGUMENT);
    }
}

// --- 3. GET GAMES BY GENRE ---
UTEST_F(GameServiceTest, GetGamesByGenre_FromDb)
{
    ::games::GetGamesByGenreRequest request;
    request.set_genre_name("RPG");
    request.set_limit(10);

    std::vector<entities::GamePostgres> games;
    games.push_back(test::CreateFakePostgresGame("Baldur's Gate 3"));

    EXPECT_CALL(mock_repo_, GetGamesByGenre(testing::Eq("RPG"), Eq(10)))
        .WillOnce(Return(games));

    auto client = MakeClient<::games::GameServiceClient>();
    auto response = client.GetGamesByGenre(request);

    EXPECT_EQ(response.games_size(), 1);
}

UTEST_F(GameServiceTest, GetGamesByGenre_FallbackIgdb)
{
    ::games::GetGamesByGenreRequest request;
    request.set_genre_name("Indie");
    request.set_limit(5);

    EXPECT_CALL(mock_repo_, GetGamesByGenre(_, _))
        .WillOnce(Return(std::vector<entities::GamePostgres>{}));

    std::vector<entities::GameInfo> igdb_res;
    entities::GameInfo info;
    info.name = "Hades";
    igdb_res.emplace_back(info);

    EXPECT_CALL(mock_igdb_, GetGamesByGenre(Eq("Indie"), Eq(5)))
        .WillOnce(Return(igdb_res));

    EXPECT_CALL(mock_repo_, CreateGame(_))
        .WillOnce(Return(test::CreateFakePostgresGame("Hades")));

    auto client = MakeClient<::games::GameServiceClient>();
    auto response = client.GetGamesByGenre(request);

    EXPECT_EQ(response.games_size(), 1);
}

// --- 4. GET TOP RATED GAMES ---
UTEST_F(GameServiceTest, GetTopRatedGames_Success)
{
    ::games::GetDiscoveryRequest request;
    request.set_limit(3);

    std::vector<entities::GamePostgres> games;
    games.push_back(test::CreateFakePostgresGame("Top Game"));

    EXPECT_CALL(mock_repo_, GetTopRatedGames(3)).WillOnce(Return(games));

    auto client = MakeClient<::games::GameServiceClient>();
    auto response = client.GetTopRatedGames(request);

    EXPECT_EQ(response.games_size(), 1);
}

// --- 5. GET UPCOMING GAMES ---
UTEST_F(GameServiceTest, GetUpcomingGames_FallbackIgdb)
{
    ::games::GetDiscoveryRequest request;
    request.set_limit(5);

    EXPECT_CALL(mock_repo_, GetUpcomingGames(_))
        .WillOnce(Return(std::vector<entities::GamePostgres>{}));

    std::vector<entities::GameInfo> igdb_res;
    entities::GameInfo info;
    info.name = "GTA VI";
    igdb_res.push_back(info);
    EXPECT_CALL(mock_igdb_, GetUpcomingGames(5)).WillOnce(Return(igdb_res));

    EXPECT_CALL(mock_repo_, CreateGame(_))
        .WillOnce(Return(test::CreateFakePostgresGame("GTA VI")));

    auto client = MakeClient<::games::GameServiceClient>();
    auto response = client.GetUpcomingGames(request);

    EXPECT_EQ(response.games_size(), 1);
}

// --- 6. LIST GAMES ---
UTEST_F(GameServiceTest, ListGames_WithFilter)
{
    ::games::ListGamesRequest request;
    request.set_limit(20);
    request.set_offset(0);
    request.set_filter(::games::SortingType::PLAYHUB_RATING);

    std::vector<entities::GamePostgres> games;
    games.push_back(test::CreateFakePostgresGame("List Item"));

    EXPECT_CALL(mock_repo_,
                GetAllGames(20, 0, ::games::SortingType::PLAYHUB_RATING))
        .WillOnce(Return(games));

    auto client = MakeClient<::games::GameServiceClient>();
    auto response = client.ListGames(request);

    EXPECT_EQ(response.games_size(), 1);
}

// --- 7. SET RATING ---
UTEST_F(GameServiceTest, SetRating_Success)
{
    ::games::RatingRequest request;
    request.set_game_id("valid-uuid");
    request.set_rating(10.0);

    EXPECT_CALL(mock_repo_, UpdateGameRating("valid-uuid", 10.0)).Times(1);

    auto client = MakeClient<::games::GameServiceClient>();
    EXPECT_NO_THROW(client.SetRating(request));
}

UTEST_F(GameServiceTest, SetRating_InvalidUuid)
{
    ::games::RatingRequest request;
    request.set_game_id("bad-uuid");
    request.set_rating(5.0);

    EXPECT_CALL(mock_repo_, UpdateGameRating(_, _))
        .WillOnce(Throw(std::runtime_error("Invalid UUID")));

    auto client = MakeClient<::games::GameServiceClient>();

    try
    {
        client.SetRating(request);
        FAIL() << "Expected INVALID_ARGUMENT";
    }
    catch (const userver::ugrpc::client::ErrorWithStatus& e)
    {
        EXPECT_EQ(e.GetStatus().error_code(),
                  grpc::StatusCode::INVALID_ARGUMENT);
    }
}