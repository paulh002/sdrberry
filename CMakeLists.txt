cmake_minimum_required(VERSION 2.7)
set(CMAKE_VERBOSE_MAKEFILE ON)
project(sdrberry)
file(GLOB_RECURSE SOURCES ./lvgl/src/*.c)
set(LIBRARIES_FROM_REFERENCES -lpthread -lbluetooth -lSoapySDR -latomic -ltinyb -lrtaudio -lliquid -lasound ->
include_directories(lvgly)
add_executable(sdrberry ${PROJECT_SOURCE_DIR}/sdrberry.cpp Agc_class.cpp AMDemodulator.cpp AMModulator.cpp Au>
target_compile_definitions(sdrberry PRIVATE __LINUX_ALSA__ LV_LVGL_H_INCLUDE_SIMPLE)
target_compile_options(sdrberry PRIVATE)
target_include_directories(sdrberry PRIVATE /usr/local/include/liquid ${PROJECT_SOURCE_DIR}/lvgl)
set_property(TARGET sdrberry PROPERTY CXX_STANDARD 17)
target_link_libraries(sdrberry "${LIBRARIES_FROM_REFERENCES}")
install(TARGETS sdrberry DESTINATION /usr/local/bin)
