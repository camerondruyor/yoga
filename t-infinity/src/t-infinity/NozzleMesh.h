#pragma once
#include <functional>
#include <memory>
#include <parfait/Extent.h>
#include <MessagePasser/MessagePasser.h>
#include <t-infinity/TinfMesh.h>

namespace inf {
namespace NozzleMesh {

    std::shared_ptr<TinfMesh> create(int nx,
                                     int ny,
                                     int nz,
                                     std::function<double(double)> area,
                                     Parfait::Extent<double> e = {{0, 0, 0}, {1, 1, 1}});

    std::shared_ptr<TinfMesh> create(MessagePasser mp,
                                     int nx,
                                     int ny,
                                     int nz,
                                     std::function<double(double)> area,
                                     Parfait::Extent<double> e = {{0, 0, 0}, {1, 1, 1}});
}
}
