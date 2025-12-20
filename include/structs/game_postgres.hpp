#pragma once

#include <string>
#include <vector>

#include <boost/uuid/uuid.hpp>
#include <userver/storages/postgres/io/chrono.hpp>



namespace entities {

struct GamePostgres
{
    boost::uuids::uuid id;

    std::string igdb_id;
    std::string name;
    std::string slug;
    std::string summary;
    
    double igdb_rating;
    double playhub_rating;
    std::int32_t hypes;
    
    std::string firstReleaseDate;
    std::vector<std::string> releaseDates;
    
    std::string coverUrl;
    std::vector<std::string> artworkUrls;
    std::vector<std::string> screenshots;
    
    std::vector<std::string> genres;
    std::vector<std::string> themes;
    std::vector<std::string> platforms;

    userver::storages::postgres::TimePointWithoutTz created_at;
    userver::storages::postgres::TimePointWithoutTz updated_at;

};

} // namespace entities