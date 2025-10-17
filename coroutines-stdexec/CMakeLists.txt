##################
# Target
get_filename_component(DIRECTORY_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" TARGET_MAIN ${DIRECTORY_NAME})
set(TARGET_MAIN tests-${TARGET_MAIN})

####################
# Sources & headers
aux_source_directory(. SRC_LIST)
file(GLOB HEADERS_LIST "*.h" "*.hpp")

Include(FetchContent)

FetchContent_Declare(
  stdexec
  GIT_REPOSITORY https://github.com/NVIDIA/stdexec.git
  GIT_TAG        1f6379682dd1598c9b48313fa6dfdae620bc8535
)

FetchContent_MakeAvailable(stdexec)

add_executable(${TARGET_MAIN} ${SRC_LIST} ${HEADERS_LIST})
target_link_libraries(${TARGET_MAIN} PRIVATE Catch2::Catch2WithMain STDEXEC::stdexec)
add_test(NAME ${TARGET_MAIN}
         COMMAND ${TARGET_MAIN})