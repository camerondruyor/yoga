#include <RingAssertions.h>
#include <parfait/DenseMatrix.h>
#include <parfait/MetricDecomposition.h>
#include <parfait/VTKWriter.h>
#include <parfait/CGNSElements.h>
#include <parfait/MotionMatrix.h>
#include <parfait/MetricTensor.h>

using namespace Parfait;

void plotMetric(const std::vector<Point<double>>& tet,
                const DenseMatrix<double, 3, 3>& metric,
                const std::string& filename) {
    auto decomp = MetricDecomposition::decompose(metric);

    for (int i = 0; i < 3; i++) decomp.D(i, i) = (1.0) / decomp.D(i, i);

    auto R = decomp.R;
    auto Rt = R;
    Rt.transpose();
    auto rebuilt = R * decomp.D * Rt;
    auto getPoint = [&](int id, double* p) {
        for (int i = 0; i < 3; i++) p[i] = tet[id][i];
    };

    auto getCellType = [](int id) { return vtk::Writer::TET; };

    auto getNodeIds = [](int cell_id, int* node_ids) {
        for (int i = 0; i < 4; i++) node_ids[i] = i;
    };

    std::array<double, 6> M;
    M[0] = rebuilt(0, 0);
    M[1] = rebuilt(0, 1);
    M[2] = rebuilt(0, 2);
    M[3] = rebuilt(1, 1);
    M[4] = rebuilt(1, 2);
    M[5] = rebuilt(2, 2);

    auto getMetric = [&](int node_id, double* m) {
        for (int i = 0; i < 6; i++) m[i] = M[i];
    };

    vtk::Writer writer(filename, 4, 1, getPoint, getCellType, getNodeIds);
    writer.addNodeTensorField("metric", getMetric);
    writer.visualize();
}

void print(const DenseMatrix<double, 3, 3>& M) {
    for (int i = 0; i < 3; i++) {
        printf("\n");
        for (int j = 0; j < 3; j++) printf("%f ", M(i, j));
    }
    printf("\n");
}

void printEigenValues(const DenseMatrix<double, 3, 3>& M, const std::string s) {
    auto decomp = MetricDecomposition::decompose(M);
    double sum = 0.0;
    for (int i = 0; i < 3; i++) {
        printf("%f ", decomp.D(i, i));
        sum += decomp.D(i, i);
    }
    printf("     sum: %f\n", sum);
}

TEST_CASE("Log-Euclidean metric is isometric (preserves distance under similarity transform)") {
    double h1 = 4;
    double h2 = 3;
    double h3 = 2;
    double alpha_0 = 0;
    double alpha_1 = 0;
    double beta_0 = 0;
    double beta_1 = 0;
    double gamma_0 = 0;
    double gamma_1 = 0;

    SECTION("The metric between two tensors is invariant to rotation in alpha") {
        alpha_1 = 5.0;

        auto metric_a = MetricTensor::metricFromEllipse(h1, h2, h3, alpha_0, beta_0, gamma_0);
        auto metric_b = MetricTensor::metricFromEllipse(h1, h2, h3, alpha_1, beta_1, gamma_1);
        double dist = MetricTensor::logEuclideanDistance(metric_a, metric_b);

        alpha_0 += 1.1;
        alpha_1 += 1.1;
        metric_a = MetricTensor::metricFromEllipse(h1, h2, h3, alpha_0, beta_0, gamma_0);
        metric_b = MetricTensor::metricFromEllipse(h1, h2, h3, alpha_1, beta_1, gamma_1);
        double dist2 = MetricTensor::logEuclideanDistance(metric_a, metric_b);

        REQUIRE(dist > 0.0);
        REQUIRE(dist == Approx(dist2));
    }

    SECTION("The metric between two tensors is invariant to rotation in beta") {
        beta_1 = 44.0;

        auto metric_a = MetricTensor::metricFromEllipse(h1, h2, h3, alpha_0, beta_0, gamma_0);
        auto metric_b = MetricTensor::metricFromEllipse(h1, h2, h3, alpha_1, beta_1, gamma_1);
        double dist = MetricTensor::logEuclideanDistance(metric_a, metric_b);

        beta_0 += 1.1;
        beta_1 += 1.1;
        metric_a = MetricTensor::metricFromEllipse(h1, h2, h3, alpha_0, beta_0, gamma_0);
        metric_b = MetricTensor::metricFromEllipse(h1, h2, h3, alpha_1, beta_1, gamma_1);
        double dist2 = MetricTensor::logEuclideanDistance(metric_a, metric_b);

        REQUIRE(dist > 0.0);
        REQUIRE(dist == Approx(dist2));
    }

    SECTION("The path between 2 tensors is a geodesic in the domain of logarithms") {
        alpha_1 = 45.0;
        auto metric_a = MetricTensor::metricFromEllipse(h1, h2, h3, alpha_0, beta_0, gamma_0);
        auto metric_b = MetricTensor::metricFromEllipse(h1, h2, h3, alpha_1, beta_1, gamma_1);

        double geodesic_length = MetricTensor::logEuclideanDistance(metric_a, metric_b);

        std::vector<DenseMatrix<double, 3, 3>> tensors{metric_a, metric_b};
        std::vector<double> weights{0, 0};
        int steps = 30;
        for (int k = 0; k <= steps + 1; k++) {
            double dt = 1 / (steps + 1.0);
            double t = k * dt;

            weights[0] = (1 - t);
            weights[1] = t;
            auto combined = MetricTensor::logEuclideanAverage(tensors, weights);

            double distance_to_a = MetricTensor::logEuclideanDistance(combined, metric_a);
            double distance_to_b = MetricTensor::logEuclideanDistance(combined, metric_b);
            double total = distance_to_a + distance_to_b;
            REQUIRE(geodesic_length == Approx(total));
        }
    }
}

double maxDifference(const DenseMatrix<double, 3, 3>& A, const DenseMatrix<double, 3, 3>& B) {
    double diff = 0.0;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++) diff = std::max(diff, std::abs(A(i, j) - B(i, j)));
    return diff;
}

TEST_CASE("invert tensor") {
    auto M = MetricTensor::metricFromEllipse(2, 3, 4, 0.5, 1.2, 3.0);
    auto M_inv = MetricTensor::invert(M);
    auto result = M_inv * M;
    auto identity = DenseMatrix<double, 3, 3>::Identity();
    double error = maxDifference(identity, result);
    REQUIRE(error < 1.0e-6);
}

void forceSymmetry(DenseMatrix<double, 3, 3>& M) {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < i; j++) M(i, j) = M(j, i);
}

TEST_CASE("Alauzet intersection") {
    Tensor M1 = MetricTensor::metricFromEllipse(2, 3, 4, 0.5, 1.2, 3.0);
    Tensor M2 = MetricTensor::metricFromEllipse(1, 1, 1, 0, 0, 0);

    auto intersection = MetricTensor::intersect(M1, M2);

    // M2 (identity) is more restrictive than M1 (spacings 2,3,4), so intersection == M2.
    double error = maxDifference(M2, intersection);
    REQUIRE(error < 1.0e-10);
}

Parfait::Point<double> calcAngles(const Parfait::Point<double>& v1,
                                  const Parfait::Point<double>& v2,
                                  const Parfait::Point<double>& v3) {
    Parfait::Point<double> angles{0, 0, 0};

    return angles;
}

TEST_CASE("edge length in metric space") {
    SECTION("identity metric yields edge length in cartesian space") {
        auto metric = Tensor::Identity();
        Parfait::Point<double> edge{5, 0, 0};
        REQUIRE(edge.magnitude() == Approx(MetricTensor::edgeLength(metric, edge)));
        edge = {1, 2, 3};
        REQUIRE(edge.magnitude() == Approx(MetricTensor::edgeLength(metric, edge)));
        edge = {0, 0, 3};
        REQUIRE(edge.magnitude() == Approx(MetricTensor::edgeLength(metric, edge)));
        edge = {0, 2, 0};
        REQUIRE(edge.magnitude() == Approx(MetricTensor::edgeLength(metric, edge)));
        edge = {-3, 100, 40};
        REQUIRE(edge.magnitude() == Approx(MetricTensor::edgeLength(metric, edge)));
    }

    auto m = MetricTensor::metricFromEllipse(2.0, 3.0, 4.0, 0, 0, 0);
    Point<double> x_edge{2, 0, 0};
    REQUIRE(1.0 == Approx(MetricTensor::edgeLength(m, x_edge)));
    Point<double> y_edge{0, 3, 0};
    REQUIRE(1.0 == Approx(MetricTensor::edgeLength(m, y_edge)));
    Point<double> z_edge{0, 0, 4};
    REQUIRE(1.0 == Approx(MetricTensor::edgeLength(m, z_edge)));

    Point<double> long_edge{2.01, 0, 0};
    REQUIRE(1.0 < MetricTensor::edgeLength(m, long_edge));

    SECTION("from ellipse based on vectors") {
        Point<double> axis_1{2, 0, 0};
        Point<double> axis_2{0, 3, 0};
        Point<double> axis_3{0, 0, 4};
        MotionMatrix motion;
        motion.setRotation({0, 0, 0}, {0, 0, 1}, 5.0);
        motion.movePoint(axis_1);
        motion.movePoint(axis_2);
        motion.movePoint(axis_3);
        auto m = MetricTensor::metricFromEllipse(axis_1, axis_2, axis_3);

        CHECK(1.0 == Approx(MetricTensor::edgeLength(m, axis_1)));
        CHECK(1.0 == Approx(MetricTensor::edgeLength(m, axis_2)));
        CHECK(1.0 == Approx(MetricTensor::edgeLength(m, axis_3)));
        CHECK(2.0 == Approx(MetricTensor::edgeLength(m, axis_1 * 2.0)));
    }
}

TEST_CASE("Can rotate a metric with motion matrix") {
    Parfait::Point<double> axis_1{5, 0, 0}, axis_2{0, 3, 0}, axis_3{0, 0, 7};
    auto metric_a = MetricTensor::metricFromEllipse(axis_1, axis_2, axis_3);

    Parfait::MotionMatrix rotation;
    rotation.addRotation({0, 0, 0}, {0, 0, 1}, 5);
    rotation.movePoint(axis_1);
    rotation.movePoint(axis_2);
    rotation.movePoint(axis_3);

    auto expected_metric = MetricTensor::metricFromEllipse(axis_1, axis_2, axis_3);

    auto metric_b = MetricTensor::rotate(metric_a, rotation);

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            REQUIRE(expected_metric(i, j) == metric_b(i, j));
        }
    }
}
