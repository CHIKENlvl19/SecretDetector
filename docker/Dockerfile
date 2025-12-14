# stage 1: build
FROM archlinux:latest AS builder

RUN pacman -Syu --noconfirm \
    && pacman -S --noconfirm base-devel cmake gcc nlohmann-json spdlog

WORKDIR /app

# копируем весь проект
COPY . .

RUN mkdir -p build \
    && cd build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release \
    && make -j$(nproc)

# stage 2: runtime (минимальный образ)
FROM archlinux:latest AS runtime

RUN pacman -Syu --noconfirm \
    && pacman -S --noconfirm nlohmann-json spdlog \
    && pacman -Scc --noconfirm

WORKDIR /app

COPY --from=builder /app/build/secret_detector /usr/local/bin/secret_detector
COPY config /app/config

ENTRYPOINT ["secret_detector"]
CMD ["--help"]
