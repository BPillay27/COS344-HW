#include "SpatialHash.h"
#include <cmath>
#include <algorithm>

SpatialHash::SpatialHash(float cellSize) : cellSize(cellSize) {
    if (cellSize <= 0.0f) {
        this->cellSize = 1.0f;
    }
}

SpatialHash::CellKey SpatialHash::getCell(const glm::vec3& position) const {
    int x = static_cast<int>(std::floor(position.x / cellSize));
    int y = static_cast<int>(std::floor(position.y / cellSize));
    int z = static_cast<int>(std::floor(position.z / cellSize));
    return CellKey(x, y, z);
}

void SpatialHash::insert(int id, const glm::vec3& position) {
    CellKey key = getCell(position);
    grid[key].push_back(id);
}

void SpatialHash::remove(int id, const glm::vec3& position) {
    CellKey key = getCell(position);
    auto it = grid.find(key);
    if (it != grid.end()) {
        auto& cell = it->second;
        auto objIt = std::find(cell.begin(), cell.end(), id);
        if (objIt != cell.end()) {
            cell.erase(objIt);
        }
        // Remove empty cells to save memory
        if (cell.empty()) {
            grid.erase(it);
        }
    }
}

void SpatialHash::update(int id, const glm::vec3& oldPosition, const glm::vec3& newPosition) {
    CellKey oldKey = getCell(oldPosition);
    CellKey newKey = getCell(newPosition);

    // Only update if the object moved to a different cell
    if (!(oldKey == newKey)) {
        remove(id, oldPosition);
        insert(id, newPosition);
    }
}

std::vector<int> SpatialHash::queryRadius(const glm::vec3& position, float radius) const {
    std::vector<int> result;
    std::vector<int> candidates = queryNearby(position);

    // Filter candidates by actual distance
    float radiusSq = radius * radius;
    for (int id : candidates) {
        // Note: We don't have object positions here, so we return all nearby candidates
        // The caller should filter by distance if needed
        result.push_back(id);
    }

    return result;
}

std::vector<int> SpatialHash::queryNearby(const glm::vec3& position) const {
    std::vector<int> result;
    CellKey center = getCell(position);

    // Query 3x3x3 cube of cells around the position
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dz = -1; dz <= 1; ++dz) {
                CellKey key(center.x + dx, center.y + dy, center.z + dz);
                auto it = grid.find(key);
                if (it != grid.end()) {
                    const auto& cell = it->second;
                    result.insert(result.end(), cell.begin(), cell.end());
                }
            }
        }
    }

    // Remove duplicates if any object appears in multiple cells
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());

    return result;
}

void SpatialHash::clear() {
    grid.clear();
}

int SpatialHash::getObjectCount() const {
    int count = 0;
    for (const auto& pair : grid) {
        count += pair.second.size();
    }
    return count;
}

void SpatialHash::setCellSize(float newCellSize) {
    if (newCellSize <= 0.0f) {
        return;  // Ignore invalid cell sizes
    }
    clear();
    cellSize = newCellSize;
}
