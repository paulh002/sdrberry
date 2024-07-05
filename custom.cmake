# Option to define LV_LVGL_H_INCLUDE_SIMPLE, default: ON
option(LV_LVGL_H_INCLUDE_SIMPLE
       "Use #include \"lvgl.h\" instead of #include \"../../lvgl.h\"" ON)

# Option to define LV_CONF_INCLUDE_SIMPLE, default: ON
option(LV_CONF_INCLUDE_SIMPLE
       "Simple include of \"lv_conf.h\" and \"lv_drv_conf.h\"" ON)

# Option to set LV_CONF_PATH, if set parent path LV_CONF_DIR is added to
# includes
option(LV_CONF_PATH "Path defined for lv_conf.h")
get_filename_component(LV_CONF_DIR ${LV_CONF_PATH} DIRECTORY)

# Option to build shared libraries (as opposed to static), default: OFF
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Option to build examples, default: OFF
option(BUILD_LVGL_EXAMPLES "Build lvgl examples" OFF)

# Option to build demos, default: OFF
option(BUILD_LVGL_DEMOS "Build lvgl demos" OFF)

file(GLOB_RECURSE SOURCES ${LVGL_ROOT_DIR}/src/*.c)

if (BUILD_SHARED_LIBS)
  add_library(lvgl SHARED ${SOURCES})
else()
  add_library(lvgl STATIC ${SOURCES})
endif()

add_library(lvgl::lvgl ALIAS lvgl)

# Define interface libraries for examples and demos
add_library(lvgl_examples INTERFACE)
add_library(lvgl::examples ALIAS lvgl_examples)
add_library(lvgl_demos INTERFACE)
add_library(lvgl::demos ALIAS lvgl_demos)

# Conditionally add sources to examples and demos if the options are enabled
if (BUILD_LVGL_EXAMPLES)
    file(GLOB_RECURSE EXAMPLE_SOURCES ${LVGL_ROOT_DIR}/examples/*.c)
    set_property(TARGET lvgl_examples APPEND PROPERTY INTERFACE_SOURCES ${EXAMPLE_SOURCES})
    target_include_directories(lvgl_examples SYSTEM
                               INTERFACE ${LVGL_ROOT_DIR}/examples)
    target_link_libraries(lvgl_examples INTERFACE lvgl)
endif()

if (BUILD_LVGL_DEMOS)
    file(GLOB_RECURSE DEMO_SOURCES ${LVGL_ROOT_DIR}/demos/*.c)
    set_property(TARGET lvgl_demos APPEND PROPERTY INTERFACE_SOURCES ${DEMO_SOURCES})
    target_include_directories(lvgl_demos SYSTEM
                               INTERFACE ${LVGL_ROOT_DIR}/demos)
    target_link_libraries(lvgl_demos INTERFACE lvgl)
endif()

target_compile_definitions(
  lvgl PUBLIC $<$<BOOL:${LV_LVGL_H_INCLUDE_SIMPLE}>:LV_LVGL_H_INCLUDE_SIMPLE>
              $<$<BOOL:${LV_CONF_INCLUDE_SIMPLE}>:LV_CONF_INCLUDE_SIMPLE>)

# Include root and optional parent path of LV_CONF_PATH
target_include_directories(lvgl SYSTEM PUBLIC ${LVGL_ROOT_DIR} ${LV_CONF_DIR})

# Library and headers can be installed to system using make install
file(GLOB LVGL_PUBLIC_HEADERS "${CMAKE_SOURCE_DIR}/lv_conf.h"
     "${CMAKE_SOURCE_DIR}/lvgl.h")

if("${LIB_INSTALL_DIR}" STREQUAL "")
  set(LIB_INSTALL_DIR "lib")
endif()
if("${INC_INSTALL_DIR}" STREQUAL "")
  set(INC_INSTALL_DIR "include/lvgl")
endif()

install(
  DIRECTORY "${CMAKE_SOURCE_DIR}/src"
  DESTINATION "${CMAKE_INSTALL_PREFIX}/${INC_INSTALL_DIR}/"
  FILES_MATCHING
  PATTERN "*.h")

install(
  FILES "${LV_CONF_PATH}"
  DESTINATION "${CMAKE_INSTALL_PREFIX}/${INC_INSTALL_DIR}/../"
  RENAME "lv_conf.h"
  OPTIONAL)

configure_file("${LVGL_ROOT_DIR}/lvgl.pc.in" lvgl.pc @ONLY)

install(
  FILES "${CMAKE_BINARY_DIR}/lvgl.pc"
  DESTINATION "${LIB_INSTALL_DIR}/pkgconfig/")

set_target_properties(
  lvgl
  PROPERTIES OUTPUT_NAME lvgl
             ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
             LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
             RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
             PUBLIC_HEADER "${LVGL_PUBLIC_HEADERS}")

install(
  TARGETS lvgl
  ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
  LIBRARY DESTINATION "${LIB_INSTALL_DIR}"
  RUNTIME DESTINATION "${LIB_INSTALL_DIR}"
  PUBLIC_HEADER DESTINATION "${INC_INSTALL_DIR}")

# Conditionally install the example and demo targets
if (BUILD_LVGL_EXAMPLES)
  install(TARGETS lvgl_examples
          ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
          LIBRARY DESTINATION "${LIB_INSTALL_DIR}"
          RUNTIME DESTINATION "${LIB_INSTALL_DIR}")
endif()

if (BUILD_LVGL_DEMOS)
  install(TARGETS lvgl_demos
          ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
          LIBRARY DESTINATION "${LIB_INSTALL_DIR}"
          RUNTIME DESTINATION "${LIB_INSTALL_DIR}")
endif()
