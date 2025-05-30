cmake_minimum_required(VERSION 3.16)
project(app_yolov2_cam
        VERSION 1.0.0 
        DESCRIPTION "Renesas SOM YoloV2 Demo"
        LANGUAGES CXX C
)
set(PROJECT_NAME app_yolov2_cam)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

include(GNUInstallDirs)

find_package(mmngr REQUIRED COMPONENTS mmngr mmngrbuf)
find_package(v4l2subdev REQUIRED)
find_package(argparse CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS opencv_imgcodecs opencv_imgproc opencv_core opencv_highgui)
pkg_check_modules(MEDIACTL libmediactl IMPORTED_TARGET GLOBAL)

# Add the source to the executable of this project.
add_executable(${PROJECT_NAME}
    main.cpp
    camera.cpp
    image.cpp
    box.cpp
    ascii.cpp
    drp_ai_accelerator.cpp
    dma_buffer.cpp
    video_server.cpp
    media_device.cpp
    robot_control.cpp
    move_to_target.cpp
    define.cpp
    wayland.cpp)

target_compile_definitions(${PROJECT_NAME} PRIVATE NDEBUG)
target_compile_options(${PROJECT_NAME} PRIVATE -Ofast -Wall -g)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_include_directories(${PROJECT_NAME} PRIVATE "package")
target_link_libraries(${PROJECT_NAME} PRIVATE
    pthread
    stdc++fs
    argparse::argparse
    nlohmann_json::nlohmann_json
    mmngr::mmngr
    mmngr::mmngrbuf 
    opencv_core
    opencv_imgcodecs 
    opencv_imgproc  
    opencv_highgui
    PkgConfig::MEDIACTL
    v4l2subdev::v4l2subdev
    wayland-client
    wayland-egl
    EGL
    GLESv2)


# Installation
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION /home/demo)

install(DIRECTORY ../exe/yolov2_cam DESTINATION /home/demo
        FILE_PERMISSIONS
            OWNER_READ OWNER_EXECUTE OWNER_WRITE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE
        DIRECTORY_PERMISSIONS
            OWNER_READ OWNER_EXECUTE OWNER_WRITE
            GROUP_READ GROUP_EXECUTE GROUP_WRITE
            WORLD_READ WORLD_EXECUTE
)
