# This is required. target system name
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# specify the cross compiler
set(CMAKE_C_COMPILER /usr/aarch64-linux-gnu/bin/aarch64-none-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER /usr/aarch64-linux-gnu/bin/aarch64-none-linux-gnu-g++)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
