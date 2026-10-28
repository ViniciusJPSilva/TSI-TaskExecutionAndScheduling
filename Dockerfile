# Etapa de build: compila o tes com GCC em ambiente Linux (requer API POSIX: fork/wait).
FROM gcc:14 AS build
WORKDIR /src
COPY tes.c tes.h ./
RUN gcc -std=gnu11 -Wall -O2 -o tes tes.c

# Etapa final: imagem enxuta contendo apenas o binário e os programas LPAS de exemplo.
FROM debian:bookworm-slim
WORKDIR /app
COPY --from=build /src/tes /usr/local/bin/tes
COPY *.lpas ./
CMD ["tes"]
