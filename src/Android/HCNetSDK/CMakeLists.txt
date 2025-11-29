# if(NOT ANDROID)
#     return()
# endif()
# #模块要用add_qml_module,用target_sources不能出现模块
# qt_add_qml_module(AndroidHCNetSDK
#     URI QGroundControl.HCNetSDK
#     VERSION 1.0
#     QML_FILES
#         # 可以写 .qml 文件（如果有的话）
#     SOURCES
#         AndroidHCNetSDKInterface.cc
#         AndroidHCNetSDKInterface.h
# )

# target_link_libraries(AndroidHCNetSDK PRIVATE Qt6::Core Qt6::Qml)
if(NOT ANDROID)
    return()
endif()

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        AndroidHCNetSDKInterface.cc
        AndroidHCNetSDKInterface.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
