#ifndef GATE_HPP
#define GATE_HPP

#include <string>
#include <Eigen/Dense>

using MatrixXc = Eigen::MatrixXcd;

class Gate {
public:
    std::string name;
    int num_qubits;       // applicable qubits
    double duration_ns;   // duration in nanoseconds
    bool is_controlled = false;
    MatrixXc matrix;

public:
    Gate(std::string n, int nq, double dur, bool controlled, const MatrixXc& mat)
        : name(n), num_qubits(nq), duration_ns(dur), is_controlled(controlled), matrix(mat) {}

    Gate() : num_qubits(0), duration_ns(0) {}
};

#endif