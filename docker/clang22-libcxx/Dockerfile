FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      bash \
      ca-certificates \
      curl \
      gdb \
      git \
      gnupg \
      lsb-release \
      make \
      perl \
      pkg-config \
      python3 \
      rsync \
      software-properties-common \
      wget && \
    rm -rf /var/lib/apt/lists/*

RUN wget -O /tmp/llvm.sh https://apt.llvm.org/llvm.sh && \
    chmod +x /tmp/llvm.sh && \
    /tmp/llvm.sh 22 && \
    rm -f /tmp/llvm.sh && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
      clang-22 \
      libc++-22-dev \
      libc++abi-22-dev \
      libunwind-22-dev \
      lld-22 && \
    rm -rf /var/lib/apt/lists/*

ENV CC=/usr/bin/clang-22
ENV CXX=/usr/bin/clang++-22
ENV CPPGM_HOST_CXX=/usr/bin/clang++-22
ENV MAKEFLAGS="CXX=/usr/bin/clang++-22 CPPGM_HOST_CXX=/usr/bin/clang++-22 CPPGM_STDLIB_FLAGS=-stdlib=libc++"

WORKDIR /work

CMD ["/bin/bash"]
