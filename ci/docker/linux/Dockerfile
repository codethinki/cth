FROM cachyos/cachyos:latest

# Keyring update is often good practice on rolling distros
RUN pacman -Syu --noconfirm archlinux-keyring cachyos-keyring && \
    pacman -Syu --noconfirm

# Install base tools
RUN pacman -S --noconfirm \
    base-devel \
    git \
    cmake \
    ninja \
    wget \
    unzip \
    tar \
    zip \
    curl \
    python

# Install LLVM/Clang Trunk (Git versions)
# CachyOS repositories or Chaotic AUR (usually enabled in CachyOS) provide these prebuilt.
RUN pacman -S --noconfirm llvm-git clang-git lld-git compiler-rt-git libc++-git libc++abi-git || \
    (echo "Failed to install git versions, falling back to stable" && pacman -S --noconfirm llvm clang lld compiler-rt libc++ libc++abi)

# Install BuildCache
RUN curl -L -o buildcache.tar.gz https://gitlab.com/bits-n-bites/buildcache/-/releases/v0.31.7/downloads/buildcache-linux-amd64.tar.gz && \
    tar -xzf buildcache.tar.gz && \
    mkdir -p /opt/buildcache && \
    mv buildcache/bin/buildcache /opt/buildcache/buildcache && \
    rm -rf buildcache buildcache.tar.gz

ENV PATH="/opt/buildcache:$PATH"

# Install Vcpkg
ENV VCPKG_ROOT=/opt/vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git $VCPKG_ROOT && \
    $VCPKG_ROOT/bootstrap-vcpkg.sh -disableMetrics

ENV PATH="$VCPKG_ROOT:$PATH"
