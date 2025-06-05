
# 检查TensorRT头文件是否存在
find_path(TENSORRT_INCLUDE_DIR NvInfer.h
    PATHS "E:/libs/CUDA/TensorRT-8.2.5.1/include"
    NO_DEFAULT_PATH
)

if(NOT TENSORRT_INCLUDE_DIR)
    message(WARNING "未找到TensorRT头文件，项目将不会被编译")
    return()
endif()

#cuda
include_directories(E:/libs/CUDA/TensorRT-8.2.5.1/include
    E:/libs/CUDA/TensorRT-8.2.5.1/include
    E:/libs/CUDA/v11.4/include)
	

# 定义导出宏
add_definitions(-DTENSORRTENGINEDLL_EXPORTS)

# 源文件
set(TENSORRT_SOURCE_FILES
    JZTensorRT.cpp
    JZTensorRT.h
	JZTensorRTImpl.cpp
	JZTensorRTImpl.h
)

# 创建DLL
add_library(TensorRTEngine SHARED ${TENSORRT_SOURCE_FILES})

# 链接库
target_link_libraries(TensorRTEngine
    ${TensorRT_LIBRARIES}
    ${CUDA_LIBRARIES}
)