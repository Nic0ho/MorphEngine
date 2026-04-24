add_library(MorphEngine STATIC
    src/MorphUtil.cpp
    src/MorphCamera.cpp)

target_include_directories(MorphEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(MorphEngine PUBLIC glm)