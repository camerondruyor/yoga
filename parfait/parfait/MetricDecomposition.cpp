#include "MetricDecomposition.h"
#include <string.h>
#include <array>
#include <cmath>
#include "StringTools.h"
#include <limits>
Parfait::MetricDecomposition::Decomposition Parfait::MetricDecomposition::decompose(
    const Parfait::DenseMatrix<double, 3, 3>& M) {
    Decomposition decomp;
    decomp.D = M;
    forceSymmetry(decomp.D);
    decomp.R = eliminate02WithHouseholderReflection(decomp.D);
    bool succeeded = iterativelyDiagonalize(decomp.R, decomp.D);
    if (not succeeded) throwFailedToDiagonalize(M);
    return decomp;
}
Parfait::DenseMatrix<double, 3, 3> Parfait::MetricDecomposition::recompose(
    const Parfait::MetricDecomposition::Decomposition& d) {
    return d.R * d.D * d.R.transpose();
}
double Parfait::MetricDecomposition::calcTargetError(const Parfait::DenseMatrix<double, 3, 3>& M) {
    return M.norm() * std::numeric_limits<double>::epsilon();
}
double Parfait::MetricDecomposition::frobeniusOfOffDiagonals(const Parfait::DenseMatrix<double, 3, 3>& M) {
    double e = 0.0;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (i != j) e += M(i, j) * M(i, j);
    return e;
}
void Parfait::MetricDecomposition::printMatrixAsUnitTestFixture(const Parfait::DenseMatrix<double, 3, 3>& M) {
    printf("%s\n", formatMatrixAsUnitTestFixture(M).c_str());
}
std::string Parfait::MetricDecomposition::formatMatrixAsUnitTestFixture(const Parfait::DenseMatrix<double, 3, 3>& M) {
    char s[1024];
    sprintf(s, "DenseMatrix<double,3,3> M {\n");
    sprintf(s + strlen(s), "\t{%.15e,%.15e,%.15e},\n", M(0, 0), M(0, 1), M(0, 2));
    sprintf(s + strlen(s), "\t{%.15e,%.15e,%.15e},\n", M(1, 0), M(1, 1), M(1, 2));
    sprintf(s + strlen(s), "\t{%.15e,%.15e,%.15e}", M(2, 0), M(2, 1), M(2, 2));
    sprintf(s + strlen(s), "};");
    std::string fixture(s);
    return Parfait::StringTools::rstrip(fixture) + "\n\n";
}
void Parfait::MetricDecomposition::throwFailedToDiagonalize(const Parfait::DenseMatrix<double, 3, 3>& M) {
    printMatrixAsUnitTestFixture(M);
    throw std::logic_error("Failed to diagonalize matrix.");
}

Parfait::MetricDecomposition::Decomposition Parfait::MetricDecomposition::decomposeRobust(
    const Parfait::DenseMatrix<double, 3, 3>& M_in) {
    using Mat = DenseMatrix<double, 3, 3>;

    // Enforce symmetry
    Mat M = M_in;
    M(1, 0) = M(0, 1);
    M(2, 0) = M(0, 2);
    M(2, 1) = M(1, 2);

    // -----------------------------------------------------------------------
    // Step 1: Eigenvalues via the closed-form trigonometric method.
    // For any real symmetric matrix all three eigenvalues are real.
    // Reference: Wikipedia "Eigenvalue algorithm" § Symmetric 3×3 matrices.
    // -----------------------------------------------------------------------
    double eig[3];

    double p1 = M(0, 1) * M(0, 1) + M(0, 2) * M(0, 2) + M(1, 2) * M(1, 2);
    double diag_sq = M(0, 0) * M(0, 0) + M(1, 1) * M(1, 1) + M(2, 2) * M(2, 2);

    if (p1 <= 1e-30 * (diag_sq + 1e-300)) {
        // Already diagonal
        eig[0] = M(0, 0);
        eig[1] = M(1, 1);
        eig[2] = M(2, 2);
    } else {
        double q  = (M(0, 0) + M(1, 1) + M(2, 2)) / 3.0;
        double p2 = (M(0, 0) - q) * (M(0, 0) - q) + (M(1, 1) - q) * (M(1, 1) - q) +
                    (M(2, 2) - q) * (M(2, 2) - q) + 2.0 * p1;
        double p = std::sqrt(p2 / 6.0);

        // B = (1/p)(A - q*I)
        Mat B;
        double inv_p = (p > 0.0) ? (1.0 / p) : 0.0;
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                B(i, j) = inv_p * (M(i, j) - (i == j ? q : 0.0));

        // r = det(B)/2,  clamped to [-1, 1] for numerical safety
        double r = 0.5 * (B(0, 0) * (B(1, 1) * B(2, 2) - B(1, 2) * B(2, 1)) -
                          B(0, 1) * (B(1, 0) * B(2, 2) - B(1, 2) * B(2, 0)) +
                          B(0, 2) * (B(1, 0) * B(2, 1) - B(1, 1) * B(2, 0)));
        r = std::max(-1.0, std::min(1.0, r));

        double phi       = std::acos(r) / 3.0;
        const double tpi = 2.0 * M_PI / 3.0;

        eig[0] = q + 2.0 * p * std::cos(phi);
        eig[2] = q + 2.0 * p * std::cos(phi + tpi);
        eig[1] = 3.0 * q - eig[0] - eig[2];  // stable from trace identity
    }

    // -----------------------------------------------------------------------
    // Step 2: Eigenvectors via cross-product of rows of (A - λI).
    // The null space of (A - λI) contains the eigenvector; the cross product
    // of any two linearly independent rows in the null space gives that vector.
    // -----------------------------------------------------------------------
    using V3 = std::array<double, 3>;

    auto cross3 = [](const V3& a, const V3& b) -> V3 {
        return {a[1] * b[2] - a[2] * b[1],
                a[2] * b[0] - a[0] * b[2],
                a[0] * b[1] - a[1] * b[0]};
    };
    auto norm3 = [](const V3& v) {
        return std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    };

    Mat R = Mat::Identity();

    for (int i = 0; i < 3; ++i) {
        Mat A = M;
        A(0, 0) -= eig[i];
        A(1, 1) -= eig[i];
        A(2, 2) -= eig[i];

        V3 rows[3] = {{{A(0, 0), A(0, 1), A(0, 2)}},
                      {{A(1, 0), A(1, 1), A(1, 2)}},
                      {{A(2, 0), A(2, 1), A(2, 2)}}};

        V3   best{0, 0, 0};
        double best_n = 0.0;
        for (int r0 = 0; r0 < 3; ++r0)
            for (int r1 = r0 + 1; r1 < 3; ++r1) {
                V3     v = cross3(rows[r0], rows[r1]);
                double n = norm3(v);
                if (n > best_n) { best = v; best_n = n; }
            }

        if (best_n > 1e-30) {
            double inv = 1.0 / best_n;
            R(0, i) = best[0] * inv;
            R(1, i) = best[1] * inv;
            R(2, i) = best[2] * inv;
        }
        // else: leave as identity column (degenerate case handled by Gram-Schmidt)
    }

    // Gram-Schmidt orthonormalization (needed when eigenvalues repeat).
    // Column 1: orthogonalize against column 0.
    {
        double dot = R(0, 0) * R(0, 1) + R(1, 0) * R(1, 1) + R(2, 0) * R(2, 1);
        R(0, 1) -= dot * R(0, 0);
        R(1, 1) -= dot * R(1, 0);
        R(2, 1) -= dot * R(2, 0);
        double n = std::sqrt(R(0, 1) * R(0, 1) + R(1, 1) * R(1, 1) + R(2, 1) * R(2, 1));
        if (n > 1e-30) {
            R(0, 1) /= n; R(1, 1) /= n; R(2, 1) /= n;
        } else {
            // Generate any vector orthogonal to col 0.
            V3 c0{R(0, 0), R(1, 0), R(2, 0)};
            for (auto& e : std::array<V3, 3>{{{{1, 0, 0}}, {{0, 1, 0}}, {{0, 0, 1}}}}) {
                V3 v = cross3(c0, e);
                n    = norm3(v);
                if (n > 1e-10) {
                    R(0, 1) = v[0] / n; R(1, 1) = v[1] / n; R(2, 1) = v[2] / n;
                    break;
                }
            }
        }
    }
    // Column 2: cross product of columns 0 and 1.
    {
        V3 c0{R(0, 0), R(1, 0), R(2, 0)};
        V3 c1{R(0, 1), R(1, 1), R(2, 1)};
        V3 v  = cross3(c0, c1);
        double n = norm3(v);
        if (n > 1e-30) { R(0, 2) = v[0] / n; R(1, 2) = v[1] / n; R(2, 2) = v[2] / n; }
    }

    // Build diagonal eigenvalue matrix.
    Mat D{{eig[0], 0, 0}, {0, eig[1], 0}, {0, 0, eig[2]}};

    Decomposition result;
    result.D = D;
    result.R = R;
    return result;
}
bool Parfait::MetricDecomposition::iterativelyDiagonalize(Parfait::DenseMatrix<double, 3, 3>& R,
                                                          Parfait::DenseMatrix<double, 3, 3>& M) {
    double target_error = calcTargetError(M);
    for (int i = 0; i < 100; i++) {
        double error = frobeniusOfOffDiagonals(M);
        if (error <= target_error) break;
        if (shouldReduce12Entry(M)) {
            auto G = reduce12WithGivensRotation(M);
            R = R * G;
        } else {
            auto G = reduce01WithGivensRotation(M);
            R = R * G;
        }
    }
    double error = frobeniusOfOffDiagonals(M);
    return error <= target_error;
}
void Parfait::MetricDecomposition::forceSymmetry(Parfait::DenseMatrix<double, 3, 3>& m) {
    m(1, 0) = m(0, 1);
    m(2, 0) = m(0, 2);
    m(2, 1) = m(1, 2);
}
bool Parfait::MetricDecomposition::shouldReduce12Entry(Parfait::DenseMatrix<double, 3, 3>& B) {
    return std::abs(B(1, 2)) <= std::abs(B(0, 1));
}
Parfait::DenseMatrix<double, 3, 3> Parfait::MetricDecomposition::eliminate02WithHouseholderReflection(
    Parfait::DenseMatrix<double, 3, 3>& M) {
    double a12 = M(1, 2);
    double a02 = M(0, 2);
    double target_error = calcTargetError(M);
    if (std::abs(a02) <= target_error) return DenseMatrix<double, 3, 3>::Identity();
    double denom = std::sqrt(a02 * a02 + a12 * a12);
    double c = a12 / denom;
    double s = -a02 / denom;
    DenseMatrix<double, 3, 3> H{{c, s, 0}, {s, -c, 0}, {0, 0, 1}};
    DenseMatrix<double, 3, 3> result = H.transpose() * M * H;
    M = result;
    return H;
}
Parfait::DenseMatrix<double, 3, 3> Parfait::MetricDecomposition::reduce01WithGivensRotation(
    Parfait::DenseMatrix<double, 3, 3>& B) {
    double b12 = B(1, 2);
    double b11 = B(1, 1);
    double b22 = B(2, 2);
    double denom = sqrt((b22 - b11) * (b22 - b11) + 4.0 * b12 * b12);
    double sigma = (b22 - b11) < 0 ? -1 : 1;
    double cos2theta = -std::abs(b22 - b11) / denom;
    double sin2theta = -2 * sigma * b12 / denom;
    double sin_theta = sqrt(.5 * (1 - cos2theta));
    double cos_theta = .5 * sin2theta / sin_theta;
    double c = cos_theta;
    double s = sin_theta;
    DenseMatrix<double, 3, 3> G{{0, 1, 0}, {c, 0, s}, {-s, 0, c}};
    DenseMatrix<double, 3, 3> result = G.transpose() * B * G;
    B = result;
    return G;
}
Parfait::DenseMatrix<double, 3, 3> Parfait::MetricDecomposition::reduce12WithGivensRotation(
    Parfait::DenseMatrix<double, 3, 3>& B) {
    double b00 = B(0, 0);
    double b11 = B(1, 1);
    double b01 = B(0, 1);
    double denom = sqrt((b00 - b11) * (b00 - b11) + 4.0 * b01 * b01);
    double sigma = (b00 - b11) < 0 ? -1 : 1;
    double cos2theta = -std::abs(b00 - b11) / denom;
    double sin2theta = -2 * sigma * b01 / denom;
    double sin_theta = sqrt(.5 * (1 - cos2theta));
    double cos_theta = .5 * sin2theta / sin_theta;
    double c = cos_theta;
    double s = sin_theta;
    DenseMatrix<double, 3, 3> G{{c, 0, -s}, {s, 0, c}, {0, 1, 0}};
    DenseMatrix<double, 3, 3> result = G.transpose() * B * G;
    B = result;
    return G;
}
