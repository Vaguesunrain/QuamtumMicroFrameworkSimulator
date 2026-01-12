#include "SimDriver.hpp"
#include "Vmodule_top.h" 

SimDriver::SimDriver(Vmodule_top* top_ptr, int num_qubits) : dut(top_ptr) {
    init_qubits(num_qubits);
}

SimDriver::~SimDriver() {
    delete qubits;
}

void SimDriver::step(uint64_t time) {
   rst_n();
}

void SimDriver::init_qubits(int num_qubits) {
    qubits = new Qubits(num_qubits);

    std::cout << "[SimDriver] Qubit system initialized with " << num_qubits << " qubits." << std::endl;
}

void SimDriver::rst_n() {
    int current_rst_n   = dut->rst_n;
    if (m_last_rst_n == 1 && current_rst_n == 0) {
        std::cout << "[SimDriver] Detected Reset Asserted. Resetting Qubits..." << std::endl;
        qubits->reset();
    }
    m_last_rst_n       = current_rst_n;
}