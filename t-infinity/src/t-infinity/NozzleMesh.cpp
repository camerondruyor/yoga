#include "NozzleMesh.h"
#include "CartMesh.h"
#include <cmath>

namespace {
void deformToNozzle(inf::TinfMeshData& mesh,
                    const std::function<double(double)>& area,
                    const Parfait::Extent<double>& e) {
    double y_lo = e.lo[1];
    double y_hi = e.hi[1];
    double z_lo = e.lo[2];
    double z_hi = e.hi[2];
    double y_center = 0.5 * (y_lo + y_hi);
    double z_center = 0.5 * (z_lo + z_hi);
    double a_ref = (y_hi - y_lo) * (z_hi - z_lo);

    for (auto& p : mesh.points) {
        double x = p[0];
        double scale = std::sqrt(area(x) / a_ref);
        p[1] = y_center + (p[1] - y_center) * scale;
        p[2] = z_center + (p[2] - z_center) * scale;
    }
}
}

std::shared_ptr<inf::TinfMesh> inf::NozzleMesh::create(int nx,
                                                        int ny,
                                                        int nz,
                                                        std::function<double(double)> area,
                                                        Parfait::Extent<double> e) {
    auto mesh = inf::CartMesh::create(nx, ny, nz, e);
    deformToNozzle(mesh->mesh, area, e);
    return mesh;
}

std::shared_ptr<inf::TinfMesh> inf::NozzleMesh::create(MessagePasser mp,
                                                        int nx,
                                                        int ny,
                                                        int nz,
                                                        std::function<double(double)> area,
                                                        Parfait::Extent<double> e) {
    auto mesh = inf::CartMesh::create(mp, nx, ny, nz, e);
    deformToNozzle(mesh->mesh, area, e);
    return mesh;
}
