#include "AnisotropicCartMesh.h"
#include "CartMesh.h"
#include <cmath>
#include <parfait/Extent.h>

namespace {
void redistributePoints(inf::TinfMeshData& mesh,
                        const inf::AnisotropicCartMesh::Distribution1D& x_dist,
                        const inf::AnisotropicCartMesh::Distribution1D& y_dist,
                        const inf::AnisotropicCartMesh::Distribution1D& z_dist,
                        const Parfait::Extent<double>& e) {
    double x_lo = e.lo[0], x_hi = e.hi[0];
    double y_lo = e.lo[1], y_hi = e.hi[1];
    double z_lo = e.lo[2], z_hi = e.hi[2];
    for (auto& p : mesh.points) {
        double tx = (p[0] - x_lo) / (x_hi - x_lo);
        double ty = (p[1] - y_lo) / (y_hi - y_lo);
        double tz = (p[2] - z_lo) / (z_hi - z_lo);
        p[0] = x_dist(tx);
        p[1] = y_dist(ty);
        p[2] = z_dist(tz);
    }
}
}

std::shared_ptr<inf::TinfMesh> inf::AnisotropicCartMesh::create(int nx,
                                                                 int ny,
                                                                 int nz,
                                                                 Distribution1D x_dist,
                                                                 Distribution1D y_dist,
                                                                 Distribution1D z_dist) {
    Parfait::Extent<double> unit_cube = {{0, 0, 0}, {1, 1, 1}};
    auto mesh = inf::CartMesh::create(nx, ny, nz, unit_cube);
    redistributePoints(mesh->mesh, x_dist, y_dist, z_dist, unit_cube);
    return mesh;
}

std::shared_ptr<inf::TinfMesh> inf::AnisotropicCartMesh::create(MessagePasser mp,
                                                                 int nx,
                                                                 int ny,
                                                                 int nz,
                                                                 Distribution1D x_dist,
                                                                 Distribution1D y_dist,
                                                                 Distribution1D z_dist) {
    Parfait::Extent<double> unit_cube = {{0, 0, 0}, {1, 1, 1}};
    auto mesh = inf::CartMesh::create(mp, nx, ny, nz, unit_cube);
    redistributePoints(mesh->mesh, x_dist, y_dist, z_dist, unit_cube);
    return mesh;
}

inf::AnisotropicCartMesh::Distribution1D inf::AnisotropicCartMesh::uniform(double lo, double hi) {
    return [lo, hi](double t) { return lo + t * (hi - lo); };
}

inf::AnisotropicCartMesh::Distribution1D inf::AnisotropicCartMesh::tanhSymmetric(double lo,
                                                                                  double hi,
                                                                                  double beta) {
    return [lo, hi, beta](double t) {
        double xi = 0.5 + std::tanh(beta * (t - 0.5)) / (2.0 * std::tanh(0.5 * beta));
        return lo + xi * (hi - lo);
    };
}

inf::AnisotropicCartMesh::Distribution1D inf::AnisotropicCartMesh::tanhAsymmetric(double lo,
                                                                                   double hi,
                                                                                   double beta) {
    return [lo, hi, beta](double t) {
        double xi = std::tanh(beta * t) / std::tanh(beta);
        return lo + xi * (hi - lo);
    };
}
