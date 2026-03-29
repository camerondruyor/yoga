#pragma once

#include <vector>
#include <string>
#include <parfait/Throw.h>
#include <parfait/ToString.h>
#include "../shared/Reader.h"

#include <parfait/FileTools.h>
#include <parfait/StringTools.h>
#include <t-infinity/TinfMesh.h>
#include <t-infinity/MeshHelpers.h>
#include <t-infinity/Extract.h>

class SU2Reader : public Reader {
  public:
    inline SU2Reader(std::string filename) {
        lines = Parfait::StringTools::split(Parfait::FileTools::loadFileToString(filename), "\n");
        dimension = findIntegerKeyword("NDIME");

        cell_count = findIntegerKeyword("NELEM");
        readCells();

        node_count = findIntegerKeyword("NPOIN");
        readNodes();

        tag_count = findIntegerKeyword("NMARK");
        readBoundaries();

        lines.clear();
        lines.shrink_to_fit();
    }
    inline virtual long nodeCount() const override { return node_count; }
    inline virtual long cellCount(inf::MeshInterface::CellType etype) const override {
        if (cell_tags.count(etype) == 0) return 0;
        return cell_tags.at(etype).size();
    }
    inline virtual std::vector<Parfait::Point<double>> readCoords(long start, long end) const override {
        std::vector<Parfait::Point<double>> out;
        out.reserve(end - start);
        for (long i = start; i < end; i++) {
            out.push_back(points.at(i));
        }
        return out;
    }

    inline virtual std::vector<long> readCells(inf::MeshInterface::CellType type, long start, long end) const override {
        std::vector<long> out;
        if (end <= start) return out;
        if (cell_nodes.count(type) == 0) return out;

        auto length = inf::MeshInterface::cellTypeLength(type);
        auto& elements = cell_nodes.at(type);
        out.reserve(length * (end - start));

        for (long e = start; e < end; e++) {
            for (int i = 0; i < length; i++) {
                out.push_back(elements[length * e + i]);
            }
        }
        return out;
    }
    inline virtual std::vector<int> readCellTags(inf::MeshInterface::CellType type,
                                                 long start,
                                                 long end) const override {
        std::vector<int> out;
        out.reserve(end - start);
        for (long i = start; i < end; i++) {
            out.push_back(cell_tags.at(type)[i]);
        }
        return out;
    }
    inline virtual std::vector<inf::MeshInterface::CellType> cellTypes() const override {
        std::vector<inf::MeshInterface::CellType> types;
        for (auto& pair : cell_tags) {
            types.push_back(pair.first);
        }
        return types;
    }

  public:
    std::vector<std::string> lines;
    int current_line_index = 0;
    int dimension = 3;
    long node_count = 0;
    long cell_count = 0;
    int tag_count = 0;
    std::vector<Parfait::Point<double>> points;
    std::map<inf::MeshInterface::CellType, std::vector<long>> cell_nodes;
    std::map<inf::MeshInterface::CellType, std::vector<int>> cell_tags;
    std::map<int, std::string> tag_names;

    inline void readNodes() {
        using namespace Parfait::StringTools;
        points.resize(node_count);

        int node_index = 0;
        for (int i = current_line_index; i < current_line_index + node_count; i++) {
            auto& l = lines[i];
            auto sanitized_line = findAndReplace(strip(l), "\t", " ");
            auto words = split(sanitized_line, " ");
            Parfait::Point<double> p;
            p[0] = toDouble(words[0]);
            p[1] = toDouble(words[1]);
            if (dimension == 3) {
                p[2] = toDouble(words[2]);
            }
            points[node_index++] = p;
        }

        current_line_index += node_count;
    }

    inline inf::MeshInterface::CellType su2CellTypeToInfType(int su2_type) {
        switch (su2_type) {
            case 3:
                return inf::MeshInterface::BAR_2;
            case 5:
                return inf::MeshInterface::TRI_3;
            case 9:
                return inf::MeshInterface::QUAD_4;
            case 10:
                return inf::MeshInterface::TETRA_4;
            case 12:
                return inf::MeshInterface::HEXA_8;
            case 13:
                return inf::MeshInterface::PENTA_6;
            case 14:
                return inf::MeshInterface::PYRA_5;
            default:
                PARFAIT_THROW("Could not map su2_type to inf: " + std::to_string(su2_type));
        }
    }

    inline void storeCell(std::string line, int tag_to_assign) {
        using namespace Parfait::StringTools;
        auto words = cleanupIntegerListString(strip(line));

        int su2_element_type = toInt(words[0]);
        auto inf_cell_type = su2CellTypeToInfType(su2_element_type);
        int type_length = inf::MeshInterface::cellTypeLength(inf_cell_type);
        for (int i = 1; i <= type_length; i++) {
            cell_nodes[inf_cell_type].push_back(toInt(words[i]));
        }
        cell_tags[inf_cell_type].push_back(tag_to_assign);
    }

    inline void readCells() {
        for (int line_index = current_line_index; line_index < current_line_index + cell_count; line_index++) {
            auto& l = lines[line_index];
            // all "elements" in the su2 format are interior cells and have a default tag of 0
            // note "interior" elements may include triangles and quads, if the mesh is 2D
            int interior_cell_tag = 0;
            storeCell(l, interior_cell_tag);
        }

        current_line_index += cell_count;
    }

    inline void readBoundaries() {
        using namespace Parfait::StringTools;

        for (int tag = 0; tag < tag_count; tag++) {
            tag_names[tag] = findStringKeyword("MARKER_TAG");
            int num_cells_in_tag = findIntegerKeyword("MARKER_ELEMS");

            for (int c = 0; c < num_cells_in_tag; c++) {
                auto& l = lines[current_line_index++];
                storeCell(l, tag);
            }
        }
    }

    inline std::string findStringKeyword(std::string keyword) {
        using namespace Parfait::StringTools;
        for (; current_line_index < int(lines.size()); current_line_index++) {
            auto& l = lines[current_line_index];
            if (startsWith(l, "%")) {
                // skip comment lines
                continue;
            }
            if (contains(l, keyword)) {
                auto line = findAndReplace(findAndReplace(l, "=", " "), "\t", " ");
                auto words = split(line, " ");
                PARFAIT_ASSERT(words[0] == keyword, "Encountered unexpected word on line:\n" + line);
                current_line_index++;
                return strip(words[1]);
            }
        }
        PARFAIT_THROW("Could not find keyword " + keyword);
    }

    inline int findIntegerKeyword(std::string keyword) {
        using namespace Parfait::StringTools;
        return toInt(findStringKeyword(keyword));
    }
};

class SU2Writer {
  public:
    static void write(std::string filename, std::shared_ptr<inf::TinfMesh> gathered_mesh) {
        SU2Writer writer(filename, gathered_mesh);
    }
    SU2Writer(std::string filename, std::shared_ptr<inf::TinfMesh>(gathered_mesh))
        : filename(filename), gathered_mesh(gathered_mesh) {}
    void write() {
        writeDimension();
        writeInteriorCells();
        writePoints();
        writeBoundaries();
        finalize();
    }

  public:
    std::string filename;
    std::shared_ptr<inf::TinfMesh> gathered_mesh;
    int dimension = 3;
    std::string output_file_contents;

    void writeDimension() {
        dimension = inf::is2D(MessagePasser(MPI_COMM_SELF), *gathered_mesh) ? 2 : 3;
        PARFAIT_ASSERT(dimension == 2 or dimension == 3,
                       "Encountered unexpected spatial dimension " + std::to_string(dimension));
        output_file_contents += "NDIME= " + std::to_string(dimension) + "\n";
    }

    void writeInteriorCells() {
        std::set<inf::MeshInterface::CellType> interior_types;
        if (dimension == 2) {
            interior_types.insert(inf::MeshInterface::TRI_3);
            interior_types.insert(inf::MeshInterface::QUAD_4);
        } else if (dimension == 3) {
            interior_types.insert(inf::MeshInterface::TETRA_4);
            interior_types.insert(inf::MeshInterface::PYRA_5);
            interior_types.insert(inf::MeshInterface::PENTA_6);
            interior_types.insert(inf::MeshInterface::HEXA_8);
        }

        int interior_cell_count = 0;
        for (auto type : interior_types) {
            interior_cell_count += gathered_mesh->cellCount(type);
        }

        output_file_contents += "NELEM= " + std::to_string(interior_cell_count) + "\n";
        for (auto t : interior_types) {
            if (gathered_mesh->mesh.cell_tags.count(t) == 0) continue;
            int su2_type = infCellTypeToSU2Type(t);
            int num_cells = gathered_mesh->mesh.cell_tags.at(t).size();
            int cell_legnth = inf::MeshInterface::cellTypeLength(t);
            std::vector<int> cell_nodes(cell_legnth);
            for (int c = 0; c < num_cells; c++) {
                for (int i = 0; i < cell_legnth; i++) {
                    cell_nodes[i] = gathered_mesh->mesh.cells.at(t)[cell_legnth * c + i];
                }
                output_file_contents += std::to_string(su2_type) + " " + Parfait::to_string(cell_nodes) + "\n";
            }
        }
    }
    inline int infCellTypeToSU2Type(inf::MeshInterface::CellType inf_type) {
        switch (inf_type) {
            case inf::MeshInterface::BAR_2:
                return 3;
            case inf::MeshInterface::TRI_3:
                return 5;
            case inf::MeshInterface::QUAD_4:
                return 9;
            case inf::MeshInterface::TETRA_4:
                return 10;
            case inf::MeshInterface::HEXA_8:
                return 12;
            case inf::MeshInterface::PENTA_6:
                return 13;
            case inf::MeshInterface::PYRA_5:
                return 14;
            default:
                PARFAIT_THROW("Could not map inf type to su2: " + inf::MeshInterface::cellTypeString(inf_type));
        }
    }

    void writePoints() {
        output_file_contents += "NPOIN= " + std::to_string(gathered_mesh->nodeCount()) + "\n";
        for (int n = 0; n < gathered_mesh->nodeCount(); n++) {
            Parfait::Point<double> p = gathered_mesh->node(n);
            output_file_contents += p.to_string("%e") + "\n";
        }
    }
    void finalize() { Parfait::FileTools::writeStringToFile(filename, output_file_contents); }

    void writeBoundaries() {
        auto tags = inf::extractTagsWithDimensionality(*gathered_mesh, dimension - 1);
        int num_tags = tags.size();
        output_file_contents += "NMARK= " + std::to_string(num_tags) + "\n";
        for (auto t : tags) {
            output_file_contents +=
                "MARKER_TAG= " + Parfait::StringTools::findAndReplace(gathered_mesh->tagName(t), " ", "_") + "\n";
            auto cell_ids_on_tags = inf::extractCellIdsOnTag(*gathered_mesh, t, dimension - 1);
            output_file_contents += "MARKER_ELEMS= " + std::to_string(cell_ids_on_tags.size()) + "\n";

            std::vector<int> cell_nodes;
            for (auto c : cell_ids_on_tags) {
                cell_nodes = gathered_mesh->cell(c);
                auto type = gathered_mesh->cellType(c);
                int su2_type = infCellTypeToSU2Type(type);
                output_file_contents += std::to_string(su2_type) + " " + Parfait::to_string(cell_nodes) + "\n";
            }
        }
    }
};
