file(GLOB_RECURSE all_srcs CONFIGURE_DEPENDS *.cpp)
add_executable(main ${all_srcs})
