# Copyright (c) 2024 Aaron Chan
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.22)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(Outlaw-GPS-Tracker)

zephyr_syscall_include_directories(include)
zephyr_include_directories(include)

add_subdirectory(lib)
