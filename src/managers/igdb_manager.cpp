// project headers
#include <managers/igdb_manager.hpp>
#include <parser/json_parser.hpp>
#include <tools/utils.hpp>

// std
#include <cstdlib>
#include <fmt/format.h>

namespace igdb {

constexpr std::string_view kSearchGameQuery =
    "fields name,summary,rating,genres.name,"
    "first_release_date,artworks.url,cover.url,"
    "hypes,platforms.name,screenshots.url,slug,themes.name; ";

constexpr std::string_view kSearchGameBySlug =
    "where slug = \"{}\" & game_type = (0,8,9,10) & (game_status = null | "
    "game_status != (6, 7));";

constexpr std::string_view kSearchGameByGenre =
    "where genres.name = \"{}\" & game_type = (0,8,9,10) & (game_status = null "
    "| game_status != (6, 7)); sort rating desc; "
    "limit {};";

constexpr std::string_view kSearchUpcomingGames =
    "where first_release_date > {} & hypes != null & game_type = (0,8,9,10) & "
    "(game_status = null | game_status != (6, 7)); "
    "sort hypes desc; "
    "limit {};";

IGDBManager::IGDBManager()
    : clientId_(std::getenv("CLIENT_ID")),
      clientSecret_(std::getenv("CLIENT_SECRET"))
{}

std::optional<std::string> IGDBManager::GetTwitchToken() const
{
    try
    {
        const auto response = PerformHttpRequest(
            "id.twitch.tv", "443",
            "/oauth2/token?client_id=" + clientId_ + "&client_secret=" +
                clientSecret_ + "&grant_type=client_credentials",
            http::verb::post, "",
            { { "Content-Type", "application/x-www-form-urlencoded" } });

        if (response.empty())
            return std::nullopt;

        return response;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error getting Twitch token: " << e.what() << std::endl;
        return std::nullopt;
    }
}

bool IGDBManager::Authenticate()
{
    if (cachedToken_ && std::chrono::system_clock::now() < tokenExpiry_)
    {
        accessToken_ = *cachedToken_;
        return true;
    }

    auto tokenResponse = GetTwitchToken();
    if (!tokenResponse || tokenResponse->empty())
    {
        std::cerr << "Failed to get Twitch token" << std::endl;
        return false;
    }

    try
    {
        auto json = nlohmann::json::parse(*tokenResponse);

        if (!json.contains("access_token"))
        {
            std::cerr << "No access_token in response" << std::endl;
            return false;
        }

        accessToken_ = json["access_token"].get<std::string>();
        cachedToken_ = accessToken_;

        if (json.contains("expires_in"))
        {
            std::uint32_t expiresIn = json["expires_in"].get<std::uint32_t>();
            tokenExpiry_ =
                std::chrono::system_clock::now() +
                std::chrono::seconds(expiresIn - kTokenExpiryBufferSeconds);
        }

        return true;
    }
    catch (const nlohmann::json::exception& e)
    {
        std::cerr << "Failed to parse token response: " << e.what()
                  << std::endl;
        return false;
    }
}

IGDBManager::GamesInfo IGDBManager::SearchGames(std::string_view query,
                                                std::int32_t limit)
{
    if (!Authenticate())
    {
        std::cerr << "Authentication failed in SearchGames" << std::endl;
        return {};
    }

    const auto body =
        fmt::format("{}search \"{}\"; where game_type = (0,8,9,10) & "
                    "(game_status = null | game_status != (6, 7)); limit {};",
                    kSearchGameQuery, query, limit);

    const auto response = PerformHttpRequest(
        "api.igdb.com", "443", "/v4/games", http::verb::post, body,
        { { "Client-ID", clientId_ },
          { "Authorization", "Bearer " + accessToken_ } });

    return ParseGamesResponse(response);
}

IGDBManager::GamesInfo IGDBManager::GetGameBySlug(std::string_view slug)
{
    if (!Authenticate())
    {
        std::cerr << "Authentication failed in GetGameBySlug" << std::endl;
        return {};
    }

    const auto body = fmt::format("{}{}", kSearchGameQuery,
                                  fmt::format(kSearchGameBySlug, slug));

    const auto response = PerformHttpRequest(
        "api.igdb.com", "443", "/v4/games", http::verb::post, body,
        { { "Client-ID", clientId_ },
          { "Authorization", "Bearer " + accessToken_ } });

    return ParseGamesResponse(response);
}

IGDBManager::GamesInfo IGDBManager::GetGamesByGenre(std::string_view genre,
                                                    std::int32_t limit)
{
    if (!Authenticate())
    {
        std::cerr << "Authentication failed in GetGamesByGenre" << std::endl;
        return {};
    }

    const auto queryPart = fmt::format(kSearchGameByGenre, genre, limit);
    const auto body = fmt::format("{}{}", kSearchGameQuery, queryPart);

    const auto response = PerformHttpRequest(
        "api.igdb.com", "443", "/v4/games", http::verb::post, body,
        { { "Client-ID", clientId_ },
          { "Authorization", "Bearer " + accessToken_ } });

    return ParseGamesResponse(response);
}

IGDBManager::GamesInfo IGDBManager::GetUpcomingGames(std::int32_t limit)
{
    if (!Authenticate())
    {
        std::cerr << "Authentication failed in GetUpcomingGames" << std::endl;
        return {};
    }

    std::time_t now = std::time(nullptr);

    const auto queryPart = fmt::format(kSearchUpcomingGames, now, limit);
    const auto body = fmt::format("{}{}", kSearchGameQuery, queryPart);

    const auto response = PerformHttpRequest(
        "api.igdb.com", "443", "/v4/games", http::verb::post, body,
        { { "Client-ID", clientId_ },
          { "Authorization", "Bearer " + accessToken_ } });

    return ParseGamesResponse(response);
}

const std::string IGDBManager::PerformHttpRequest(
    std::string_view host, std::string_view port, std::string_view target,
    http::verb method, std::string_view body,
    const std::vector<std::pair<std::string_view, std::string_view>>& headers) const
{
    namespace ssl = net::ssl;

    try
    {
        net::io_context ioc;
        ssl::context ctx(ssl::context::tlsv12_client);
        ctx.set_default_verify_paths();

        ssl::stream<tcp::socket> stream(ioc, ctx);
        tcp::resolver resolver(ioc);

        const auto results = resolver.resolve(host, port);

        if (!SSL_set_tlsext_host_name(stream.native_handle(), host.data()))
        {
            beast::error_code ec{ static_cast<int>(::ERR_get_error()),
                                  net::error::get_ssl_category() };
            throw beast::system_error{ ec };
        }

        auto& lowest_layer = beast::get_lowest_layer(stream);
        net::connect(lowest_layer, results.begin(), results.end());
        stream.handshake(ssl::stream_base::client);

        http::request<http::string_body> req{ method,
                                              boost::string_view(target.data()),
                                              11 };
        req.set(http::field::host, boost::string_view(host.data()));
        req.set(http::field::user_agent, "IGDB-CPP-Client/1.0");

        for (const auto& [key, value] : headers)
        {
            req.set(boost::string_view(key.data()),
                    boost::string_view(value.data()));
        }

        if (!body.empty())
        {
            req.set(http::field::content_type, "application/json");
            req.body() = body;
            req.prepare_payload();
        }

        http::write(stream, req);

        beast::flat_buffer buffer;
        http::response<http::dynamic_body> res;
        http::read(stream, buffer, res);

        beast::error_code ec;
        stream.shutdown(ec);

        if (ec == net::error::eof || ec == ssl::error::stream_truncated)
            ec = {};
        if (ec)
            throw beast::system_error{ ec };

        return beast::buffers_to_string(res.body().data());
    }
    catch (const std::exception& e)
    {
        std::cerr << "HTTP Request Error: " << e.what() << std::endl;
        return "";
    }
}

IGDBManager::GamesInfo
IGDBManager::ParseGamesResponse(std::string_view response) const
{
    std::vector<entities::GameInfo> games;

    if (response.empty())
    {
        std::cerr << "Empty response received" << std::endl;
        return games;
    }

    try
    {
        auto json = nlohmann::json::parse(response);

        if (!json.is_array())
        {
            std::cerr << "Expected array in response, got: " << json.type_name()
                      << std::endl;
            return games;
        }

        for (const auto& gameJson : json)
        {
            entities::GameInfo game;

            try
            {
                // ID
                if (gameJson.contains("id") && !gameJson["id"].is_null())
                    game.id =
                        std::to_string(gameJson["id"].get<std::uint32_t>());

                // name
                if (gameJson.contains("name") && !gameJson["name"].is_null())
                    game.name = gameJson["name"].get<std::string>();

                // slug
                if (gameJson.contains("slug") && !gameJson["slug"].is_null())
                    game.slug = gameJson["slug"].get<std::string>();

                // summary
                if (gameJson.contains("summary") &&
                    !gameJson["summary"].is_null())
                    game.summary = gameJson["summary"].get<std::string>();

                // igdb_rating
                if (gameJson.contains("rating") &&
                    !gameJson["rating"].is_null())
                    game.igdb_rating = gameJson["rating"].get<double>();
                else
                    game.igdb_rating = 0.0;

                // hypes
                if (gameJson.contains("hypes") && !gameJson["hypes"].is_null())
                    game.hypes = gameJson["hypes"].get<std::uint32_t>();

                // firstReleaseDate
                if (gameJson.contains("first_release_date") &&
                    !gameJson["first_release_date"].is_null())
                    game.firstReleaseDate = utils::TimestampToString(
                        gameJson["first_release_date"].get<time_t>());
                else
                    game.firstReleaseDate = "N/A";

                // releaseDates
                if (gameJson.contains("release_dates") &&
                    gameJson["release_dates"].is_array())
                    for (const auto& releaseDate : gameJson["release_dates"])
                        game.releaseDates.emplace_back(utils::TimestampToString(
                            releaseDate["date"].get<time_t>()));

                // coverUrls
                if (gameJson.contains("cover") && !gameJson["cover"].is_null())
                    if (gameJson["cover"].is_object() &&
                        gameJson["cover"].contains("url"))
                        game.coverUrl = utils::ForceOriginalQuality(
                            gameJson["cover"]["url"].get<std::string>());

                // artworkUrls
                if (gameJson.contains("artworks") &&
                    gameJson["artworks"].is_array())
                    for (const auto& artwork : gameJson["artworks"])
                        if (artwork.contains("url") &&
                            !artwork["url"].is_null())
                            game.artworkUrls.emplace_back(
                                utils::ForceOriginalQuality(
                                    artwork["url"].get<std::string>()));

                // screenshots
                if (gameJson.contains("screenshots") &&
                    gameJson["screenshots"].is_array())
                    for (const auto& screenshot : gameJson["screenshots"])
                        if (screenshot.contains("url") &&
                            !screenshot["url"].is_null())
                            game.screenshots.emplace_back(
                                utils::ForceOriginalQuality(
                                    screenshot["url"].get<std::string>()));

                // genres
                if (gameJson.contains("genres") &&
                    gameJson["genres"].is_array())
                    for (const auto& genre : gameJson["genres"])
                        if (genre.contains("name") && !genre["name"].is_null())
                            game.genres.emplace_back(
                                genre["name"].get<std::string>());

                // themes
                if (gameJson.contains("themes") &&
                    gameJson["themes"].is_array())
                    for (const auto& theme : gameJson["themes"])
                        if (theme.contains("name") && !theme["name"].is_null())
                            game.themes.emplace_back(
                                theme["name"].get<std::string>());

                // platforms
                if (gameJson.contains("platforms") &&
                    gameJson["platforms"].is_array())
                    for (const auto& platform : gameJson["platforms"])
                        if (platform.contains("name") &&
                            !platform["name"].is_null())
                            game.platforms.emplace_back(
                                platform["name"].get<std::string>());

                games.emplace_back(game);
            }
            catch (const nlohmann::json::exception& e)
            {
                std::cerr << "Error parsing game entry: " << e.what()
                          << std::endl;
            }
        }
    }
    catch (const nlohmann::json::exception& e)
    {
        std::cerr << "Failed to parse games response: " << e.what()
                  << std::endl;
        std::cerr << "Response was: " << response << std::endl;
    }

    return games;
}
} // namespace igdb