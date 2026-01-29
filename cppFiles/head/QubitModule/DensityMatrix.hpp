#ifndef DENSITY_MATRIX_MODULE_HPP
#define DENSITY_MATRIX_MODULE_HPP

#include "Qubits.hpp"
#include <complex>
#include <vector>
#include <iostream>
#include <omp.h> // 必须引入 OpenMP 压榨 128 核性能

#define INSERT_ZERO_BIT(val, mask) (((val) & ~((mask) - 1)) << 1 | ((val) & ((mask) - 1)))

class DensityMatrixModule : public QubitModule {
private:
    std::complex<double>* m_rho = nullptr; // 指向 1TB 连续空间的指针
    int m_num_qubits = 0;
    size_t m_dim = 0; // 2^N
    const GateLibrary& m_gate_lib;

public:
    DensityMatrixModule(const GateLibrary& lib) : m_gate_lib(lib) {}
    bool requests_global_state() const override { return true; }//state that module needs full access to big ram
    void on_init(int num) override {
        m_num_qubits = num;
        m_dim = static_cast<size_t>(1) << num;
        // 注意：内存申请由外部 Qubits 类完成，这里只做逻辑确认
        std::cout << "  -> [DensityMatrix] Ready to manage " << (m_dim * m_dim * sizeof(std::complex<double>)) / (1024*1024*1024.0) 
                  << " GB state.\n";
    }

    // 接收来自 Qubits 类的 1TB 原始指针
    void attach_data(std::complex<double>* raw_ptr) override {
        m_rho = raw_ptr;
    }

    void on_gate(const std::string& gate_name, int target) override {
        if (!m_rho) return;

        const Gate& gate = m_gate_lib.get(gate_name);
        
        if (gate.num_qubits == 1) {
            apply_single_qubit_gate(target, gate.matrix);
        } else {
            // 这里以后扩展 CNOT 等双比特门
        }
    }

    void on_print() override {
        // 对于 18-Qubit，打印完整矩阵是不可能的，这里只打印迹 Trace
        std::cout << "--- Density Matrix Status ---\n";
        std::complex<double> trace(0, 0);
        for (size_t i = 0; i < m_dim; ++i) {
            trace += m_rho[i * m_dim + i]; // 累加对角线
        }
        std::cout << "  -> Dim: " << m_dim << "x" << m_dim << "\n";
        std::cout << "  -> Trace: " << trace.real() << " + " << trace.imag() << "j (Should be 1.0)\n";
    }

    void try_print_full_matrix() override {
        if (m_num_qubits > 6) {
            std::cout << "--- We do not suggest printing full matrix for >6 qubits ---\n";
            std::cout << "If you really want to, please modify the code in DensityMatrixModule.hpp\n";
            std::cout << "[DensityMatrix] Full matrix print skipped for >6 qubits.\n";
            return;
        }
        std::cout << "--- Full Density Matrix ---\n";
        for (size_t r = 0; r < m_dim; ++r) {
            for (size_t c = 0; c < m_dim; ++c) {
                std::complex<double> val = m_rho[r * m_dim + c];
                std::cout << "(" << val.real() << "," << val.imag() << ") ";
            }
            std::cout << "\n";
        }
    }
private:
   
    // Math: rho_new = U * rho_sub * U_dag
    void apply_single_qubit_gate(int target, const MatrixXc& U) {
        size_t mask = static_cast<size_t>(1) << target;
        
        // 预先加载门矩阵元素到寄存器，避免重复访问
        const std::complex<double> u00 = U(0, 0);
        const std::complex<double> u01 = U(0, 1);
        const std::complex<double> u10 = U(1, 0);
        const std::complex<double> u11 = U(1, 1);

        // 预计算共轭（用于右乘）
        const std::complex<double> u00_c = std::conj(u00);
        const std::complex<double> u01_c = std::conj(u01);
        const std::complex<double> u10_c = std::conj(u10);
        const std::complex<double> u11_c = std::conj(u11);

        // 我们遍历所有的 block，每个 block 包含 4 个元素：
        // (r0, c0), (r0, c1), (r1, c0), (r1, c1)
        // 这里的 r 和 c 是排除了 target bit 后的“压缩索引”
        // 外层循环迭代行，内层循环迭代列
        
        // OpenMP: 只需要并行化外层循环（行）。
        // schedule(static) 对于 NUMA 通常是最优的，前提是初始化也是 static 划分。
        #pragma omp parallel for schedule(static)
        for (size_t r = 0; r < m_dim / 2; ++r) {
            size_t row0 = INSERT_ZERO_BIT(r, mask);
            size_t row1 = row0 | mask;

            // 指针优化：提取行首指针，减少乘法运算
            std::complex<double>* ptr_row0 = m_rho + row0 * m_dim;
            std::complex<double>* ptr_row1 = m_rho + row1 * m_dim;

            for (size_t c = 0; c < m_dim / 2; ++c) {
                size_t col0 = INSERT_ZERO_BIT(c, mask);
                size_t col1 = col0 | mask;

                // 读取 4 个关键位置的值
                // 物理内存位置:
                // rho00 @ ptr_row0[col0]
                // rho01 @ ptr_row0[col1]
                // rho10 @ ptr_row1[col0]
                // rho11 @ ptr_row1[col1]

                std::complex<double> rho00 = ptr_row0[col0];
                std::complex<double> rho01 = ptr_row0[col1];
                std::complex<double> rho10 = ptr_row1[col0];
                std::complex<double> rho11 = ptr_row1[col1];

                // 核心计算：tmp = U * rho
                std::complex<double> t00 = u00 * rho00 + u01 * rho10;
                std::complex<double> t01 = u00 * rho01 + u01 * rho11;
                std::complex<double> t10 = u10 * rho00 + u11 * rho10;
                std::complex<double> t11 = u10 * rho01 + u11 * rho11;
        
                // 核心计算：final = tmp * U_dag
                // 注意：右乘 U_dag 相当于列变换
                // Res00 = t00 * u00_c + t01 * u01_c
                // Res01 = t00 * u10_c + t01 * u11_c  <-- 注意矩阵乘法行列对应
                
                ptr_row0[col0] = t00 * u00_c + t01 * u01_c;
                ptr_row0[col1] = t00 * u10_c + t01 * u11_c; // U_dag(0,1) is conj(U(1,0))
                ptr_row1[col0] = t10 * u00_c + t11 * u01_c;
                ptr_row1[col1] = t10 * u10_c + t11 * u11_c;
            }
        }
    }
};
/*
ToDo:
还可以压榨的性能点：
SIMD (AVX-512)：目前的 std::complex 很难自动向量化。如果你极度追求性能，可以将实部和虚部分开存储（Structure of Arrays, SoA），或者使用 reinterpret_cast<double*>(m_rho) 将其视为 double 数组，手写 AVX-512 Intrinsics 来一次处理 4 个复数（8 个 double）。这通常能再带来 2-4 倍的提升。
Cache Blocking (分块)：虽然现在的遍历方式对于行是连续的，但如果 col0 和 col1 跨度很大（即 target qubit 索引很大），会导致 Cache Thrashing。可以引入 Cache Blocking 技术，将内层循环分块处理，保证处理的数据块能放入 L2 Cache。
*/
#endif