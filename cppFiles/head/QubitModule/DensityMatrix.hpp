#ifndef DENSITY_MATRIX_MODULE_HPP
#define DENSITY_MATRIX_MODULE_HPP

#include "Qubits.hpp"
#include <complex>
#include <vector>
#include <iostream>
#include "QubitModule/DMKernels.hpp"



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
    }

    // 接收来自 Qubits 类的 1TB 原始指针
    void attach_data(std::complex<double>* raw_ptr) override {
        m_rho = raw_ptr;
    }

    void on_gate(const std::string& gate_name, int target) override {
        if (!m_rho) return;
        const Gate& gate = m_gate_lib.get(gate_name);
        Eigen::Matrix2cd fixed_mat = gate.matrix;
        DMKernels::apply_single_qubit_gate(m_rho, m_dim, target, fixed_mat);
     
    }

    void on_multi_gate(const std::string& gate_name, const std::vector<int>& targets) override {
        if (!m_rho) return;

        const Gate& gate = m_gate_lib.get(gate_name);
        
        if (gate.num_qubits == 2 && targets.size() == 2) {
            if(gate_name == "SWAP") {
                DMKernels::apply_swap(m_rho, m_dim, targets[0], targets[1]);
                return;
            }
            if(gate.is_controlled) {
                std::cout << "[DensityMatrix] Applying controlled gate: " << gate_name << " on Q" 
                          << targets[0] << " (control) and Q" << targets[1] << " (target)." << std::endl;
                Eigen::Matrix2cd V = gate.matrix.block(2, 2, 2, 2);
                DMKernels::apply_controlled_gate(m_rho, m_dim, targets[0], targets[1], V);
            }
            else{
                Eigen::Matrix4cd U = gate.matrix;
                DMKernels::apply_general_2q_gate(m_rho, m_dim, targets[0], targets[1], U);
            }
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
    void reset() override {
        if (!m_rho) return;
        // 重置为 |0><0| 状态
        #pragma omp parallel for schedule(static)
        for (size_t i = 0; i < m_dim * m_dim; ++i) {
            m_rho[i] = std::complex<double>(0, 0);
        }
        m_rho[0] = std::complex<double>(1.0, 0.0);
        std::cout << "  -> [DensityMatrix] Reset to |0><0| state.\n";
    }
};
   
#endif