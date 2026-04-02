#pragma once
#include "DenseMatrix.h"

namespace Parfait {

class MetricDecomposition {
  public:
    struct Decomposition {
        DenseMatrix<double, 3, 3> D;
        DenseMatrix<double, 3, 3> R;
    };

    static Decomposition decompose(const DenseMatrix<double, 3, 3>& M);

    // Analytical eigendecomposition using Cardano's method + cross-product
    // eigenvectors.  Always succeeds for any symmetric 3×3 matrix — no iteration.
    static Decomposition decomposeRobust(const DenseMatrix<double, 3, 3>& M);

    static DenseMatrix<double, 3, 3> recompose(const Decomposition& d);

    static double calcTargetError(const DenseMatrix<double, 3, 3>& M);

    static double frobeniusOfOffDiagonals(const DenseMatrix<double, 3, 3>& M);

    static void printMatrixAsUnitTestFixture(const DenseMatrix<double, 3, 3>& M);

    static std::string formatMatrixAsUnitTestFixture(const DenseMatrix<double, 3, 3>& M);

    static void throwFailedToDiagonalize(const DenseMatrix<double, 3, 3>& M);

    static bool iterativelyDiagonalize(DenseMatrix<double, 3, 3>& R, DenseMatrix<double, 3, 3>& M);

  private:
    static void forceSymmetry(DenseMatrix<double, 3, 3>& m);

    static bool shouldReduce12Entry(DenseMatrix<double, 3, 3>& B);

    static DenseMatrix<double, 3, 3> eliminate02WithHouseholderReflection(DenseMatrix<double, 3, 3>& M);

    static DenseMatrix<double, 3, 3> reduce01WithGivensRotation(DenseMatrix<double, 3, 3>& B);
    static DenseMatrix<double, 3, 3> reduce12WithGivensRotation(DenseMatrix<double, 3, 3>& B);
};
}
