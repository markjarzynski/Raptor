find_path(IMGUI_INCLUDE_DIR NAMES imgui.h PATHS "/usr/include/" "/usr/local/include/" "${CMAKE_SOURCE_DIR}/Common/External/imgui/")

find_package_handle_standard_args(IMGUI REQUIRED_VARS IMGUI_INCLUDE_DIR VERSION_VAR IMGUI_VERSION)