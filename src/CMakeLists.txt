cmake_minimum_required (VERSION 3.6)
project (ProjectPokemon)

set(CMAKE_CXX_STANDARD 11)

file(GLOB SOURCE_FILES "Core/*.hpp" "Core/*.cpp" "*.hpp" "*.cpp")
configure_file(UserCrawler.py UserCrawler.py COPYONLY)
configure_file(TestCrawler.py TestCrawler.py COPYONLY)

add_executable(ProjectPokemon ${SOURCE_FILES})
