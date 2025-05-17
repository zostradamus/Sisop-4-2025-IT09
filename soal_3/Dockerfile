# ---- Stage 1: Compile antink.c into a standalone FUSE binary ----
FROM ubuntu:22.04 AS builder

LABEL stage=builder
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      build-essential \
      libfuse3-dev \
      pkg-config \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY antink.c .

# Compile dengan flags ekstra untuk strict warnings
RUN gcc \
      -std=gnu11 \
      -Wall -Wextra -O2 \
      antink.c \
      -o antink-fuse \
      `pkg-config fuse3 --cflags --libs`

# ---- Stage 2: Runtime image with only FUSE installed ----
FROM ubuntu:22.04

LABEL stage=runtime
ENV DEBIAN_FRONTEND=noninteractive

# Hanya install runtime lib untuk FUSE
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      fuse3 \
 && rm -rf /var/lib/apt/lists/*

# Salin binary hasil compile
COPY --from=builder /build/antink-fuse /usr/local/bin/antink-fuse

# Buat direktori kerja dan mount points
RUN mkdir -p /srv/antiNK/original \
             /srv/antiNK/mount \
             /srv/antiNK/logs

VOLUME ["/srv/antiNK/original", "/srv/antiNK/mount", "/srv/antiNK/logs"]

# Jalankan FUSE dalam foreground, dengan log ke /srv/antiNK/logs/it24.log
ENTRYPOINT ["/usr/local/bin/antink-fuse"]
CMD ["--root", "/srv/antiNK/original", \
     "--mountpoint", "/srv/antiNK/mount", \
     "--logfile", "/srv/antiNK/logs/it24.log", \
     "-f", "-s"]
