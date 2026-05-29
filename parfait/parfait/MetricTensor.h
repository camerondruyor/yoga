#pragma once
#include <parfait/DenseMatrix.h>
#include <parfait/MetricDecomposition.h>
#include <parfait/Point.h>
#include <cmath>
#include <vector>
#include "MotionMatrix.h"

namespace Parfait {

using Tensor = DenseMatrix<double, 3, 3>;

class MetricTensor {
  public:
    static double edgeLength(const Tensor& M, const Point<double>& edge) {
        Vector<double, 3> e{edge[0], edge[1], edge[2]};
        return std::sqrt(e.dot(M * e));
    }

    static Tensor metricFromEllipse(const Point<double>& axis_1,
                                    const Point<double>& axis_2,
                                    const Point<double>& axis_3) {
        MetricDecomposition::Decomposition decomp;
        decomp.D.clear();
        double h = axis_1.magnitude();
        decomp.D(0, 0) = 1.0 / (h * h);
        h = axis_2.magnitude();
        decomp.D(1, 1) = 1.0 / (h * h);
        h = axis_3.magnitude();
        decomp.D(2, 2) = 1.0 / (h * h);

        auto eigenvector_1 = axis_1;
        eigenvector_1.normalize();
        auto eigenvector_2 = axis_2;
        eigenvector_2.normalize();
        auto eigenvector_3 = axis_3;
        eigenvector_3.normalize();

        for (int i = 0; i < 3; i++) {
            decomp.R(i, 0) = eigenvector_1[i];
            decomp.R(i, 1) = eigenvector_2[i];
            decomp.R(i, 2) = eigenvector_3[i];
        }

        return MetricDecomposition::recompose(decomp);
    }

    static Tensor metricFromEllipse(double h1, double h2, double h3, double alpha, double beta, double gamma) {
        alpha *= M_PI / 180.0;
        beta *= M_PI / 180.0;
        gamma *= M_PI / 180.0;
        h1 = h1 * h1;
        h2 = h2 * h2;
        h3 = h3 * h3;
        double sa = sin(alpha);
        double sb = sin(beta);
        double sg = sin(gamma);
        double ca = cos(alpha);
        double cb = cos(beta);
        double cg = cos(gamma);
        DenseMatrix<double, 3, 3> R{{ca * cb, ca * sb * sg - sa * cg, ca * sb * cg + sa * sg},
                                    {sa * cb, sa * sb * sg + ca * cg, sa * sb * cg - ca * sg},
                                    {-sb, cb * sg, cb * cg}};
        // printf("R:\n");
        // print(R);
        DenseMatrix<double, 3, 3> D{{1.0 / h1, 0, 0}, {0, 1.0 / h2, 0}, {0, 0, 1.0 / h3}};
        // printf("D:\n");
        // print(D);
        return R * D * R.transpose();
    }

    static double logEuclideanDistance(const Tensor& A, const Tensor& B) {
        auto L1 = metricTransform(A, Op::Log);
        auto L2 = metricTransform(B, Op::Log);
        auto diff = L1 - L2;
        auto squared = metricTransform(diff, Op::Square);
        return std::sqrt(squared.trace());
    }

    static Tensor logEuclideanAverage(const std::vector<Tensor>& tensors, const std::vector<double>& weights) {
        Tensor mean;
        mean.clear();
        for (size_t i = 0; i < tensors.size(); i++) {
            try {
                mean = mean + (metricTransform(tensors[i], Op::Log) * weights[i]);
            } catch (...) {
                std::stringstream ss;
                ss << "Mean:\n";
                ss << MetricDecomposition::formatMatrixAsUnitTestFixture(mean);
                ss << "Input tensors:\n";
                // for(auto& tensor:tensors)
                ss << MetricDecomposition::formatMatrixAsUnitTestFixture(tensors[i]);
                ss << "Weights\n";
                for (auto w : weights) ss << w << " ";
                ss << "\n";
                printf("Log euclidean average failed:\n%s\n", ss.str().c_str());
                PARFAIT_THROW("Log euclidean average failed");
            }
        }
        return metricTransform(mean, Op::Exponential);
    }

    static Tensor invert(const Tensor& M) {
        auto d = MetricDecomposition::decomposeRobust(M);
        for (int i = 0; i < 3; i++) d.D(i, i) = 1.0 / d.D(i, i);
        return d.R * d.D * d.R.transpose();
    }

    static Tensor rotate(const Tensor& A, const Parfait::MotionMatrix& rotation) {
        DenseMatrix<double, 4, 4> m;
        rotation.getMatrix(m.data());
        Tensor M = DenseMatrix<double, 4, 4>::SubMatrix(0, 0, 3, 3, m);

        auto Mt = M.transpose();
        Tensor B = A * Mt;
        B = M * B;
        return B;
    }

    static Vector<double, 3> extractEigenvector(const MetricDecomposition::Decomposition& decomp, int n) {
        Vector<double, 3> e{0, 0, 0};
        for (int i = 0; i < 3; i++) e[i] = decomp.R(n, i);
        return e;
    }

    static Tensor intersect(const Tensor& M1, const Tensor& M2) { return intersectAlauzet(M1, M2); }

    static Tensor intersectAlauzet(const Tensor& M1, const Tensor& M2) {
        // Simultaneous diagonalization in M1's eigenbasis.
        //
        // 1. M1 = R D R^T  (decomposeRobust)
        // 2. M2_basis = R^T M2 R  (symmetric — M2 in M1's frame)
        // 3. N(i,j) = M2_basis(i,j) / sqrt(D_ii * D_jj)
        //    = D^{-1/2} M2_basis D^{-1/2} = M1^{-1/2} M2 M1^{-1/2}  (still symmetric)
        // 4. N = Q Lambda Q^T  (decomposeRobust)
        // 5. Intersection eigenvalues: max(1, Lambda_i)  (1 = M1's unit eigenvalue in this frame)
        // 6. M_int = R (D^{1/2} Q) max(I,Lambda) (D^{1/2} Q)^T R^T
        //
        // Note: operator* returns a lazy MatrixMultiply; use Tensor (not auto) to force
        // evaluation so temporaries don't dangle.

        auto         d1 = MetricDecomposition::decomposeRobust(M1);
        const Tensor& R = d1.R;
        const Tensor& D = d1.D;

        // M2 in M1's eigenbasis: R^T M2 R  (2 matmuls)
        Tensor Rt       = R.transpose();  // evaluated copy
        Tensor M2_basis = Rt * M2 * R;

        // N = D^{-1/2} M2_basis D^{-1/2}  (scale by diagonal — no matmul)
        Tensor N = M2_basis;
        for (int i = 0; i < 3; i++) {
            double si = (D(i, i) > 0.0) ? 1.0 / std::sqrt(D(i, i)) : 0.0;
            for (int j = 0; j < 3; j++) {
                double sj = (D(j, j) > 0.0) ? 1.0 / std::sqrt(D(j, j)) : 0.0;
                N(i, j) = M2_basis(i, j) * si * sj;
            }
        }

        auto d2 = MetricDecomposition::decomposeRobust(N);

        // Q_scaled = D^{1/2} Q: scale row i of Q by sqrt(D_ii)  (no matmul)
        Tensor Q_scaled = d2.R;
        for (int i = 0; i < 3; i++) {
            double si = std::sqrt(std::max(D(i, i), 0.0));
            for (int j = 0; j < 3; j++) Q_scaled(i, j) = d2.R(i, j) * si;
        }

        Tensor P    = R * Q_scaled;  // 1 matmul
        Tensor L_int = d2.D;
        for (int i = 0; i < 3; i++) L_int(i, i) = std::max(d2.D(i, i), 1.0);
        Tensor PtT  = P.transpose();
        return P * L_int * PtT;      // 2 matmuls in return expression
    }

  private:
    enum Op { Log, Exponential, Root, Square };
    static Tensor metricTransform(const Tensor& m, Op op) {
        auto decomp = MetricDecomposition::decomposeRobust(m);
        auto& D = decomp.D;
        auto& R = decomp.R;
        for (int i = 0; i < 3; i++) {
            switch (op) {
                case Op::Log:
                    D(i, i) = std::max(D(i, i), 1e-10);
                    D(i, i) = std::log(D(i, i));
                    break;
                case Op::Exponential:
                    D(i, i) = std::exp(D(i, i));
                    break;
                case Op::Root:
                    D(i, i) = std::sqrt(D(i, i));
                    break;
                case Op::Square:
                    D(i, i) *= D(i, i);
                    break;
            }
            if (!std::isfinite(D(i, i))) PARFAIT_THROW("metricTransofrm generated a non-finite number and failed");
        }
        return R * D * R.transpose();
    }
};

}