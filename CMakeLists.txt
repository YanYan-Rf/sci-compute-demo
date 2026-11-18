cmake_minimum_required(VERSION 3.25)
project(SciComputeDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. 查找通过 vcpkg 安装的科学计算库
find_package(Eigen3 CONFIG REQUIRED)
find_package(FFTW3 CONFIG REQUIRED)
find_package(OpenMP)

add_executable(sci_demo src/main.cpp)

# 2. 链接目标库
target_link_libraries(sci_demo PRIVATE
    Eigen3::Eigen
    FFTW3::fftw3
)

if(OpenMP_CXX_FOUND)
    target_link_libraries(sci_demo PRIVATE OpenMP::OpenMP_CXX)
endif()

# 3. 针对 MSVC 的科学计算编译优化选项
if(MSVC)
    target_compile_options(sci_demo PRIVATE
        /utf-8           # 统一源码与执行字符集为 UTF-8
        /W4              # 开启较高警告级别
        # Release 模式下开启 AVX2 指令集向量化、全程序优化与 O2
        $<$<CONFIG:Release>:/O2 /arch:AVX2 /GL>
    )
    target_link_options(sci_demo PRIVATE
        $<$<CONFIG:Release>:/LTCG>  # 链接时代码生成（跨编译单元内联）
    )
endif()