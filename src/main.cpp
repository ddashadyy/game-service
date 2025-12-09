// #include <iostream>
// #include <iomanip>
// #include <managers/igdb_namager.hpp>


// void PrintGameInfoFull(const entities::GameInfo& game, const std::string& prefix = "") 
// {
//     std::cout << prefix << "═══════════════════════════════════════════════════════\n";
//     std::cout << prefix << "🎮 GAME: " << game.name << "\n";
//     std::cout << prefix << "═══════════════════════════════════════════════════════\n";
    
//     // Основная информация
//     std::cout << prefix << "📋 BASIC INFO:\n";
//     std::cout << prefix << "  ID:          " << game.id << "\n";
//     std::cout << prefix << "  Name:        " << game.name << "\n";
//     std::cout << prefix << "  Slug:        " << game.slug << "\n";
    
//     // Рейтинги
//     std::cout << prefix << "📊 RATINGS & POPULARITY:\n";
//     std::cout << prefix << "  Rating:      " << std::fixed << std::setprecision(1) 
//               << game.rating << "/100\n";
//     std::cout << prefix << "  Hypes:       " << game.hypes << "\n";
    
//     // Даты
//     std::cout << prefix << "📅 RELEASE DATES:\n";
//     std::cout << prefix << "  First:       " << game.firstReleaseDate << "\n";
//     std::cout << prefix << "  All (" << game.releaseDates.size() << "):\n";
//     for (size_t i = 0; i < game.releaseDates.size(); ++i) 
//     {
//         std::cout << prefix << "    " << (i+1) << ". " << game.releaseDates[i] << "\n";
//     }
    
//     // Медиа
//     std::cout << prefix << "🖼️ MEDIA:\n";
//     std::cout << prefix << "  Cover URL:   " 
//               << (game.coverUrl.empty() ? "[NONE]" : game.coverUrl) << "\n";
//     std::cout << prefix << "  Artworks (" << game.artworkUrls.size() << "):\n";
//     for (size_t i = 0; i < std::min<size_t>(3, game.artworkUrls.size()); ++i) 
//     {
//         std::cout << prefix << "    " << (i+1) << ". " << game.artworkUrls[i] << "\n";
//     }
//     if (game.artworkUrls.size() > 3) 
//     {
//         std::cout << prefix << "    ... and " << (game.artworkUrls.size() - 3) 
//                   << " more\n";
//     }
    
//     std::cout << prefix << "  Screenshots (" << game.screenshots.size() << "):\n";
//     for (size_t i = 0; i < std::min<size_t>(3, game.screenshots.size()); ++i) 
//     {
//         std::cout << prefix << "    " << (i+1) << ". " << game.screenshots[i] << "\n";
//     }
//     if (game.screenshots.size() > 3) 
//     {
//         std::cout << prefix << "    ... and " << (game.screenshots.size() - 3) 
//                   << " more\n";
//     }
    
//     // Классификации
//     std::cout << prefix << "🏷️ CLASSIFICATIONS:\n";
    
//     std::cout << prefix << "  Genres (" << game.genres.size() << "): ";
//     for (size_t i = 0; i < game.genres.size(); ++i) 
//     {
//         if (i > 0) std::cout << ", ";
//         std::cout << game.genres[i];
//     }
//     if (game.genres.empty()) std::cout << "[NONE]";
//     std::cout << "\n";
    
//     std::cout << prefix << "  Themes (" << game.themes.size() << "): ";
//     for (size_t i = 0; i < game.themes.size(); ++i) 
//     {
//         if (i > 0) std::cout << ", ";
//         std::cout << game.themes[i];
//     }
//     if (game.themes.empty()) std::cout << "[NONE]";
//     std::cout << "\n";
    
//     std::cout << prefix << "  Platforms (" << game.platforms.size() << "): ";
//     for (size_t i = 0; i < game.platforms.size(); ++i) 
//     {
//         if (i > 0) std::cout << ", ";
//         std::cout << game.platforms[i];
//     }
//     if (game.platforms.empty()) std::cout << "[NONE]";
//     std::cout << "\n";
    
//     // Описание
//     if (!game.summary.empty()) 
//     {
//         std::cout << prefix << "📖 SUMMARY (" << game.summary.length() << " chars):\n";
//         std::cout << prefix << "  " << game.summary.substr(0, 200);
//         if (game.summary.length() > 200) std::cout << "...";
//         std::cout << "\n";
//     }
    
//     std::cout << prefix << "═══════════════════════════════════════════════════════\n\n";
// }

// void TestSearchGames(igdb::IGDBManager& manager) 
// {
//     std::cout << "\n🔍 Testing SearchGames\n";
//     std::cout << "═══════════════════════════════════════════════════════\n";
    
//     // Тест 1: Поиск существующей игры
//     std::cout << "Test 1: Searching for 'Escape from Tarkov'\n";
//     auto results = manager.SearchGames("Escape from Tarkov", 2);
//     std::cout << "Found " << results.size() << " result(s)\n\n";
    
//     for (size_t i = 0; i < results.size(); ++i) 
//     {
//         std::cout << "Result #" << (i+1) << ":\n";
//         PrintGameInfoFull(results[i], "  ");
//     }
    
//     // Тест 2: Поиск другой игры
//     std::cout << "\nTest 2: Searching for 'Minecraft'\n";
//     results = manager.SearchGames("Minecraft", 2);
//     std::cout << "Found " << results.size() << " result(s)\n\n";
    
//     if (!results.empty()) 
//     {
//         PrintGameInfoFull(results[0], "  ");
//     }
// }

// void TestGetGamesByGenre(igdb::IGDBManager& manager) 
// {
//     std::cout << "\n🎭 Testing GetGamesByGenre\n";
//     std::cout << "═══════════════════════════════════════════════════════\n";
    
//     // Тест разных жанров
//     std::vector<std::string> genres = {
//         "Role-playing (RPG)",
//         "Action",
//         "Adventure",
//         "Strategy",
//         "Simulator"
//     };
    
//     for (const auto& genre : genres) 
//     {
//         std::cout << "\nTest: Getting " << genre << " games (limit 3)\n";
//         auto results = manager.GetGamesByGenre(genre, 3);
//         std::cout << "Found " << results.size() << " game(s)\n";
        
//         if (!results.empty()) 
//         {
//             std::cout << "First game details:\n";
//             PrintGameInfoFull(results[0], "  ");
            
//             // Проверяем, что игра принадлежит указанному жанру
//             bool hasGenre = false;
//             for (const auto& g : results[0].genres) 
//             {
//                 if (g.find(genre.substr(0, 10)) != std::string::npos) 
//                 {
//                     hasGenre = true;
//                     break;
//                 }
//             }
            
//             if (hasGenre) 
//             {
//                 std::cout << "  ✅ Game has genre: " << genre << "\n";
//             }
//             else 
//             {
//                 std::cout << "  ⚠️  Game doesn't have expected genre. Genres: ";
//                 for (const auto& g : results[0].genres) std::cout << g << " ";
//                 std::cout << "\n";
//             }
//         }
//     }
// }

// void TestGetTopRatedGames(igdb::IGDBManager& manager) 
// {
//     std::cout << "\n🏆 Testing GetTopRatedGames\n";
//     std::cout << "═══════════════════════════════════════════════════════\n";
    
//     for (int limit : {3, 5}) 
//     {
//         std::cout << "\nTest: Top " << limit << " rated games (>75)\n";
//         auto results = manager.GetTopRatedGames(limit);
//         std::cout << "Found " << results.size() << " highly rated game(s)\n";
        
//         if (!results.empty()) 
//         {
//             std::cout << "\nTop " << std::min(limit, 2) << " games:\n";
//             for (size_t i = 0; i < std::min<size_t>(2, results.size()); ++i) 
//             {
//                 std::cout << "\n#" << (i+1) << " (Rating: " << results[i].rating << "):\n";
//                 PrintGameInfoFull(results[i], "  ");
//             }
            
//             // Проверяем, что рейтинги идут по убыванию
//             bool sorted = true;
//             for (size_t i = 1; i < results.size(); ++i) 
//             {
//                 if (results[i].rating > results[i-1].rating) 
//                 {
//                     sorted = false;
//                     break;
//                 }
//             }
            
//             if (sorted) 
//             {
//                 std::cout << "✅ Ratings are correctly sorted (descending)\n";
//             }
//             else 
//             {
//                 std::cout << "⚠️  Ratings are not properly sorted\n";
//             }
//         }
//     }
// }

// void TestGetUpcomingGames(igdb::IGDBManager& manager) 
// {
//     std::cout << "\n🚀 Testing GetUpcomingGames\n";
//     std::cout << "═══════════════════════════════════════════════════════\n";
    
//     std::cout << "\nTest: 3 upcoming games\n";
//     auto results = manager.GetUpcomingGames(3);
//     std::cout << "Found " << results.size() << " upcoming game(s)\n";
    
//     if (!results.empty()) 
//     {
//         std::cout << "\nUpcoming games:\n";
//         for (size_t i = 0; i < results.size(); ++i) 
//         {
//             std::cout << "\n#" << (i+1) << ":\n";
//             PrintGameInfoFull(results[i], "  ");
            
//             // Можно добавить проверку, что дата выпуска в будущем
//             // (но это зависит от формата даты в firstReleaseDate)
//         }
        
//         // Проверяем сортировку по дате
//         std::cout << "\nRelease dates: ";
//         for (const auto& game : results) 
//         {
//             std::cout << game.firstReleaseDate << " | ";
//         }
//         std::cout << "\n";
//     }
// }

// void TestAllMethods(igdb::IGDBManager& manager) 
// {
//     std::cout << "🎮 COMPREHENSIVE IGDB API TESTS - FULL DETAILS\n";
//     std::cout << "═══════════════════════════════════════════════════════\n\n";
    
//     TestSearchGames(manager);
//     TestGetGamesByGenre(manager);
//     TestGetTopRatedGames(manager);
//     TestGetUpcomingGames(manager);
    
//     std::cout << "\n═══════════════════════════════════════════════════════\n";
//     std::cout << "✅ All tests completed!\n";
// }

int main() 
{
    // try 
    // {
    //     std::cout << "Starting IGDB API tests with full details...\n";
        
    //     // Создаем менеджер
    //     igdb::IGDBManager manager("/home/gennadiy/game-service/.env");
        
    //     // Аутентификация
    //     if (!manager.Authenticate()) 
    //     {
    //         std::cerr << "❌ Authentication failed\n";
    //         return 1;
    //     }
    //     std::cout << "✅ Authentication successful\n\n";
        
    //     // Запускаем все тесты
    //     TestAllMethods(manager);
        
    // } 
    // catch (const std::exception& e) 
    // {
    //     std::cerr << "\n❌ ERROR: " << e.what() << std::endl;
    //     return 1;
    // }
    
    return 0;
}