#pragma once

#include <string>
#include <vector>
#include <cstdint> 

namespace entities {

struct GameInfo 
{
    // Основная информация
    std::string id;
    std::string name;
    std::string slug;
    std::string summary;
    
    // Рейтинги и популярность
    double rating;
    std::int32_t hypes;
    
    // Даты
    std::string firstReleaseDate;
    std::vector<std::string> releaseDates;
    
    // Медиа
    std::string coverUrl;
    std::vector<std::string> artworkUrls;
    std::vector<std::string> screenshots;
    
    // Классификации
    std::vector<std::string> genres;
    std::vector<std::string> themes;
    std::vector<std::string> platforms;
};

} // namespace entities 