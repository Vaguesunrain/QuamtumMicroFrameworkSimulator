## Qubit model

```mermaid

classDiagram
    class Qubits {
        -int m_num_qubits
        -vector~shared_ptr~QubitModule~~ m_modules
        +install_module(mod)
        +apply_gate(name, target)
        +reset()
        +print_status()
    }

    class QubitModule {
        <<Interface>>
        +on_init(num)
        +on_gate(name, target)
        +reset()
        +on_print()
    }

    class BlochSphereModule {
        -vector~Vector3~ m_vectors
        +on_init(num)
        +on_gate(name, target)
        +reset()
        +on_print()
    }

    class StateVectorModule {
        -vector~complex~ m_amplitudes
        +on_init(num)
        +on_gate(name, target)
        +reset()
        +on_print()
    }

    %% 关系描述
    Qubits o-- QubitModule : 聚合 (拥有列表)
    BlochSphereModule --|> QubitModule : 继承/实现
    StateVectorModule --|> QubitModule : 继承/实现

    %% 注释
    note for Qubits "总线 / 传声筒\n负责分发指令"
    note for BlochSphereModule "插件 A\n处理 Bloch 旋转"
    note for StateVectorModule "插件 B\n处理全态演化"
```


```mermaid
graph TD
    %% 驱动层：Testbench & Input
    subgraph Testbench_Layer [Driver Layer]
        TB[Quantum Testbench] --> |Timed Instructions| CIR[Circuit Buffer / QASM]
        CAL[IBM Calibration Data] --> |JSON/API| PCM
    end

    %% 调度层：逻辑大脑
    subgraph Scheduling_Layer [Scheduling & Control Layer]
        CIR --> DES[Discrete Event Scheduler]
        DES --> |1. Calc Idle DeltaT| DES
        DES --> |2. Trigger Gate Event| MM
        DES --> |3. Sync Global Time| PCM
    end

    %% 物理限制与噪声层
    subgraph Physics_Layer [Physical Constraint Layer]
        PCM[Physical Constraint Manager]
        PCM --> |T1 / T2 Decay| IDLE[Idle Noise Module]
        PCM --> |Coupling Map| CTS[Crosstalk Engine]
        PCM --> |Residual ZZ| STC[Static Interaction Module]
        
        IDLE -.-> |Apply Factors| MM
        CTS -.-> |Apply Factors| MM
    end

    %% HPC 核心与内存调度层
    subgraph HPC_Core_Layer [HPC & Memory Management Layer]
        MM[Memory/NUMA Manager] --> |Tiling / Blocking| CACHE[L3 Cache Optimization]
        MM --> |Thread Affinity| OMP[OpenMP 128-Core Worker]
        
        OMP --> |SIMD / AVX-512| KERNEL[Density Matrix Kernel]
        KERNEL <--> |1TB Shared Memory| RAM[Main RAM / NUMA Nodes]
    end

    %% 监控与验证
    subgraph Verification_Layer [Verification & Monitoring]
        KERNEL --> |State Trace| MON[Numerical Stability Monitor]
        MON --> |Assertion| TB
        KERNEL --> |Fidelity Calc| VAL[Result Validator]
    end

    %% 样式定义
    style RAM fill:#f96,stroke:#333,stroke-width:2px
    style DES fill:#bbf,stroke:#333,stroke-width:2px
    style MM fill:#bfb,stroke:#333,stroke-width:2px
    style TB fill:#f9f,stroke:#333,stroke-width:2px
```


## 时钟树

1.建议分区域，qubit附近采用差分低频信号
2.注意在差分线路加入延时
3.使用时钟分发模块
4.避免glitch
5.差分信号做低摆幅评估


## 实验
1.慢速有时间差的时钟向qubit和gates发送流水线指令
vs
2.指令堆集在qubit附近，传入低平时钟，然后在PLL产高速时钟爆发执行

## 密度矩阵
1.多线程
2.串扰等操作不要操作整个矩阵