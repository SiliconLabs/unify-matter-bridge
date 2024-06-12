FROM debian:bookworm

ENV DEBIAN_FRONTEND=noninteractive

SHELL ["/bin/bash", "-c"]

WORKDIR /

ARG ARCH="arm64"

ARG TARGET_DEPS="libavahi-client-dev:${ARCH}=0.8-10 libssl-dev:${ARCH}=3.0.11-1~deb12u2 libsystemd-dev:${ARCH}=252.22-1~deb12u1 libwrap0-dev:${ARCH}=7.6.q-32 uthash-dev:${ARCH}=2.3.0-1+b1 \
  libsqlite3-dev:${ARCH}=3.40.1-2 libedit-dev:${ARCH}=3.1-20221030-2 libyaml-cpp0.7:${ARCH}=0.7.0+dfsg-8+b1 libmosquitto-dev:${ARCH}=2.0.11-1.2+deb12u1\
  libreadline-dev:${ARCH}=8.2-1.3 libncurses5-dev:${ARCH}=6.4-4 libncursesw5-dev:${ARCH}=6.4-4 libyaml-cpp-dev:${ARCH}=0.7.0+dfsg-8+b1 \
  libboost-atomic-dev:${ARCH}=1.74.0.3 libboost-chrono-dev:${ARCH}=1.74.0.3 libboost-date-time-dev:${ARCH}=1.74.0.3 \
  libboost-filesystem-dev:${ARCH}=1.74.0.3 libboost-regex-dev:${ARCH}=1.74.0.3 libboost-program-options-dev:${ARCH}=1.74.0.3 \
  libboost-serialization-dev:${ARCH}=1.74.0.3 libboost-system-dev:${ARCH}=1.74.0.3 libboost-thread-dev:${ARCH}=1.74.0.3 \
  libboost-log-dev:${ARCH}=1.74.0.3 procps:${ARCH}=2:4.0.2-3 libmbedtls-dev:${ARCH}=2.28.3-1 libglib2.0-dev:${ARCH}=2.74.6-2+deb12u2"

ARG HOST_DEPS="ninja-build=1.11.1-1 git=1:2.39.2-1* git-lfs=3.3.0-1+b5 \
  curl=7.88.1-10+deb12u5 pkg-config=1.8.1-1 \
  ruby=1:3.1 ruby-dev=1:3.1 lcov=1.16-1 \
  gcc=4:12.2.0-3 g++=4:12.2.0-3 gdb-multiarch=13.1-3 \
  build-essential=12.9 \
  clang=1:14.0-55.7~deb12u1 nlohmann-json3-dev=3.11.2-2 \
  libdbus-glib-1-dev=0.112-3 libssl-dev=3.0.11-1~deb12u2 \
  libgirepository1.0-dev=1.74.0-3 \
  fakeroot=1.31-1.2"

# configure additional arches in dpkg/apt
RUN for arch in ${ARCH} ; do \
  dpkg --add-architecture $arch ; \
  echo $arch ; \
  done

RUN apt-get update \
  && apt-get install --no-install-recommends -y curl gnupg2 ca-certificates \
  && apt-get update \
  && apt-get install --no-install-recommends -y \
  ${HOST_DEPS} \
  $(for dep in ${TARGET_DEPS} ; do \
  for arch in ${ARCH} ; do echo $dep; done ; \
  done) \
  $(for arch in ${ARCH} ; do echo crossbuild-essential-${arch} ; done)\
  && rm -rf /var/lib/apt/lists/*

# Install python 3.9
RUN curl -L https://repo.anaconda.com/miniconda/Miniconda3-py39_4.12.0-Linux-x86_64.sh --output /tmp/Miniconda.sh \
  && echo 78f39f9bae971ec1ae7969f0516017f2413f17796670f7040725dd83fcff5689 /tmp/Miniconda.sh > /tmp/Miniconda.sh.sha256 \
  && sha256sum -c /tmp/Miniconda.sh.sha256 \
  && bash /tmp/Miniconda.sh -b -p /usr/local/miniconda \
  && /usr/local/miniconda/condabin/conda init bash \
  && ln -sf /usr/local/miniconda/bin/python3.9 /usr/bin/python3 \
  && ln -sf /usr/local/miniconda/bin/pip3 /usr/bin/pip3

# Install gcvor
RUN pip3 install --no-cache-dir gcovr==5.2 deb-pkg-tools==8.4

# Rust Version to install
ENV RUST_VERSION=1.65.0
# Rust and Cargo home directories
ENV RUSTUP_HOME=/opt/rustup-home
ENV CARGO_HOME=/opt/cargo-home

# Install Rust and Cargo
RUN curl https://sh.rustup.rs -sSf --output /tmp/sh.rustup.rs \
  && chmod +x /tmp/sh.rustup.rs \
  && if [[ $ARCH == *"armhf"* ]]; then export RUST_TRIPLES="$RUST_TRIPLES armv7-unknown-linux-gnueabihf"; fi \
  && if [[ $ARCH == *"amd64"* ]]; then export RUST_TRIPLES="$RUST_TRIPLES x86_64-unknown-linux-gnu"; fi \
  && if [[ $ARCH == *"arm64"* ]]; then export RUST_TRIPLES="$RUST_TRIPLES aarch64-unknown-linux-gnu"; fi \
  && ./tmp/sh.rustup.rs -y --profile minimal --target ${RUST_TRIPLES} --default-toolchain ${RUST_VERSION}\
  && rm /tmp/sh.rustup.rs \
  && /opt/cargo-home/bin/cargo install cargo2junit --locked \
  && /opt/cargo-home/bin/cargo install --version 1.41.3 cargo-deb \
  && chmod -R a+rw ${RUSTUP_HOME} ${CARGO_HOME} \
  && find ${RUSTUP_HOME} ${CARGO_HOME} -type d -exec chmod a+x {} \;
ENV PATH="${CARGO_HOME}/bin:${PATH}"

# Install cmake 3.21.6
RUN curl -L https://github.com/Kitware/CMake/releases/download/v3.21.6/cmake-3.21.6-Linux-x86_64.sh --output /tmp/cmake-3.21.6-Linux-x86_64.sh \
  && echo d460a33c42f248388a8f2249659ad2f5eab6854bebaf4f57c1df49ded404e593  /tmp/cmake-3.21.6-Linux-x86_64.sh > /tmp/cmake-3.21.6-Linux-x86_64.sh.sha256 \
  && sha256sum -c /tmp/cmake-3.21.6-Linux-x86_64.sh.sha256 \
  && chmod +x /tmp/cmake-3.21.6-Linux-x86_64.sh \
  && /tmp/cmake-3.21.6-Linux-x86_64.sh --prefix=/usr/local --skip-license \
  && rm /tmp/cmake*
