#!/bin/bash
# 命令执行失败时，脚本会退出
set -e
SCRIPT_DIR=$(pwd)
# 如果不存在则创建
mkdir -p "$SCRIPT_DIR/build"
cd "$SCRIPT_DIR/build"
cmake ..
make

cd "$SCRIPT_DIR"
