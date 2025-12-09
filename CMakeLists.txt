cmake_minimum_required(VERSION 3.12...3.31)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

project(game-service CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(DownloadUserver)

find_package(userver COMPONENTS core postgresql grpc QUIET)
if(NOT userver_FOUND)
  # Tries TRY_DIR first, falls back to downloading userver from GitHub using CPM.
  download_userver(TRY_DIR third_party/userver VERSION 2.8)
endif()

userver_setup_environment()

find_package(nlohmann_json REQUIRED)

# twitch api 
add_library(igdb_api_client_lib OBJECT
    include/managers/igdb_namager.hpp
    src/managers/igdb_manager.cpp
    
    include/parser/json_parser.hpp
    src/parser/json_parser.cpp

    include/tools/utils.hpp
    src/tools/utils.cpp

    include/structs/game_info.hpp
)

target_link_libraries(igdb_api_client_lib PUBLIC 
    Boost::headers
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
)

target_include_directories(igdb_api_client_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(igdb_api_client_lib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Common sources
add_library(${PROJECT_NAME}_objs OBJECT
    include/repository/postgres_manager.hpp
    src/repository/postgres_manager.cpp

    include/handlers/game_grpc.hpp
    src/handlers/game_grpc.cpp

    include/structs/game_postgres.hpp

)
target_link_libraries(${PROJECT_NAME}_objs PUBLIC userver::postgresql userver::grpc igdb_api_client_lib)


set(GAME_SERVICE_PROTO_FILE "~/playhub-proto/proto/games/service.proto")
set(PROTO_ROOT "~/playhub-proto/proto/games")
set(GENERATED_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/generated")


# # Create a proto library with userver extensions
userver_add_grpc_library(
    ${PROJECT_NAME}_proto
    PROTOS ${GAME_SERVICE_PROTO_FILE}
    SOURCE_PATH ${PROTO_ROOT}
    OUTPUT_PATH ${GENERATED_ROOT}
)
target_link_libraries(${PROJECT_NAME}_objs PUBLIC ${PROJECT_NAME}_proto)


# The Service
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs)

target_include_directories(${PROJECT_NAME}_objs PUBLIC ${GENERATED_ROOT})

# Unit Tests
# add_executable(${PROJECT_NAME}_unittest
#     src/hello_test.cpp
# )
# target_link_libraries(${PROJECT_NAME}_unittest PRIVATE ${PROJECT_NAME}_objs userver::utest)
# add_google_tests(${PROJECT_NAME}_unittest)


# # Benchmarks
# add_executable(${PROJECT_NAME}_benchmark
#     src/hello_benchmark.cpp
# )
# target_link_libraries(${PROJECT_NAME}_benchmark PRIVATE ${PROJECT_NAME}_objs userver::ubench)
# add_google_benchmark_tests(${PROJECT_NAME}_benchmark)


# # Functional Tests
# userver_testsuite_add_simple()


# Install
include(GNUInstallDirs)

if(DEFINED ENV{PREFIX})
  message(STATUS "Set install prefix: $ENV{PREFIX}")
  file(TO_CMAKE_PATH "$ENV{PREFIX}" PREFIX_PATH)
  set(CMAKE_INSTALL_PREFIX "${PREFIX_PATH}")
endif()

file(GLOB CONFIGS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.yaml ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.json)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${PROJECT_NAME})
install(FILES ${CONFIGS_FILES} DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME} COMPONENT ${PROJECT_NAME})