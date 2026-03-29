#include <RingAssertions.h>
#include <t-infinity/NozzleMesh.h>
#include <t-infinity/CartMesh.h>
#include <t-infinity/Cell.h>
#include <cmath>

TEST_CASE("NozzleMesh with constant area matches CartMesh") {
    double area = 0.01;
    Parfait::Extent<double> e = {{0, 0, 0}, {1, 0.1, 0.1}};
    auto nozzle = inf::NozzleMesh::create(10, 2, 2, [=](double) { return area; }, e);
    auto cart = inf::CartMesh::create(10, 2, 2, e);

    REQUIRE(nozzle->nodeCount() == cart->nodeCount());
    REQUIRE(nozzle->cellCount() == cart->cellCount());

    for (int n = 0; n < nozzle->nodeCount(); n++) {
        Parfait::Point<double> pn, pc;
        nozzle->nodeCoordinate(n, pn.data());
        cart->nodeCoordinate(n, pc.data());
        REQUIRE(pn.approxEqual(pc, 1e-14));
    }
}

TEST_CASE("NozzleMesh preserves cell and surface counts") {
    auto area = [](double) { return 0.04; };
    auto mesh = inf::NozzleMesh::create(5, 3, 3, area, {{0, 0, 0}, {1, 0.2, 0.2}});
    long volume = 5 * 3 * 3;
    long xy = 2 * 5 * 3;
    long xz = 2 * 5 * 3;
    long yz = 2 * 3 * 3;
    REQUIRE(mesh->cellCount(inf::MeshInterface::HEXA_8) == volume);
    REQUIRE(mesh->cellCount(inf::MeshInterface::QUAD_4) == xy + xz + yz);
}

TEST_CASE("NozzleMesh deforms cross-section to match area function") {
    double L = 1.0;
    double h = 0.2;
    double a_ref = h * h;

    auto area = [=](double x) { return a_ref * (1.0 + x / L); };

    auto mesh = inf::NozzleMesh::create(10, 2, 2, area, {{0, 0, 0}, {L, h, h}});

    double center = h / 2.0;
    for (int n = 0; n < mesh->nodeCount(); n++) {
        Parfait::Point<double> p;
        mesh->nodeCoordinate(n, p.data());
        double x = p[0];
        double expected_scale = std::sqrt(area(x) / a_ref);
        double expected_half = center * expected_scale;

        REQUIRE(p[1] >= Approx(center - expected_half).margin(1e-14));
        REQUIRE(p[1] <= Approx(center + expected_half).margin(1e-14));
        REQUIRE(p[2] >= Approx(center - expected_half).margin(1e-14));
        REQUIRE(p[2] <= Approx(center + expected_half).margin(1e-14));
    }
}

TEST_CASE("NozzleMesh converging-diverging nozzle has minimum area at throat") {
    double L = 1.0;
    double h = 0.4;
    double a_ref = h * h;
    double a_throat = 0.5 * a_ref;
    double x_throat = L / 2.0;

    auto area = [=](double x) {
        double dx = x - x_throat;
        return a_throat + (a_ref - a_throat) * (2.0 * dx / L) * (2.0 * dx / L);
    };

    auto mesh = inf::NozzleMesh::create(20, 4, 4, area, {{0, 0, 0}, {L, h, h}});

    double center = h / 2.0;
    double min_half_width = h;
    double min_half_x = -1;
    for (int n = 0; n < mesh->nodeCount(); n++) {
        Parfait::Point<double> p;
        mesh->nodeCoordinate(n, p.data());
        double half_width = std::abs(p[1] - center);
        if (half_width > 1e-14 && half_width < min_half_width) {
            min_half_width = half_width;
            min_half_x = p[0];
        }
    }
    REQUIRE(min_half_x == Approx(x_throat).margin(L / 20.0));
}

TEST_CASE("NozzleMesh x-coordinates are unchanged") {
    auto mesh = inf::NozzleMesh::create(
        5, 2, 2, [](double x) { return 0.01 * (1.0 + x); }, {{0, 0, 0}, {1, 0.1, 0.1}});
    auto cart = inf::CartMesh::create(5, 2, 2, {{0, 0, 0}, {1, 0.1, 0.1}});

    for (int n = 0; n < mesh->nodeCount(); n++) {
        Parfait::Point<double> pn, pc;
        mesh->nodeCoordinate(n, pn.data());
        cart->nodeCoordinate(n, pc.data());
        REQUIRE(pn[0] == Approx(pc[0]).margin(1e-14));
    }
}
