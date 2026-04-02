#include <RingAssertions.h>
#include <parfait/DenseMatrix.h>
#include <parfait/MetricDecomposition.h>
#include <parfait/MetricTensor.h>
using namespace Parfait;

template <typename M>
void print(const M& m) {
    for (int i = 0; i < m.rows(); i++) {
        for (int j = 0; j < m.cols(); j++) printf("%e ", m(i, j));
        printf("\n");
    }
}

DenseMatrix<double, 3, 3> reformMatrix(const DenseMatrix<double, 3, 3>& D) {
    DenseMatrix<double, 3, 3> Dt = D;
    Dt.transpose();
    DenseMatrix<double, 3, 3> E;
    for (int i = 0; i < 3; i++) E(i, i) = D(i, i);
    return D * E * Dt;
}

bool isReformedMatrixEquivalentToOriginal(const MetricDecomposition::Decomposition& decomp,
                                          const Tensor& M,
                                          double tol) {
    auto& D = decomp.D;
    auto& R = decomp.R;
    Tensor M2 = R * D * R.transpose();

    double largest_error = 0.0;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++) largest_error = std::abs(M(i, j) - M2(i, j));
    return largest_error <= tol;
}

bool diagonalEntriesAreFiniteAndPositive(const DenseMatrix<double, 3, 3>& D) {
    for (int i = 0; i < 3; i++)
        if (D(i, i) <= 0.0 or not std::isfinite(D(i, i))) return false;
    return true;
}

// Check |M - R*D*R^T|_F / |M|_F <= tol
static bool robustRecomposeCheck(const DenseMatrix<double,3,3>& M, double rel_tol = 1e-10) {
    auto decomp = MetricDecomposition::decomposeRobust(M);
    auto& R = decomp.R;
    auto& D = decomp.D;
    DenseMatrix<double,3,3> M2 = R * D * R.transpose();
    double ref = 0.0;
    for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) ref += M(i,j)*M(i,j);
    ref = std::sqrt(ref);
    double err = 0.0;
    for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) err += (M(i,j)-M2(i,j))*(M(i,j)-M2(i,j));
    err = std::sqrt(err);
    return (ref < 1e-30) ? (err < 1e-30) : (err / ref <= rel_tol);
}

// Check R^T * R ≈ I
static bool isOrthogonal(const DenseMatrix<double,3,3>& R, double tol = 1e-10) {
    auto I = R.transpose() * R;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) {
            double expected = (i == j) ? 1.0 : 0.0;
            if (std::abs(I(i,j) - expected) > tol) return false;
        }
    return true;
}

TEST_CASE("decomposeRobust: recompose recovers original matrix") {
    SECTION("identity matrix") {
        DenseMatrix<double,3,3> M{{1,0,0},{0,1,0},{0,0,1}};
        REQUIRE(robustRecomposeCheck(M));
    }

    SECTION("diagonal matrix") {
        DenseMatrix<double,3,3> M{{3,0,0},{0,2,0},{0,0,1}};
        REQUIRE(robustRecomposeCheck(M));
        auto decomp = MetricDecomposition::decomposeRobust(M);
        REQUIRE(isOrthogonal(decomp.R));
    }

    SECTION("all repeated eigenvalues") {
        DenseMatrix<double,3,3> M{{5,0,0},{0,5,0},{0,0,5}};
        REQUIRE(robustRecomposeCheck(M));
    }

    SECTION("generic symmetric matrix") {
        DenseMatrix<double,3,3> M{{4,1,2},{1,3,0},{2,0,5}};
        REQUIRE(robustRecomposeCheck(M));
        auto decomp = MetricDecomposition::decomposeRobust(M);
        REQUIRE(isOrthogonal(decomp.R));
    }

    SECTION("tanh3 failing case: large negative (0,2) entry") {
        // This matrix caused decompose() to fail because eliminate02WithHouseholder
        // compared a02 signed (a02=-20 <= eps was true, skipping the reduction).
        DenseMatrix<double,3,3> M{{0,0,-20},{0,0,0},{-20,0,0}};
        REQUIRE(robustRecomposeCheck(M));
        auto decomp = MetricDecomposition::decomposeRobust(M);
        REQUIRE(isOrthogonal(decomp.R));
    }

    SECTION("tanh3 failing case: large negative (0,2) entry variant") {
        DenseMatrix<double,3,3> M{{0,0,-10},{0,0,0},{-10,0,0}};
        REQUIRE(robustRecomposeCheck(M));
        auto decomp = MetricDecomposition::decomposeRobust(M);
        REQUIRE(isOrthogonal(decomp.R));
    }

    SECTION("near-zero matrix") {
        DenseMatrix<double,3,3> M{{0,0,0},{0,0,0},{0,0,0}};
        REQUIRE(robustRecomposeCheck(M, 1.0));  // tol is irrelevant; everything should be ~0
    }

    SECTION("large anisotropy ratio") {
        DenseMatrix<double,3,3> M{{1e6,0,0},{0,1e3,0},{0,0,1}};
        REQUIRE(robustRecomposeCheck(M));
        auto decomp = MetricDecomposition::decomposeRobust(M);
        REQUIRE(isOrthogonal(decomp.R));
    }

    SECTION("two repeated eigenvalues") {
        DenseMatrix<double,3,3> M{{2,0,0},{0,2,0},{0,0,7}};
        REQUIRE(robustRecomposeCheck(M));
    }

    SECTION("existing test: near repeated eigenvalues") {
        DenseMatrix<double,3,3> M{{12345234,245,1700},{245,45,5},{1700,5,24000}};
        REQUIRE(robustRecomposeCheck(M));
        auto decomp = MetricDecomposition::decomposeRobust(M);
        REQUIRE(isOrthogonal(decomp.R));
    }

    SECTION("existing test: negative off-diagonals") {
        DenseMatrix<double,3,3> M{
            {9.686227951973914e-02, -9.052415041747888e-19, 7.344156315669605e-01},
            {-7.499352197680383e-19, -9.987384442437347e-02, -5.089133158499858e-19},
            {7.344156315669608e-01, -1.686454909073156e-18, 3.011564904634806e-03}};
        REQUIRE(robustRecomposeCheck(M));
    }
}

TEST_CASE("decomposeRobust: sign bug fix — negative a02 no longer skips Householder") {
    // Before the sign bug fix, decompose() used `a02 <= target_error` (signed),
    // which is true for a02 = -20, causing the Householder step to be skipped.
    // decomposeRobust uses the analytical path and must not exhibit this failure.
    DenseMatrix<double,3,3> M{{0,0,-20},{0,0,0},{-20,0,0}};
    // Should not throw and must recompose correctly
    REQUIRE_NOTHROW(MetricDecomposition::decomposeRobust(M));
    REQUIRE(robustRecomposeCheck(M));
}

TEST_CASE("Diagonalize symmetric 3x3 matrices") {
    SECTION("all zero matrix") {
        DenseMatrix<double, 3, 3> M{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

        double target_error = MetricDecomposition::calcTargetError(M);

        auto decomp = MetricDecomposition::decompose(M);

        double error = MetricDecomposition::frobeniusOfOffDiagonals(decomp.D);
        REQUIRE(error <= target_error);

        bool same = isReformedMatrixEquivalentToOriginal(decomp, M, target_error);
        REQUIRE(same);
    }

    SECTION("near repeated eigenvalues") {
        DenseMatrix<double, 3, 3> M{{12345234, 245, 1700}, {245, 45, 5}, {1700, 5, 24000}};
        double target_error = MetricDecomposition::calcTargetError(M);

        auto decomp = MetricDecomposition::decompose(M);

        double error = MetricDecomposition::frobeniusOfOffDiagonals(decomp.D);
        REQUIRE(error <= target_error);

        bool same = isReformedMatrixEquivalentToOriginal(decomp, M, target_error);
        REQUIRE(same);
    }

    SECTION("already diagonal matrix") {
        DenseMatrix<double, 3, 3> M{{3, 0, 0}, {0, 2, 0}, {0, 0, 1}};
        double target_error = MetricDecomposition::calcTargetError(M);

        auto decomp = MetricDecomposition::decompose(M);

        double error = MetricDecomposition::frobeniusOfOffDiagonals(decomp.D);
        REQUIRE(error <= target_error);

        bool same = isReformedMatrixEquivalentToOriginal(decomp, M, target_error);
        REQUIRE(same);
    }

    SECTION("diagonal and repeated") {
        DenseMatrix<double, 3, 3> M{{10, 0, 0}, {0, 10, 0}, {0, 0, 10}};
        double target_error = MetricDecomposition::calcTargetError(M);

        auto decomp = MetricDecomposition::decompose(M);

        double error = MetricDecomposition::frobeniusOfOffDiagonals(decomp.D);
        REQUIRE(error <= target_error);

        bool same = isReformedMatrixEquivalentToOriginal(decomp, M, target_error);
        REQUIRE(same);
    }

    SECTION("negative off-diagonals") {
        DenseMatrix<double, 3, 3> M{{9.686227951973914e-02, -9.052415041747888e-19, 7.344156315669605e-01},
                                    {-7.499352197680383e-19, -9.987384442437347e-02, -5.089133158499858e-19},
                                    {7.344156315669608e-01, -1.686454909073156e-18, 3.011564904634806e-03}};

        double target_error = MetricDecomposition::calcTargetError(M);

        auto decomp = MetricDecomposition::decompose(M);
        double error = MetricDecomposition::frobeniusOfOffDiagonals(decomp.D);
        REQUIRE(error <= target_error);
    }

    SECTION("negative on diagonal") {
        DenseMatrix<double, 3, 3> M{{1.411938695674686e-02, 0.000000000000000e+00, 5.993324183457549e-03},
                                    {0.000000000000000e+00, 0.000000000000000e+00, 0.000000000000000e+00},
                                    {5.993324183457993e-03, 0.000000000000000e+00, -1.411938695674786e-02}};

        double target_error = MetricDecomposition::calcTargetError(M);

        auto decomp = MetricDecomposition::decompose(M);
        double error = MetricDecomposition::frobeniusOfOffDiagonals(decomp.D);
        REQUIRE(error <= target_error);
    }

    SECTION("not quite symmetric") {
        DenseMatrix<double, 3, 3> M{{2.867134124414472e-01, 1.741156884634754e-02, -9.798589635396433e-02},
                                    {1.741157054252079e-02, 1.220067339719620e-01, -3.741481825674488e-03},
                                    {-9.798590557351100e-02, -3.741481791361987e-03, 1.437005142189077e-01}};

        double target_error = MetricDecomposition::calcTargetError(M);

        auto decomp = MetricDecomposition::decompose(M);
        double error = MetricDecomposition::frobeniusOfOffDiagonals(decomp.D);
        REQUIRE(error <= target_error);
    }
}
