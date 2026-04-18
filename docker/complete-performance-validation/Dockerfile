FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive
ARG ZIG_VERSION=0.14.1
ARG TARGETARCH

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        bash \
        build-essential \
        ca-certificates \
        cmake \
        curl \
        gcc-13 \
        g++-13 \
        git \
        libc6-dev \
        make \
        pkg-config \
        python3 \
        xz-utils \
    && rm -rf /var/lib/apt/lists/*

RUN case "$TARGETARCH" in \
        arm64) zig_arch="aarch64" ;; \
        amd64) zig_arch="x86_64" ;; \
        *) echo "Unsupported Docker target architecture: $TARGETARCH" >&2; exit 1 ;; \
    esac \
    && curl -L --fail "https://ziglang.org/download/${ZIG_VERSION}/zig-${zig_arch}-linux-${ZIG_VERSION}.tar.xz" -o /tmp/zig.tar.xz \
    && mkdir -p /opt/zig \
    && tar -xf /tmp/zig.tar.xz -C /opt/zig --strip-components=1 \
    && ln -sf /opt/zig/zig /usr/local/bin/zig \
    && rm -f /tmp/zig.tar.xz

ENV CC=gcc-13
ENV CXX=g++-13
ENV PATH="/opt/zig:${PATH}"

WORKDIR /workspace
CMD ["bash"]
