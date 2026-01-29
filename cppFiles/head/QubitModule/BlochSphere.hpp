#ifndef BLOCH_SPHERE_HPP
#define BLOCH_SPHERE_HPP

#include "Qubits.hpp"
#include <vector>
#include <cmath>
#include <iomanip>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include "GateLibrary.hpp" 

using Vector3d = Eigen::Vector3d;

class BlochSphereModule : public QubitModule {
private:
    std::vector<Vector3d> m_vectors;
    const GateLibrary& m_gate_lib;

public:
    BlochSphereModule(const GateLibrary& lib) : m_gate_lib(lib) {}

    void on_init(int num) override {
        m_vectors.assign(num, Vector3d(0.0, 0.0, 1.0));
        std::cout << "  -> [Bloch] Initialized storage for " << num << " vectors.\n";
    }

    void on_gate(const std::string& gate_name, int target) override {
        try {
            const Gate& gate = m_gate_lib.get(gate_name);
            if (gate.num_qubits != 1) return; 

            if (target >= m_vectors.size()) return;
            Vector3d& v = m_vectors[target];
            apply_rotation_from_matrix(v, gate.matrix);

        } catch (const std::runtime_error& e) {
            std::cerr << "[Bloch Error] " << e.what() << std::endl;
        }
    }
    
    void on_print() override {
        std::cout << "--- Bloch Sphere Status ---\n";
        for (size_t i = 0; i < m_vectors.size(); ++i) {
            const auto& v = m_vectors[i];
            // 修正 v.x -> v.x()
            std::cout << "Q" << i << ": [" 
                      << std::fixed << std::setprecision(4)
                      << v.x() << ", " << v.y() << ", " << v.z() << "]\n";
        }
    }

    void reset() {
        for (auto& v : m_vectors) {
            v = Vector3d(0.0, 0.0, 1.0);
        }
        std::cout << "  -> [Bloch] All vectors reset to |0> state.\n";
    }

private:
    void apply_rotation_from_matrix(Vector3d& v, const MatrixXc& U_raw) {
        //  det(U) = 1
        std::complex<double> det = U_raw.determinant();
        MatrixXc U = U_raw / std::sqrt(det);

        std::complex<double> trace = U.trace();
        double cos_half_theta = trace.real() / 2.0;
        cos_half_theta = std::max(-1.0, std::min(1.0, cos_half_theta));
        double theta = 2.0 * std::acos(cos_half_theta);

  
        if (std::abs(theta) < 1e-9) return;

   
        //  Pauli  U = cos(t/2)I - i*sin(t/2)(nxX + nyY + nzZ)
        double sin_half_theta = std::sin(theta / 2.0);
        
        // if theta close to 0 ,sin_half_theta close to 0, avoid div by zero
        if (std::abs(sin_half_theta) < 1e-9) return;

        double nx = -U(0, 1).imag() / sin_half_theta; 
        double ny = -U(0, 1).real() / sin_half_theta;
        double nz = (U(1, 1).imag() - U(0, 0).imag()) / (2.0 * sin_half_theta);
        
        Vector3d axis(nx, ny, nz);
        if (axis.norm() > 1e-9) {
           axis.normalize();
        }

        Eigen::AngleAxisd rotation(theta, axis);
        v = rotation * v;
    }
};
#endif