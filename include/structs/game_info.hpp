#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace entities {

struct GameInfo
{
    std::string id;
    std::string name;
    std::string slug;
    std::string summary;

    std::int32_t igdb_rating;
    std::int32_t playhub_rating;
    std::int32_t hypes;

    std::string firstReleaseDate;
    std::vector<std::string> releaseDates;

    std::string coverUrl;
    std::vector<std::string> artworkUrls;
    std::vector<std::string> screenshots;

    std::vector<std::string> genres;
    std::vector<std::string> themes;
    std::vector<std::string> platforms;
};

} // namespace entities