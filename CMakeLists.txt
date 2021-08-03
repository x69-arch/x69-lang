cmake_minimum_required(VERSION 3.8)




set(libroot "./lib/")


project(x69-lang VERSION 0.0.1)

include("CMakeUtils.cmake")

add_executable(${PROJECT_NAME} "source/main.cpp" "source/util/lexer.cpp")

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_ROOT="${CMAKE_CURRENT_LIST_DIR}/")
target_include_directories(${PROJECT_NAME} PRIVATE "source")


#
# Dependency handling
#

# jclib
ADD_GIT_DEPENDENCY("./lib/jclib/" JCLib "https://github.com/JonathanCline/JCLib" dev)




target_link_libraries(${PROJECT_NAME} PUBLIC JCLib)
