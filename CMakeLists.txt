cmake_minimum_required(VERSION 3.0)
enable_language(C)

project(yeast)

set(CMAKE_POSITION_INDEPENDENT_CODE TRUE CACHE BOOL "pic" FORCE)

set(TREESITTER_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/external/tree-sitter")
set(TREESITTER_BUILD "${TREESITTER_ROOT}/out/Release/obj.target")
set(TREESITTER_INCLUDE "${TREESITTER_ROOT}/include")
set(LIBRUNTIME_FILE "${TREESITTER_BUILD}/libruntime.a")
set(LIBCOMPILER_FILE "${TREESITTER_BUILD}/libcompiler.a")

add_custom_command(
  OUTPUT ${LIBRUNTIME_FILE} ${LIBCOMPILER_FILE}
  COMMAND script/build-tree-sitter
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

add_custom_target(tree_sitter DEPENDS ${LIBRUNTIME_FILE} ${LIBCOMPILER_FILE})

add_library(runtime STATIC IMPORTED)
add_dependencies(runtime tree_sitter)

set_target_properties(
  runtime PROPERTIES
  IMPORTED_LOCATION ${LIBRUNTIME_FILE}
  INTERFACE_INCLUDE_DIRECTORIES ${TREESITTER_INCLUDE}
  )

add_library(compiler STATIC IMPORTED)
add_dependencies(compiler tree_sitter)

set_target_properties(
  compiler PROPERTIES
  IMPORTED_LOCATION ${LIBCOMPILER_FILE}
  INTERFACE_INCLUDE_DIRECTORIES ${TREESITTER_INCLUDE}
  )

file(GLOB YEAST_SRCS src/*.c)
set(YEAST_PARSERS
  "${CMAKE_CURRENT_SOURCE_DIR}/external/json/src/parser.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/external/python/src/parser.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/external/python/src/scanner.cc"
  )

add_library(yeast SHARED ${YEAST_SRCS} ${YEAST_PARSERS})
set_target_properties(yeast PROPERTIES C_STANDARD 99)

# Emacs looks for .so on linux and OSX.
# By default CMake builds .dylib on OSX, so override that.
if(APPLE)
  set_target_properties(yeast PROPERTIES SUFFIX .so)
endif(APPLE)

# if(CMAKE_COMPILER_IS_GNUCC)
#   target_compile_options(yeast PRIVATE -Wall -Wextra)
# endif(CMAKE_COMPILER_IS_GNUCC)

target_link_libraries(yeast runtime)
target_include_directories(yeast SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/external/uthash")

# add_custom_command(
#   TARGET yeast POST_BUILD
#   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:yeast> ${CMAKE_CURRENT_SOURCE_DIR}
#   )
