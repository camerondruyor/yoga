#pragma once
#include <functional>
#include <memory>
#include <MessagePasser/MessagePasser.h>
#include <t-infinity/TinfMesh.h>

namespace inf {
namespace AnisotropicCartMesh {

    using Distribution1D = std::function<double(double)>;

    std::shared_ptr<TinfMesh> create(int nx,
                                     int ny,
                                     int nz,
                                     Distribution1D x_dist,
                                     Distribution1D y_dist,
                                     Distribution1D z_dist);

    std::shared_ptr<TinfMesh> create(MessagePasser mp,
                                     int nx,
                                     int ny,
                                     int nz,
                                     Distribution1D x_dist,
                                     Distribution1D y_dist,
                                     Distribution1D z_dist);

    Distribution1D uniform(double lo, double hi);
    Distribution1D tanhSymmetric(double lo, double hi, double beta);
    Distribution1D tanhAsymmetric(double lo, double hi, double beta);
}
}
