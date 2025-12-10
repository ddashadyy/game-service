cmake_minimum_required(VERSION 3.12...3.31)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

project(game-service CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(DownloadUserver)

find_package(userver COMPONENTS core postgresql grpc QUIET)
if(NOT userver_FOUND)
  download_userver(TRY_DIR third_party/userver VERSION 2.8)
endif()

userver_setup_environment()

# find_package(fmt REQUIRED)

include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

# --- [FIX 1] Меняем OBJECT на STATIC для надежной линковки ---
add_library(igdb_api_client_lib STATIC
    include/managers/igdb_manager.hpp # (Проверьте имя файла, у вас было namager)
    src/managers/igdb_manager.cpp
    
    include/parser/json_parser.hpp
    src/parser/json_parser.cpp

    include/tools/utils.hpp
    src/tools/utils.cpp

    include/structs/game_info.hpp
)

target_link_libraries(igdb_api_client_lib PUBLIC 
    # --- [FIX 2] Добавляем userver::core, чтобы видеть fmt и логи ---
    userver::core
    # fmt::fmt
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

# Common sources (можно оставить OBJECT, так как это линкуется в exe напрямую)
add_library(${PROJECT_NAME}_objs OBJECT
    include/repository/postgres_manager.hpp
    src/repository/postgres_manager.cpp

    include/handlers/game_grpc.hpp
    src/handlers/game_grpc.cpp

    include/structs/game_postgres.hpp
)

# Теперь, так как igdb - STATIC, она корректно передастся сюда
target_link_libraries(${PROJECT_NAME}_objs PUBLIC userver::postgresql userver::grpc igdb_api_client_lib)

# --- [FIX 3] Пути с тильдой (~) могут не работать. Используйте абсолютный путь или переменную ---
# Например, если папка playhub-proto лежит на уровень выше папки проекта:
# set(PROTO_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../playhub-proto")
# Если путь жесткий, лучше указывать полный путь, но попробуем оставить, если работает.
set(PROTO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/playhub-proto/proto/games")
set(GAME_SERVICE_PROTO_FILE "${PROTO_ROOT}/games.proto")
set(GENERATED_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/generated")

userver_add_grpc_library(
    ${PROJECT_NAME}_proto
    PROTOS ${GAME_SERVICE_PROTO_FILE}
    SOURCE_PATH ${PROTO_ROOT}
    OUTPUT_PATH ${GENERATED_ROOT}
)
target_link_libraries(${PROJECT_NAME}_objs PUBLIC ${PROJECT_NAME}_proto)

# The Service
add_executable(${PROJECT_NAME} src/main.cpp)

# Сначала objs, потом либы. 
# Важно: если _objs это OBJECT lib, его нужно добавлять в sources или через $<TARGET_OBJECTS:...>
# Но cmake умеет линковать OBJECT через target_link_libraries в новых версиях.
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs)

target_include_directories(${PROJECT_NAME}_objs PUBLIC ${GENERATED_ROOT})

# Install
include(GNUInstallDirs)
# ... (остальное без изменений)
if(DEFINED ENV{PREFIX})
  message(STATUS "Set install prefix: $ENV{PREFIX}")
  file(TO_CMAKE_PATH "$ENV{PREFIX}" PREFIX_PATH)
  set(CMAKE_INSTALL_PREFIX "${PREFIX_PATH}")
endif()

file(GLOB CONFIGS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.yaml ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.json)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${PROJECT_NAME})
install(FILES ${CONFIGS_FILES} DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME} COMPONENT ${PROJECT_NAME})