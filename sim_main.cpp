#include "Vmodule_top.h"
#include "verilated.h"
#include "verilated_vcd_c.h" 
#include "SimDriver.hpp" 
int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    Vmodule_top* top = new Vmodule_top;

    // --- open wave ---
    Verilated::traceEverOn(true);
    VerilatedVcdC* tfp = new VerilatedVcdC;

    SimDriver* driver = new SimDriver(top);
    top->trace(tfp, 99);
    tfp->open("wave.vcd"); 

    int main_time = 0;
    while (main_time < 40) { // 稍微跑长一点
        top->clk = main_time % 2; 

        if (main_time < 4) top->rst_n = 0; else top->rst_n = 1;

        if (main_time == 10) { 
       
        } else if (main_time > 10) {
        }

        top->eval();
        tfp->dump(main_time); // 将当前时刻的数据存入波形

        // if (top->clk && main_time > 4) {
        //     printf("Time: %d | Active: %d | Out: 0x%x\n", main_time, top->pulse_active, top->out_val);
        // }
        main_time++;
    }
    tfp->close();



    delete driver;
    delete top;
 

    return 0;
}
