#ifndef DM_KERNELS_HPP
#define DM_KERNELS_HPP

#include <complex>
#include <cstddef> // for size_t
#include "Eigen/Dense" 

namespace DMKernels {
    
    void apply_controlled_gate(std::complex<double>* rho, size_t dim, 
                               int ctrl, int target, const Eigen::Matrix2cd& V);

    void apply_general_2q_gate(std::complex<double>* rho, size_t dim, 
                               int q1, int q2, const Eigen::Matrix4cd& U);

    void apply_swap(std::complex<double>* rho, size_t dim, 
                    int q1, int q2);

    void apply_single_qubit_gate(std::complex<double>* rho, size_t dim, 
                                 int target, const Eigen::Matrix2cd& U);
}

#endif