
// Copyright 2016 United States Government as represented by the Administrator of the National Aeronautics and Space
// Administration. No copyright is claimed in the United States under Title 17, U.S. Code. All Other Rights Reserved.
//
// The “Parfait: A Toolbox for CFD Software Development [LAR-18839-1]” platform is licensed under the
// Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License.
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0.
//
// Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
// on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and limitations under the License.
#include <numeric>
#include <RingAssertions.h>
#include <parfait/CGNSElements.h>
#include <parfait/DualMetrics.h>

TEST_CASE("Triangle Edge to node") {
    auto& edge_to_node = Parfait::CGNS::Triangle::edge_to_node;
    REQUIRE((edge_to_node[0] == std::array<int, 2>{0, 1}));
    REQUIRE((edge_to_node[1] == std::array<int, 2>{1, 2}));
    REQUIRE((edge_to_node[2] == std::array<int, 2>{2, 0}));
}

TEST_CASE("Triangle Center") {
    std::array<Parfait::Point<double>, 3> triangle;
    triangle[0] = {0, 0, 0};
    triangle[1] = {1, 0, 0};
    triangle[2] = {0, 1, 0};

    auto center = std::accumulate(triangle.begin(), triangle.end(), Parfait::Point<double>{}) / 3.0;
    REQUIRE(center.approxEqual(Parfait::CGNS::Triangle::computeCenter(triangle)));
}

TEST_CASE("Triangle Edge Centers") {
    std::array<Parfait::Point<double>, 3> triangle;
    triangle[0] = {0, 0, 0};
    triangle[1] = {1, 0, 0};
    triangle[2] = {0, 1, 0};

    auto edge_centers = Parfait::CGNS::Triangle::computeEdgeCenters(triangle);
    REQUIRE(edge_centers.size() == 3);

    REQUIRE(edge_centers[0][0] == Approx(0.5));
    REQUIRE(edge_centers[0][1] == Approx(0.0));
    REQUIRE(edge_centers[0][2] == Approx(0.0));

    REQUIRE(edge_centers[1][0] == Approx(0.5));
    REQUIRE(edge_centers[1][1] == Approx(0.5));
    REQUIRE(edge_centers[1][2] == Approx(0.0));

    REQUIRE(edge_centers[2][0] == Approx(0.0));
    REQUIRE(edge_centers[2][1] == Approx(0.5));
    REQUIRE(edge_centers[2][2] == Approx(0.0));
}
