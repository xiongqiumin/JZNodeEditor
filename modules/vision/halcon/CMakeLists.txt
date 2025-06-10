
# 检查HALCON头文件是否存在
find_path(HALCON_INCLUDE_DIR NvInfer.h
    PATHS "E:/libs/Halcon/include"
    NO_DEFAULT_PATH
)

if(NOT HALCON_INCLUDE_DIR)
    message(WARNING "未找到Halcon头文件，项目将不会被编译")
    return()
endif()

#halcon
include_directories(E:/libs/Halcon/include)
	
# 定义导出宏
add_definitions(-DJZHALCONDLL_EXPORTS)

# 源文件
set(JZHALCON_SOURCE_FILES
    JZHalconWrapper.cpp
    JZHalconWrapper.h
)

# 创建DLL
add_library(JZHalcon SHARED ${JZHALCON_SOURCE_FILES})

# 链接库
target_link_libraries(JZHalcon
    Qt${QT_VERSION_MAJOR}::Core
    ${OpenCV_LIBRARIES}    
)