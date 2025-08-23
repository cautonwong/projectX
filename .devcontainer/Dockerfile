FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN sed -i.bak 's|http://archive.ubuntu.com/ubuntu/|http://mirrors.aliyun.com/ubuntu/|g' /etc/apt/sources.list \
    && sed -i 's|http://security.ubuntu.com/ubuntu|http://mirrors.aliyun.com/ubuntu|g' /etc/apt/sources.list
    
RUN apt-get update && apt-get install -y \
    build-essential cmake ninja-build gdb git curl unzip wget sudo software-properties-common \
    openjdk-17-jdk maven gradle \
    python3.11 python3-pip python3-venv \
    && rm -rf /var/lib/apt/lists/*

# Rust
RUN curl https://sh.rustup.rs -sSf | sh -s -- -y
ENV PATH="/root/.cargo/bin:${PATH}"

# Go
RUN wget https://go.dev/dl/go1.25.0.linux-amd64.tar.gz \
    && tar -C /usr/local -xzf go1.25.0.linux-amd64.tar.gz \
    && rm go1.25.0.linux-amd64.tar.gz
ENV PATH="/usr/local/go/bin:${PATH}"

# Node.js
RUN curl -fsSL https://deb.nodesource.com/setup_20.x | bash - \
    && apt-get install -y nodejs\
    && npm install -g @google/gemini-cli

ENV ARM_TOOLCHAIN_VERSION="13.2.Rel1"
RUN wget "https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-${ARM_TOOLCHAIN_VERSION}-x86_64-arm-none-eabi.tar.xz" -O /tmp/arm-toolchain.tar.xz \
    && mkdir -p /opt/arm-toolchain \
    && tar -xf /tmp/arm-toolchain.tar.xz -C /opt/arm-toolchain --strip-components=1 \
    && rm /tmp/arm-toolchain.tar.xz
ENV PATH="/opt/arm-toolchain/bin:${PATH}"    

WORKDIR /workspaces
CMD ["/bin/bash"]
