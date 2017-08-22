cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(camtest)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIBRARY_DIRS})
add_definitions(${OpenCV_DEFINITIONS})

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK2 REQUIRED gtk+-2.0)
include_directories(${GTK2_INCLUDE_DIRS})
link_directories(${GTK2_LIBRARY_DIRS})
add_definitions(${GTK2_CFLAGS_OTHER})


# Use PkgConfig to detect gtkglarea
pkg_check_modules(GTKGL REQUIRED gtkgl-2.0)
include_directories(${GTKGL_INCLUDE_DIRS})
message(STATUS "${GTKGL_INCLUDE_DIRS}")
link_directories(${GTKGL_LIBRARY_DIRS})
add_definitions(${GTKGL_CFLAGS_OTHER})


find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
find_package(GLUT REQUIRED)

include_directories(${GLUT_INCLUDE_DIRS})
link_directories(${GLUT_LIBRARY_DIRS})
add_definitions(${GLUT_DEFINITIONS})
if(NOT GLUT_FOUND)
    message(ERROR " GLUT not found!")
endif(NOT GLUT_FOUND)

add_executable (camtest camtest.cpp)

set(OPENCV_LIBS opencv_core opencv_imgproc opencv_calib3d opencv_video opencv_features2d opencv_ml opencv_highgui opencv_objdetect opencv_contrib opencv_legacy opencv_gpu)
#set(OPENCV_LIBS opencv_highgui)

target_link_libraries (camtest ${OPENCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GTK2_LIBRARIES} ${GTKGL_LIBRARIES})
