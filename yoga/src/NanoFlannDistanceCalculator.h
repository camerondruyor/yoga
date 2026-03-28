#pragma once
#include <MessagePasser/MessagePasser.h>
#include <nanoflann.hpp>
#include <thread>
#include <vector>
#include "YogaMesh.h"

namespace YOGA {
class NanoFlannDistanceCalculator {
  public:
    NanoFlannDistanceCalculator(const std::vector<std::vector<Parfait::Point<double>>>& surfaces) {
        for (size_t i = 0; i < surfaces.size(); ++i) {
            auto& surface = surfaces[i];
            auto point_cloud_ptr = std::make_shared<PointCloudAdapter>(surface);
            point_clouds.push_back(point_cloud_ptr);
            trees.push_back(std::make_shared<nano_flann_kd_tree>(
                3, *point_cloud_ptr.get(), nanoflann::KDTreeSingleIndexAdaptorParams(10)));
        }
        for (auto& tree : trees) {
            std::this_thread::yield();
            tree->buildIndex();
        }
    }

    double calcDistance(const Parfait::Point<double>& p,int component){
        auto& tree = trees[component];
        auto& cloud = *point_clouds[component];
        return std::sqrt(getSquaredDistance(*tree, cloud, p));
    }

    std::vector<double> calculateDistances(const std::vector<Parfait::Point<double>>& points,
                                           const std::vector<int>& grid_ids_for_nodes) {
        std::vector<double> distance(points.size());
        for(size_t i=0;i<points.size();i++) {
            int component = grid_ids_for_nodes[i];
            auto& tree = trees[component];
            auto& cloud = *point_clouds[component];
            auto& p = points[i];
            distance[i] = std::sqrt(getSquaredDistance(*tree, cloud, p));
        }
        return distance;
    }

  private:
    double MAX_DISTANCE = 1.0e10;
    class PointCloudAdapter;

    typedef nanoflann::
        KDTreeSingleIndexAdaptor<nanoflann::L2_Simple_Adaptor<double, PointCloudAdapter>, PointCloudAdapter, 3>
            nano_flann_kd_tree;

    std::vector<std::shared_ptr<PointCloudAdapter>> point_clouds;
    std::vector<std::shared_ptr<nano_flann_kd_tree>> trees;

    double getSquaredDistance(const nano_flann_kd_tree& tree, const PointCloudAdapter& cloud, const Parfait::Point<double>& p){
        if(cloud.kdtree_get_point_count() == 0){
            return MAX_DISTANCE;
        }
        const size_t num_results = 1;
        size_t ret_index;
        double out_dist_sqr;
        nanoflann::KNNResultSet<double> resultSet(num_results);
        resultSet.init(&ret_index, &out_dist_sqr);
        tree.findNeighbors(resultSet, p.data(), nanoflann::SearchParameters());
        return out_dist_sqr;
    }

    class PointCloudAdapter {
      public:
        PointCloudAdapter(const std::vector<Parfait::Point<double>>& pts) : pts(pts) {}

        inline size_t kdtree_get_point_count() const { return pts.size(); }

        inline double kdtree_get_pt(const size_t idx, const size_t dim) const { return pts[idx][dim]; }

        template <class BBOX>
        bool kdtree_get_bbox(BBOX&) const {
            return false;
        }

      private:
        const std::vector<Parfait::Point<double>>& pts;
    };
};
}
