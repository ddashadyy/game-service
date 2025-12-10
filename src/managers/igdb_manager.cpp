// project headers
#include <managers/igdb_manager.hpp>
#include <parser/json_parser.hpp>
#include <tools/utils.hpp>

// std
#include <format>

namespace {
    constexpr std::string_view APIQuery = 
        "fields name,summary,rating,genres.name,"
        "first_release_date,artworks.url,cover.url,"
        "hypes,platforms.name,screenshots.url,slug,themes.name; ";
}

igdb::IGDBManager::IGDBManager(std::string_view envFile)
{
    const auto env = utils::LoadEnv(envFile);
    clientId_ = env.at("CLIENT_ID");
    clientSecret_ = env.at("CLIENT_SECRET");
}

std::optional<std::string> igdb::IGDBManager::GetTwitchToken() const 
{
    try 
    {
        const auto response = utils::PerformHttpRequest(
            "id.twitch.tv",
            "443",
            "/oauth2/token?client_id=" + clientId_ + 
            "&client_secret=" + clientSecret_ + 
            "&grant_type=client_credentials",
            http::verb::post,
            "",
            {{"Content-Type", "application/x-www-form-urlencoded"}}
        );
        
        if (response.empty()) 
        {
            return std::nullopt;
        }
        
        return response;
    } 
    catch (const std::exception& e) 
    {
        std::cerr << "Error getting Twitch token: " << e.what() << std::endl;
        return std::nullopt;
    }
}

bool igdb::IGDBManager::Authenticate() 
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
            tokenExpiry_ = std::chrono::system_clock::now() + 
                          std::chrono::seconds(expiresIn - kTokenExpiryBufferSeconds);
        }
        
        return true;
    } 
    catch (const nlohmann::json::exception& e) 
    {
        std::cerr << "Failed to parse token response: " << e.what() << std::endl;
        return false;
    }

}

igdb::IGDBManager::GamesInfo igdb::IGDBManager::SearchGames(std::string_view query, std::int32_t limit) 
{
    if (!Authenticate()) 
    {
        std::cerr << "Authentication failed in SearchGames" << std::endl;
        return {};
    }
    
    const auto body = std::format(
        "{}search \"{}\"; limit {};", 
        APIQuery, query, limit
    );
    
    const auto response = utils::PerformHttpRequest(
        "api.igdb.com",
        "443",
        "/v4/games",
        http::verb::post,
        body,
        {
            {"Client-ID", clientId_},
            {"Authorization", "Bearer " + accessToken_}
        }
    );
    
    return ParseGamesResponse(response);
}

igdb::IGDBManager::GamesInfo igdb::IGDBManager::GetGamesByGenre(std::string_view genre, std::int32_t limit) 
{
    if (!Authenticate()) 
    {
        std::cerr << "Authentication failed in GetGamesByGenre" << std::endl;
        return {};
    }

    const auto body = std::format(
        "{}where genres.name = \"{}\"; limit {};",
        APIQuery,
        genre,
        limit
    );
    
    const auto response = utils::PerformHttpRequest(
        "api.igdb.com",
        "443",
        "/v4/games",
        http::verb::post,
        body,
        {
            {"Client-ID", clientId_},
            {"Authorization", "Bearer " + accessToken_}
        }
    );
    
    return ParseGamesResponse(response);
}

igdb::IGDBManager::GamesInfo igdb::IGDBManager::GetTopRatedGames(std::int32_t limit) 
{
    if (!Authenticate()) 
    {
        std::cerr << "Authentication failed in GetTopRatedGames" << std::endl;
        return {};
    }

    const auto body = std::format(
        "{}where rating > 75; "
        "sort rating desc; "
        "limit {};",
        APIQuery,
        limit
    );
    
    const auto response = utils::PerformHttpRequest(
        "api.igdb.com",
        "443",
        "/v4/games",
        http::verb::post,
        body,
        {
            {"Client-ID", clientId_},
            {"Authorization", "Bearer " + accessToken_}
        }
    );
    
    return ParseGamesResponse(response);
}

igdb::IGDBManager::GamesInfo igdb::IGDBManager::GetUpcomingGames(std::int32_t limit) 
{
    if (!Authenticate()) 
    {
        std::cerr << "Authentication failed in GetUpcomingGames" << std::endl;
        return {};
    }
    
    std::time_t now = std::time(nullptr);
    const auto body = std::format(
        "{}where first_release_date > {}; "
        "sort first_release_date asc; "
        "limit {};",
        APIQuery,
        now,
        limit
    );
    
    const auto response = utils::PerformHttpRequest(
        "api.igdb.com",
        "443",
        "/v4/games",
        http::verb::post,
        body,
        {
            {"Client-ID", clientId_},
            {"Authorization", "Bearer " + accessToken_}
        }
    );
    
    return ParseGamesResponse(response);
}

igdb::IGDBManager::GamesInfo igdb::IGDBManager::ParseGamesResponse(std::string_view response) const 
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
            std::cerr << "Expected array in response, got: " << json.type_name() << std::endl;
            return games;
        }
        
        for (const auto& gameJson : json) 
        {
            entities::GameInfo game;
        
            try 
            {
                // ID
                if (gameJson.contains("id") && !gameJson["id"].is_null()) 
                    game.id = std::to_string(gameJson["id"].get<std::uint32_t>());

                // name
                if (gameJson.contains("name") && !gameJson["name"].is_null()) 
                    game.name = gameJson["name"].get<std::string>();

                // slug
                if (gameJson.contains("slug") && !gameJson["slug"].is_null()) 
                    game.slug = gameJson["slug"].get<std::string>();

                // summary
                if (gameJson.contains("summary") && !gameJson["summary"].is_null()) 
                    game.summary = gameJson["summary"].get<std::string>();

                // rating
                if (gameJson.contains("rating") && !gameJson["rating"].is_null()) 
                    game.rating = gameJson["rating"].get<double>();
                else game.rating = 0.0;
                
                // hypes
                if (gameJson.contains("hypes") && !gameJson["hypes"].is_null()) 
                    game.hypes = gameJson["hypes"].get<std::uint32_t>();
                
                // firstReleaseDate
                if (gameJson.contains("first_release_date") && !gameJson["first_release_date"].is_null()) 
                    game.firstReleaseDate = utils::TimestampToString(gameJson["first_release_date"].get<time_t>());
                else game.firstReleaseDate = "N/A";
                
                // releaseDates
                if (gameJson.contains("release_dates") && gameJson["release_dates"].is_array()) 
                    for (const auto& releaseDate : gameJson["release_dates"])                      
                        game.releaseDates.emplace_back(utils::TimestampToString(releaseDate["date"].get<time_t>()));
                    
                // coverUrls
                if (gameJson.contains("cover") && !gameJson["cover"].is_null()) 
                    if (gameJson["cover"].is_object() && gameJson["cover"].contains("url")) 
                        game.coverUrl = utils::ForceOriginalQuality(gameJson["cover"]["url"].get<std::string>());
                
                // artworkUrls
                if (gameJson.contains("artworks") && gameJson["artworks"].is_array()) 
                    for (const auto& artwork : gameJson["artworks"])
                        if (artwork.contains("url") && !artwork["url"].is_null())
                            game.artworkUrls.emplace_back(utils::ForceOriginalQuality(artwork["url"].get<std::string>()));
                    
                // screenshots
                if (gameJson.contains("screenshots") && gameJson["screenshots"].is_array()) 
                    for (const auto& screenshot : gameJson["screenshots"])
                        if (screenshot.contains("url") && !screenshot["url"].is_null())
                            game.screenshots.emplace_back(utils::ForceOriginalQuality(screenshot["url"].get<std::string>()));

                // genres
                if (gameJson.contains("genres") && gameJson["genres"].is_array()) 
                    for (const auto& genre : gameJson["genres"]) 
                        if (genre.contains("name") && !genre["name"].is_null()) 
                            game.genres.emplace_back(genre["name"].get<std::string>());
                        
                // themes
                if (gameJson.contains("themes") && gameJson["themes"].is_array()) 
                    for (const auto& theme : gameJson["themes"]) 
                        if (theme.contains("name") && !theme["name"].is_null()) 
                            game.themes.emplace_back(theme["name"].get<std::string>());
                
                // platforms
                if (gameJson.contains("platforms") && gameJson["platforms"].is_array()) 
                    for (const auto& platform : gameJson["platforms"]) 
                        if (platform.contains("name") && !platform["name"].is_null()) 
                            game.platforms.emplace_back(platform["name"].get<std::string>());
                

                games.emplace_back(game);
            } 
            catch (const nlohmann::json::exception& e) 
            {
                std::cerr << "Error parsing game entry: " << e.what() << std::endl;
            }
        }
    } 
    catch (const nlohmann::json::exception& e) 
    {
        std::cerr << "Failed to parse games response: " << e.what() << std::endl;
        std::cerr << "Response was: " << response << std::endl;
    }
    
    return games;
}