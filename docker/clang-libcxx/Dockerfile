# Pinned toolchain for the clang/libc++ cell.
#
# A libc++ reference is only reproducible against a fixed library: libc++ 21
# emits C++20 conditional explicit outside its C++03 branch, and earlier
# releases do not, so the constructs a header run exercises move between
# versions.  Pin the toolchain here and keep test fixtures written against the
# language construct rather than against the header that found it.
FROM ubuntu:24.04

ARG LLVM_VERSION=21

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      bash \
      ca-certificates \
      g++ \
      git \
      gnupg \
      lsb-release \
      make \
      perl \
      python3 \
      software-properties-common \
      wget && \
    rm -rf /var/lib/apt/lists/*

RUN wget -O /tmp/llvm.sh https://apt.llvm.org/llvm.sh && \
    chmod +x /tmp/llvm.sh && \
    /tmp/llvm.sh ${LLVM_VERSION} && \
    rm -f /tmp/llvm.sh && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
      clang-${LLVM_VERSION} \
      libc++-${LLVM_VERSION}-dev \
      libc++abi-${LLVM_VERSION}-dev \
      libunwind-${LLVM_VERSION}-dev && \
    rm -rf /var/lib/apt/lists/*

# The cell targets in the root Makefile take the compiler from CLANG_CXX, so
# the image only has to name it.  g++ stays installed so the default cell can
# still be built and compared inside the same image.
ENV CLANG_CXX=/usr/bin/clang++-${LLVM_VERSION}

WORKDIR /work

CMD ["/bin/bash"]
