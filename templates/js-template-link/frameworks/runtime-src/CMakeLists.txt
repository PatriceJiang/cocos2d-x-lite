cmake_minimum_required(VERSION 3.8)

set(APP_NAME MyGame)
project(${APP_NAME} CXX)

enable_language(C ASM)

set(CMAKE_CXX_STANDARD 14)
set(USE_SE_V8 ON
set(USE_V8_DEBUGGER ON))
set(USE_VIDEOPLAYER OFF)
set(USE_WEBVIEW OFF)
set(USE_SOCKET ON)
set(USE_TIFF ON)
set(USE_AUDIO           ON)     #Enable AudioEngine
set(USE_SOCKET          ON)     #Enable WebSocket & SocketIO

#set(cocosdir "${COCOS_X_ROOT}")
set(cocosdir /Users/cocos/Github/cocos2d-x-lite)
include(${cocosdir}/CMakeLists.txt)

if(ANDROID)
    set(LIB_NAME cocos2djs)
elseif(MACOSX)
    set(LIB_NAME ${APP_NAME}-desktop)
elseif(IOS)
    set(LIB_NAME ${APP_NAME}-mobile)
else()
    set(LIB_NAME ${APP_NAME})
endif()

set(UI_RESOURCES)
set(PROJ_SOURCES 
    Classes/Game.h
    Classes/Game.cpp
    Classes/jsb_module_register.cpp
)

if(WINDOWS)
    list(APPEND PROJ_SOURCES
        proj.win32/AppDelegate.h
        proj.win32/AppDelegate.cpp
        proj.win32/main.h
        proj.win32/main.cpp
        proj.win32/resource.h
    )
elseif(MACOSX)
    set(UI_RESOURCES
        proj.ios_mac/mac/Assets.xcassets/Contents.json
        proj.ios_mac/mac/Assets.xcassets/AppIcon.appiconset/Contents.json
        proj.ios_mac/mac/UserConfigMac.release.xcconfig
        proj.ios_mac/mac/UserConfigMac.debug.xcconfig
        proj.ios_mac/mac/Info.plist
        proj.ios_mac/mac/Icon.icns
    )
    list(APPEND PROJ_SOURCES
        proj.ios_mac/mac/main.m
        proj.ios_mac/mac/ViewController.mm
        proj.ios_mac/mac/ViewController.h
        proj.ios_mac/mac/Prefix.pch
        proj.ios_mac/mac/AppDelegate.mm
        proj.ios_mac/mac/AppDelegate.h
        ${UI_RESOURCES}
    )
elseif(IOS)
endif()


set(GAME_RES_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../)
set(GAME_RES_FILES
    ${GAME_RES_ROOT}/main.js
    ${GAME_RES_ROOT}/project.json
)
set(GAME_RES_FOLDERS 
    ${GAME_RES_ROOT}/jsb-adapter
    ${GAME_RES_ROOT}/subpackages
    ${GAME_RES_ROOT}/res
    ${GAME_RES_ROOT}/src
)
set(GAME_RES_ALL
    ${GAME_RES_FILES}
    ${GAME_RES_FOLDERS}
)
set(asset_files)
foreach(res ${GAME_RES_ALL})
    set(res_list)
    if(IS_DIRECTORY ${res})
        file(GLOB_RECURSE res_list "${res}/*")
    else()
        set(res_list ${res})
    endif()
    foreach(res ${res_list})
        get_filename_component(res_abs ${res} ABSOLUTE)
        file(RELATIVE_PATH res_rel ${GAME_RES_ROOT} ${res_abs})
        get_filename_component(res_dir ${res_rel} PATH)
        set_source_files_properties(${res_abs} PROPERTIES
                                    MACOSX_PACKAGE_LOCATION "Resources/${res_dir}/"
                                    HEADER_FILE_ONLY 1
                                    )
        list(APPEND asset_files ${res_abs})
    endforeach()
endforeach()

if(ANDROID)
    add_library(${LIB_NAME} SHARED ${PROJ_SOURCES})
elseif(APPLE)
    add_executable(${LIB_NAME} ${PROJ_SOURCES} ${asset_files})
    source_group(TREE ${GAME_RES_ROOT} PREFIX "Resources" FILES ${asset_files})

else() 
    add_executable(${LIB_NAME} ${PROJ_SOURCES})
endif()

target_link_libraries(${LIB_NAME} cocos2d)
target_include_directories(${LIB_NAME} PRIVATE 
    Classes
)


source_group(TREE ${CMAKE_CURRENT_LIST_DIR} PREFIX "Source Files" FILES ${PROJ_SOURCES})

if(MACOSX)
    set_target_properties(${LIB_NAME} PROPERTIES MACOSX_BUNDLE 1)
    set_target_properties(${LIB_NAME} PROPERTIES RESOURCE "${UI_RESOURCES}")
    set_target_properties(${LIB_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/proj.ios_mac/mac/Info.plist")
    if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/proj.ios_mac/mac/Info.plist)
        message(STATUS "found Info.plist")
    endif()
    set_target_properties(${LIB_NAME} PROPERTIES
        MACOSX_DEPLOYMENT_TARGET 10.10
    )
    get_property(gles2_loc TARGET GLESv2 PROPERTY IMPORTED_LOCATION)
    get_property(egl_loc TARGET egl PROPERTY IMPORTED_LOCATION)
    message(STATUS "GLES2 location ${gles2_loc} ${egl_loc}")
    add_custom_command(TARGET ${LIB_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${LIB_NAME}>/../Frameworks
        COMMAND ${CMAKE_COMMAND} -E copy ${gles2_loc} $<TARGET_FILE_DIR:${LIB_NAME}>/../Frameworks
        COMMAND ${CMAKE_COMMAND} -E copy ${egl_loc} $<TARGET_FILE_DIR:${LIB_NAME}>/../Frameworks
    )
endif()

if(WINDOWS)
    add_custom_command(TARGET ${LIB_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ../../../jsb-adapter $<TARGET_FILE_DIR:${LIB_NAME}>/jsb-adapter
        COMMAND ${CMAKE_COMMAND} -E copy_directory ../../../res $<TARGET_FILE_DIR:${LIB_NAME}>/res
        COMMAND ${CMAKE_COMMAND} -E copy_directory ../../../src $<TARGET_FILE_DIR:${LIB_NAME}>/src
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ../../../main.js $<TARGET_FILE_DIR:${LIB_NAME}>/main.js
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ../../../project.json $<TARGET_FILE_DIR:${LIB_NAME}>/project.json
    )
    target_include_directories(${LIB_NAME} PRIVATE 
        proj.win32
    )
endif()

if(MSVC) 
    foreach(item ${WIN32_DLLS})
        get_filename_component(filename ${item} NAME)
        get_filename_component(abs ${item} ABSOLUTE)
        add_custom_command(TARGET ${LIB_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${abs} $<TARGET_FILE_DIR:${LIB_NAME}>/${filename}
        )
    endforeach()
    target_compile_options(${LIB_NAME} PUBLIC /MP)
    target_link_options(${LIB_NAME} PRIVATE /SUBSYSTEM:WINDOWS)
endif()