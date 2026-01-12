#include "Vpulse_ctrl.h"
#include "verilated.h"
#include "verilated_vcd_c.h" // 必须包含这个头文件

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    Vpulse_ctrl* top = new Vpulse_ctrl;

    // --- 波形追踪设置 ---
    Verilated::traceEverOn(true);
    VerilatedVcdC* tfp = new VerilatedVcdC;
    top->trace(tfp, 99); // 追踪 99 层层级
    tfp->open("wave.vcd"); // 保存的文件名

    int main_time = 0;
    while (main_time < 40) { // 稍微跑长一点
        top->clk = main_time % 2; 

        // 修正逻辑：先给复位，再撤销
        if (main_time < 4) top->rst_n = 0; else top->rst_n = 1;

        // 在复位撤销后给 trigger
        if (main_time == 10) { 
            top->trigger = 1; 
            top->amplitude = 0xAA;
        } else if (main_time > 10) {
            top->trigger = 0;
        }

        top->eval();
        tfp->dump(main_time); // 将当前时刻的数据存入波形

        if (top->clk && main_time > 4) {
            printf("Time: %d | Active: %d | Out: 0x%x\n", main_time, top->pulse_active, top->out_val);
        }
        main_time++;
    }
    tfp->close();
    delete top;
    return 0;
}
