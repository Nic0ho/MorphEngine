add_library(MorphEngine STATIC
    src/MorphUtil.cpp)

target_include_directories(MorphEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)