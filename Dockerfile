# === Этап сборки (Builder) ===
# Используем тот же образ, что и в devcontainer.json
# Это сэкономит кучу времени, так как userver и зависимости уже внутри.
FROM ghcr.io/userver-framework/ubuntu-22.04-userver-pg-dev:latest AS builder

# Настраиваем рабочую директорию
WORKDIR /app

# Копируем исходный код
COPY . .

# Настройка и сборка
# Используем CMAKE_CXX_STANDARD=20, так как вы его задали в CMakeLists
RUN cmake -B build_release \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSERVER_FEATURE_GRPC=ON \
    -DUSERVER_FEATURE_POSTGRESQL=ON \
    -DCMAKE_CXX_STANDARD=20 \
    -GNinja \
    && cmake --build build_release

# === Этап запуска (Runtime) ===
# Для запуска берем чистую Ubuntu, чтобы образ весил меньше
FROM ubuntu:22.04

# Устанавливаем только библиотеки, необходимые для ЗАПУСКА (Runtime dependencies)
# Userver image основан на ubuntu 22.04, поэтому версии библиотек совпадут
RUN apt-get update && apt-get install -y \
    libssl3 \
    libev4 \
    libyaml-cpp0.7 \
    libnghttp2-14 \
    libcurl4 \
    libpq5 \
    ca-certificates \
    tzdata \
    libatomic1 \
    libfmt8 \
    libcctz2 \
    && rm -rf /var/lib/apt/lists/*

# Настройка таймзоны (опционально)
ENV TZ=UTC
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

WORKDIR /app

# Копируем скомпилированный бинарник из этапа builder
COPY --from=builder /app/build_release/game-service /app/game-service

# Копируем папку с конфигами
COPY --from=builder /app/configs /app/configs

# Создаем директорию для логов (если сервис пишет в файл)
RUN mkdir -p /var/log/game-service

# Указываем команду запуска
# --config указывает путь к статическому конфигу
CMD ["/app/game-service", "--config", "/app/configs/static_config.yaml", "--config_vars", "/app/configs/config_vars.yaml"]
