cmake_minimum_required(VERSION 3.8)

set(APP_NAME MyGame)
project(${APP_NAME} CXX)

enable_language(C)

set(CMAKE_CXX_STANDARD 14)
set(USE_SE_V8 ON)

set(cocosdir C:/Github/cocos2d-x-lite)
include(${cocosdir}/CMakeLists.txt)


set(PROJ_SOURCES 
    Classes/Game.h
    Classes/Game.cpp
    Classes/jsb_module_register.cpp
)

if(WINDOWS)
    list(APPEND PROJ_SOURCES
        proj.win32/AppDelegate.h
        proj.win32/AppDelegate.cpp
        proj.win32/View-win32.h
        proj.win32/View-win32.cpp
        proj.win32/main.h
        proj.win32/main.cpp
        proj.win32/resource.h
    )
endif()



add_executable(${APP_NAME} ${PROJ_SOURCES})
target_link_libraries(${APP_NAME} cocos2d)
target_include_directories(${APP_NAME} PRIVATE 
    Classes
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} PREFIX "Source Files" FILES ${PROJ_SOURCES})

if(WINDOWS)
    add_custom_command(TARGET ${APP_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ../../../jsb-adapter $<TARGET_FILE_DIR:MyGame>/jsb-adapter
        COMMAND ${CMAKE_COMMAND} -E copy_directory ../../../res $<TARGET_FILE_DIR:MyGame>/res
        COMMAND ${CMAKE_COMMAND} -E copy_directory ../../../src $<TARGET_FILE_DIR:MyGame>/src
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ../../../main.js $<TARGET_FILE_DIR:MyGame>/main.js
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ../../../project.json $<TARGET_FILE_DIR:MyGame>/project.json
    )
    target_include_directories(${APP_NAME} PRIVATE 
        proj.win32
    )
endif()

if(MSVC) 
    foreach(item ${WIN32_DLLS})
        get_filename_component(filename ${item} NAME)
        get_filename_component(abs ${item} ABSOLUTE)
        add_custom_command(TARGET ${APP_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${abs} $<TARGET_FILE_DIR:MyGame>/${filename}
        )
    endforeach()
    target_compile_options(${APP_NAME} PUBLIC /MP)
    target_link_options(${APP_NAME} PRIVATE /SUBSYSTEM:WINDOWS)
endif()