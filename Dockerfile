
# Stage 1: Builder
FROM ubuntu:22.04 AS builder

# Отключить интерактивные запросы
ENV DEBIAN_FRONTEND=noninteractive

# Установить зависимости для сборки
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    nlohmann-json3-dev \
    libpcre2-dev \
    libspdlog-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Скопировать исходники
WORKDIR /build
COPY CMakeLists.txt ./
COPY src/ ./src/
COPY config/ ./config/

# Собрать только CLI версию (без GUI)
RUN mkdir -p build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DBUILD_GUI=OFF \
          .. && \
    make -j$(nproc) secret_detector && \
    strip secret_detector

# Stage 2: Runtime (минимальный образ)
FROM ubuntu:22.04

# Установить только runtime зависимости
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    libpcre2-8-0 \
    libspdlog1 \
    && rm -rf /var/lib/apt/lists/*

# Создать непривилегированного пользователя
RUN useradd -m -u 1000 scanner && \
    mkdir -p /app/config /scan /reports && \
    chown -R scanner:scanner /app /scan /reports

WORKDIR /app

# Скопировать бинарник и конфиг
COPY --from=builder --chown=scanner:scanner /build/build/secret_detector /app/
COPY --from=builder --chown=scanner:scanner /build/config/patterns.json /app/config/

# Переключиться на непривилегированного пользователя
USER scanner

# Volumes для данных
VOLUME ["/scan", "/reports"]

# Entrypoint
ENTRYPOINT ["/app/secret_detector"]
CMD ["--help"]

# Метаданные
LABEL maintainer="Berdnikov Alexey"
LABEL version="1.0.0"
LABEL description="Secret Detector - Find secrets in your code"
