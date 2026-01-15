#include "SimDriver.hpp"
#include "Vmodule_top.h" 
#include "GateLibrary.hpp"
GateLibrary gate_lib;
SimDriver::SimDriver(Vmodule_top* top_ptr, int num_qubits) : dut(top_ptr) {
    init_qubits(num_qubits);
    auto bloch_module = std::make_shared<BlochSphereModule>(gate_lib);
    qubits->install_module(bloch_module);
}

SimDriver::~SimDriver() {
    delete qubits;
}

void SimDriver::step(uint64_t time) {
   rst_n();
   if(dut->trigger) {
       std::cout << "[SimDriver] Time " << time << ": Trigger received. Performing operations..." << std::endl;
       // Example: Apply a Hadamard gate to qubit 0 on trigger
       qubits->apply_gate("H", {0});
       qubits->print_status();
   }
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