FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# 使用国内镜像源
RUN sed -i 's|http://archive.ubuntu.com/ubuntu/|http://mirrors.aliyun.com/ubuntu/|g' /etc/apt/sources.list \
    && sed -i 's|http://security.ubuntu.com/ubuntu|http://mirrors.aliyun.com/ubuntu|g' /etc/apt/sources.list

# 安装基础工具
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates gnupg curl \
    && apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake ninja-build gdb git unzip wget sudo software-properties-common \
    openjdk-17-jdk maven gradle \
    device-tree-compiler rsync \
    gperf \
    ccache \
    dfu-util \
    python3-dev \
    python3-pip \
    python3-setuptools \
    python3-tk \
    python3-wheel \
    xz-utils \
    file \
    bear \
    make \
    gcc \
    clang clangd lldb \
    mosquitto \
    mosquitto-clients \
    gcc-multilib \
    g++-multilib \
    libsdl2-dev \    
    python3.11 python3-venv \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# 安装Node.js二进制包
RUN wget https://npmmirror.com/mirrors/node/v20.18.0/node-v20.18.0-linux-x64.tar.xz -O /tmp/node.tar.xz \
    && tar -xJf /tmp/node.tar.xz -C /usr/local --strip-components=1 \
    && rm /tmp/node.tar.xz \
    && ln -s /usr/local/bin/node /usr/local/bin/nodejs

# 安装npm全局包
RUN npm config set registry https://registry.npmmirror.com \
    && npm install -g @google/gemini-cli yarn typescript eslint

# 安装Go（使用国内镜像）
RUN wget https://golang.google.cn/dl/go1.25.0.linux-amd64.tar.gz -O /tmp/go.tar.gz \
    && tar -C /usr/local -xzf /tmp/go.tar.gz \
    && rm /tmp/go.tar.gz

# 设置Go环境变量
ENV PATH="/root/go/bin:/usr/local/go/bin:${PATH}"
ENV GOPROXY=https://goproxy.cn,direct
ENV GOPATH=/root/go

# 安装Go开发工具（分开安装避免冲突）
RUN go install golang.org/x/tools/gopls@latest

RUN go install github.com/go-delve/delve/cmd/dlv@latest

RUN go install honnef.co/go/tools/cmd/staticcheck@latest

# 安装Air (热重载工具) - 使用正确的仓库
RUN go install github.com/air-verse/air@latest

# 安装Templ (HTML模板工具)
RUN go install github.com/a-h/templ/cmd/templ@latest

# 安装其他常用的Go开发工具（分开安装）
RUN go install github.com/cespare/reflex@latest

RUN go install golang.org/x/tools/cmd/goimports@latest

RUN go install github.com/segmentio/golines@latest

RUN go install mvdan.cc/gofumpt@latest

# 安装ARM工具链

ENV PATH="/opt/arm-toolchain/bin:${PATH}" 
 
ENV ZEPHYR_TOOLCHAIN_VARIANT=cross-compile
ENV CROSS_COMPILE=/opt/arm-toolchain/bin/arm-none-eabi-

WORKDIR /workspaces
CMD ["/bin/bash"]