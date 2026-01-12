#ifndef QUBITS_HPP
#define QUBITS_HPP

#include <vector>
#include <iostream>
#include <string>

class Qubits {
private:
    int m_num_qubits;
    std::vector<int> m_states;

public:
    Qubits(int num);
    ~Qubits();
    void reset();
    void set_state(int index, int state);
    int get_state(int index) const;
    void print_info() const;
};

#endif // QUBITS_HPP