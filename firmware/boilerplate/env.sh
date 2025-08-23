#!/bin/bash
# Zephyr Cross Compile Toolchain Setup

# 指定使用 cross-compile 工具链模式
export ZEPHYR_TOOLCHAIN_VARIANT=cross-compile

# 指定交叉编译工具链的路径和前缀
export CROSS_COMPILE=/opt/arm-toolchain/bin/arm-none-eabi-

# 如果有多版本 Python，确保 Zephyr 使用正确的
# export PATH=$HOME/.local/bin:$PATH
export ZEPHYR_BASE=/workspaces/projectX/vendor/zephyrproject/zephyr

# 输出提示
echo "Zephyr toolchain environment configured."
echo "  ZEPHYR_TOOLCHAIN_VARIANT=$ZEPHYR_TOOLCHAIN_VARIANT"
echo "  CROSS_COMPILE=$CROSS_COMPILE"
