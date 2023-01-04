# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(multiple_lcd)

# As the upstream LVGL library has build warnings in esp-idf build system, this is only for temporarily workaround
# Will remove this workaround when upstream LVGL fixes the warnings in the next release
idf_component_get_property(lvgl_lib lvgl__lvgl COMPONENT_LIB)
target_compile_options(${lvgl_lib} PRIVATE "-O2" "-Wno-empty-body" "-Wno-strict-prototypes" "-DLV_TICK_CUSTOM_SYS_TIME_EXPR=((int64_t)esp_timer_get_time() / 1000)")