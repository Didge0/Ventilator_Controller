# =========================
# Étape compilation
# =========================

FROM debian:bookworm-slim AS builder

RUN apt update && apt install -y \
    g++ \
    cmake \
    make \
    libgpiod-dev

WORKDIR /app

COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build -j2

# =========================
# Étape runtime
# =========================

FROM debian:bookworm-slim

RUN apt update && apt install -y \
    libgpiod2 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/bin/Release/ventilator_controller .

CMD ["./ventilator_controller"]