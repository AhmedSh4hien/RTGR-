cmake_minimum_required (VERSION 3.10)
cmake_policy(VERSION 3.10)

MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO() # from: https://stackoverflow.com/questions/7787823/cmake-how-to-get-the-name-of-all-subdirectories-of-a-directory


project (RTG1)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

if(WIN32)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
endif(WIN32)

#link_directories(${CMAKE_SOURCE_DIR}/lib)
#list(APPEND CMAKE_CXX_FLAGS "-std=c++17")

# find packages using vcpkg
# make sure you set the DCMAKE_TOOLCHAIN_FILE to vcpkg
# e.g., cmake  --build .. "-DCMAKE_TOOLCHAIN_FILE=C:\dev\vcpkg\scripts\buildsystems\vcpkg.cmake"
find_package(assimp CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

SUBDIRLIST(SUBDIRS "${CMAKE_SOURCE_DIR}/src")

FOREACH(subdir ${SUBDIRS})
    file(GLOB SOURCE
            "src/${subdir}/*.h"
            "src/${subdir}/*.cpp"
            "src/${subdir}/*.vs"
            "src/${subdir}/*.fs"
            "src/${subdir}/*.gs"
            "src/${subdir}/*.glsl"
            "src/${subdir}/*.frag"
            "src/${subdir}/*.vert"
        )
    set(NAME "${subdir}")
    add_executable(${NAME} ${SOURCE})

    set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/")
    if ( MSVC ) # write all exe files into bin so relative paths are working! see https://stackoverflow.com/questions/8848268/how-to-not-add-release-or-debug-to-output-path
      set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/")
      set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/")
      set_target_properties(${NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/")
    endif ( MSVC )

    target_link_libraries(${NAME} PRIVATE assimp::assimp)
    target_link_libraries(${NAME} PRIVATE glfw)
    target_link_libraries(${NAME} PRIVATE glad::glad)
    target_link_libraries(${NAME} PRIVATE glm::glm)
    target_link_libraries(${NAME} PRIVATE imgui::imgui)
    target_link_libraries(${NAME} PRIVATE ${OPENGL_gl_LIBRARY})

    set_property(TARGET ${NAME} PROPERTY CXX_STANDARD 17) # using the c++17 standard


ENDFOREACH(subdir)


include_directories(${CMAKE_SOURCE_DIR}/include)