zephyr_library()

FILE(GLOB sources *.c *.cpp)
zephyr_library_sources(${sources})
