# === Этап сборки (Build Stage) ===
FROM ubuntu:22.04 AS builder

# Установка часового пояса и обновление
ENV TZ=Europe/Moscow
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Установка зависимостей сборки
RUN apt-get update && apt-get install -y \
    software-properties-common \
    wget \
    git \
    make \
    cmake \
    python3 \
    # Добавляем репозиторий для свежего GCC (нужен для C++20 stdlib)
    && add-apt-repository ppa:ubuntu-toolchain-r/test -y \
    && apt-get update \
    && apt-get install -y \
        gcc-13 g++-13 \
        clang-14 \
        libboost-all-dev \
        libssl-dev \
        libev-dev \
        libyaml-cpp-dev \
        libnghttp2-dev \
        libcurl4-openssl-dev \
        libpq-dev \
        libldap2-dev \
        libkrb5-dev \
        ccache

# Настраиваем clang на использование свежей стандартной библиотеки от gcc-13
ENV CC=clang-14
ENV CXX=clang++-14
ENV CXXFLAGS="-stdlib=libstdc++ --gcc-toolchain=/usr/lib/gcc/x86_64-linux-gnu/13 -I/usr/include/c++/13 -I/usr/include/x86_64-linux-gnu/c++/13"

# Копируем исходный код
WORKDIR /app
COPY . .

# Сборка проекта
RUN cmake -B build_release \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSERVER_FEATURE_GRPC=ON \
    -DUSERVER_FEATURE_POSTGRESQL=ON \
    && cmake --build build_release -- -j$(nproc)

# === Этап запуска (Runtime Stage) ===
FROM ubuntu:22.04

# Установка runtime-зависимостей
RUN apt-get update && apt-get install -y \
    libssl3 \
    libev4 \
    libyaml-cpp0.7 \
    libnghttp2-14 \
    libcurl4 \
    libpq5 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Копируем бинарник из этапа сборки
COPY --from=builder /app/build_release/game-service /app/game-service

# Копируем конфиги (предполагаем, что они лежат в папке configs)
COPY --from=builder /app/configs /app/configs

# Создаем папку для логов (если нужна)
RUN mkdir -p /var/log/game-service

# Запуск
CMD ["/app/game-service", "--config", "/app/configs/config.yaml", "/app/configs/config.yaml"]