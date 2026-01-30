#include "Qubits.hpp"
#include <complex> 
#include <omp.h> 
Qubits::Qubits(int num) : m_num_qubits(num), m_dim(0), m_global_state(nullptr) {
   
}

Qubits::~Qubits() {
    delete[] m_global_state;
}

void Qubits:: allocate_global_state() {
    // avert double allocation
    if (m_global_state) return;

    m_dim = static_cast<size_t>(1) << m_num_qubits;
    size_t total_elements = m_dim * m_dim;
    
    std::cout << "[Qubits] A module requested global state. Allocating " 
                << (total_elements * sizeof(std::complex<double>) / (1024.0*1024.0*1024.0)) 
                << " GB..." << std::endl;

    m_global_state = new std::complex<double>[total_elements];

    // NUMA First-Touch initialization
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < total_elements; ++i) {
        m_global_state[i] = std::complex<double>(0, 0);
    }
    m_global_state[0] = std::complex<double>(1.0, 0.0);
}

void Qubits::install_module(std::shared_ptr<QubitModule> mod) {
        mod->on_init(m_num_qubits);
        if (mod->requests_global_state()) {
            allocate_global_state();
            mod->attach_data(m_global_state);
        }
        m_modules.push_back(mod);
    }


void Qubits::apply_gate(std::string name, int target) {
    std::cout << "[System] Applying " << name << " on Q" << target << std::endl;
    for (auto& mod : m_modules) {
        mod->on_gate(name, target);
    }
}
void Qubits::apply_multi_gate(std::string name, const std::vector<int>& targets) {
    for (auto& mod : m_modules) mod->on_multi_gate(name, targets);
}
void Qubits::print_status() {
    for (auto& mod : m_modules) {
        mod->on_print();
    }
}

void Qubits::reset() {
    std::cout << "[System] Resetting all modules..." << std::endl;
    for (auto& mod : m_modules) {
        mod->reset(); 
    }
}

void Qubits::print_full_matrix() {
    for (auto& mod : m_modules) {
        mod->try_print_full_matrix();
    }
}