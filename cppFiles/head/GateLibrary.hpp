#ifndef GATE_LIBRARY_HPP
#define GATE_LIBRARY_HPP

#include "Gate.hpp"
#include <map>
#include <stdexcept>
#include <complex>

class GateLibrary {
private:
    std::map<std::string, Gate> m_gate_map;

public:
    GateLibrary() {
        using namespace std::complex_literals; // for 1i
        // Pauli-X
        MatrixXc X(2, 2); X << 0, 1, 1, 0;
        m_gate_map.emplace("X", Gate("X", 1, 20.0, X));
        // Hadamard
        MatrixXc H(2, 2); H << 1, 1, 1, -1; H /= sqrt(2.0);
        m_gate_map.emplace("H", Gate("H", 1, 20.0, H));
        // CNOT (4x4 matrix)
        MatrixXc CNOT(4, 4); CNOT << 1,0,0,0, 0,1,0,0, 0,0,0,1, 0,0,1,0;
        m_gate_map.emplace("CNOT", Gate("CNOT", 2, 200.0, CNOT));
        // other gates.....
    }

    //make your own gate library
    void register_gate(const Gate& gate) {
        m_gate_map[gate.name] = gate;
    }
 
    const Gate& get(const std::string& name) const {
        auto it = m_gate_map.find(name);
        if (it == m_gate_map.end()) {

            throw std::runtime_error("Gate not found: " + name);
        }
        return it->second;
    }
};

#endif