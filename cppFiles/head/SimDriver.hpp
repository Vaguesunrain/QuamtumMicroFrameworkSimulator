#ifndef SIM_DRIVER_HPP
#define SIM_DRIVER_HPP

#include <iostream>
#include "Qubits.hpp"
#include "QubitModule/BlochSphere.hpp" 
#include "QubitModule/DensityMatrix.hpp"
// 前向声明 Verilator 的模型类，避免在头文件中包含巨大 generated 头文件
class Vmodule_top; 

class SimDriver {
private:
    Vmodule_top* dut;  
    Qubits* qubits;   
    int m_last_rst_n;

public:
    
    SimDriver(Vmodule_top* top_ptr , int num_qubits=3 , short select_module=1);
    ~SimDriver();
    void step(uint64_t time);

private:
    void init_qubits(int num_qubits);
    void rst_n();
};

#endif