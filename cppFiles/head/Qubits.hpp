#ifndef QUBITS_HPP
#define QUBITS_HPP

#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <complex> 
#include <omp.h> 

class QubitModule {
public:
    virtual ~QubitModule() = default;
    virtual void on_init(int num_qubits) {} 
    virtual bool requests_global_state() const { return false; }//state that module needs full access to big ram
    virtual void attach_data(std::complex<double>* raw_state_ptr) {} 
    virtual void on_gate(const std::string& gate, int target_q) {}
    virtual void on_print() {}
    virtual void try_print_full_matrix() {}
    virtual void reset() {}
};





class Qubits {
private:
    int m_num_qubits;

    size_t m_dim;
    // 【新增】由 Qubits 类持有唯一的 1TB 数据的所有权
    std::complex<double>* m_global_state = nullptr; 
    std::vector<std::shared_ptr<QubitModule>> m_modules;
    void allocate_global_state();

public:
    
    
    Qubits(int num);
    ~Qubits();
    void install_module(std::shared_ptr<QubitModule> mod);
    void apply_gate(std::string name, int target); 
    void print_status();
    void reset();
    void print_full_matrix();
};

#endif