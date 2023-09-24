find_path(GLFW_INCLUDE_DIR NAMES GLFW/glfw3.h PATHS "/usr/include/" "/usr/local/include/" "${CMAKE_SOURCE_DIR}/Common/External/glfw/include")

find_package_handle_standard_args(GLFW REQUIRED_VARS GLFW_INCLUDE_DIR VERSION_VAR GLFW_VERSION)