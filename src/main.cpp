#include <iostream>
#include <vector>
#include <cmath>
#include <numbers>
#include <chrono>
#include <Eigen/Dense>
#include <fftw3.h>
#include <omp.h>

// 1. Eigen 线性代数求解示例
void run_linear_algebra_demo() {
    std::cout << "=== 1. Eigen 线性方程组求解 (Ax = b) ===\n";
    constexpr int N = 1000;

    // 构造对角占优矩阵 A 与右端项 b，确保非奇异
    Eigen::MatrixXd A = Eigen::MatrixXd::Random(N, N);
    A.diagonal().array() += static_cast<double>(N);
    Eigen::VectorXd b = Eigen::VectorXd::Random(N);

    auto t0 = std::chrono::high_resolution_clock::now();
    // 使用部分主元 LU 分解求解
    Eigen::VectorXd x = A.partialPivLu().solve(b);
    auto t1 = std::chrono::high_resolution_clock::now();

    double rel_err = (A * x - b).norm() / b.norm();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::cout << "矩阵规模: " << N << "x" << N << "\n"
              << "求解耗时: " << ms << " ms\n"
              << "相对残差 ||Ax - b|| / ||b||: " << rel_err << "\n\n";
}

// 2. OpenMP + FFTW3 信号生成与频谱分析示例
void run_fft_demo() {
    std::cout << "=== 2. OpenMP + FFTW3 快速傅里叶变换 ===\n";
    constexpr int N = 4096;
    constexpr double fs = 1000.0; // 采样率 1000 Hz
    constexpr double f1 = 50.0;   // 信号主频 50 Hz
    constexpr double f2 = 120.0;  // 次频 120 Hz

    // 使用 fftw_alloc 保证 32/64 字节内存对齐，以发挥 AVX2 SIMD 性能
    double* in = fftw_alloc_real(N);
    fftw_complex* out = fftw_alloc_complex(N / 2 + 1);

    // 使用 OpenMP 并行生成离散采样信号
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; ++i) {
        double t = static_cast<double>(i) / fs;
        in[i] = 2.5 * std::sin(2.0 * std::numbers::pi * f1 * t)
              + 1.0 * std::cos(2.0 * std::numbers::pi * f2 * t);
    }

    // 创建并执行一维实数到复数的 FFT 计划
    fftw_plan plan = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);
    fftw_execute(plan);

    // 计算幅值谱并找出峰值频率
    double max_amp = 0.0;
    double peak_freq = 0.0;
    for (int k = 0; k <= N / 2; ++k) {
        double re = out[k][0];
        double im = out[k][1];
        double amp = (2.0 / N) * std::hypot(re, im);
        if (amp > max_amp) {
            max_amp = amp;
            peak_freq = static_cast<double>(k) * fs / N;
        }
    }

    std::cout << "最大可用 OpenMP 线程数: " << omp_get_max_threads() << "\n"
              << "FFT 检测到主频: " << peak_freq << " Hz (幅值约 " << max_amp << ")\n";

    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);
}

int main() {
    // 若开启 OpenMP，可查看 Eigen 当前使用的线程数
    std::cout << "Eigen 默认线程数: " << Eigen::nbThreads() << "\n\n";
    run_linear_algebra_demo();
    run_fft_demo();
    return 0;
}