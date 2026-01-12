#include "Qubits.hpp"


Qubits::Qubits(int num) : m_num_qubits(num) {
    m_states.resize(num, 0);
    std::cout << "[Qubits] Initialized system with " << num << " qubits." << std::endl;
}


Qubits::~Qubits() {
    std::cout << "[Qubits] Destroying system." << std::endl;
}


void Qubits::reset() {
    std::fill(m_states.begin(), m_states.end(), 0);
    std::cout << "[Qubits] All qubits reset to ground state |0>." << std::endl;
}


void Qubits::set_state(int index, int state) {
    if (index >= 0 && index < m_num_qubits) {
        m_states[index] = state;
    } else {
        std::cerr << "[Error] Qubit index " << index << " out of range!" << std::endl;
    }
}


int Qubits::get_state(int index) const {
    if (index >= 0 && index < m_num_qubits) {
        return m_states[index];
    }
    return -1; // Error code
}


void Qubits::print_info() const {
    std::cout << "--- Qubit System Status ---" << std::endl;
    for (int i = 0; i < m_num_qubits; ++i) {
        std::cout << "Q[" << i << "]: " << m_states[i] << std::endl;
    }
    std::cout << "---------------------------" << std::endl;
}