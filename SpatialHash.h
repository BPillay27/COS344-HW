#ifndef SPATIAL_HASH_H
#define SPATIAL_HASH_H

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include <functional>

/**
 * @class SpatialHash
 * @brief 3D spatial hash grid for efficient spatial queries and collision detection
 * 
 * Divides 3D space into uniform cells and maintains a hash map of objects in each cell.
 * Allows fast queries for objects near a given position.
 */
class SpatialHash {
public:
    /**
     * Constructor
     * @param cellSize Size of each cell in the hash grid
     */
    explicit SpatialHash(float cellSize = 1.0f);

    /**
     * Insert an object at the given position
     * @param id Unique identifier for the object
     * @param position Position in 3D space
     */
    void insert(int id, const glm::vec3& position);

    /**
     * Remove an object from the hash
     * @param id Unique identifier for the object
     * @param position Previous position of the object (needed to find which cell)
     */
    void remove(int id, const glm::vec3& position);

    /**
     * Update an object's position
     * @param id Unique identifier for the object
     * @param oldPosition Previous position
     * @param newPosition New position
     */
    void update(int id, const glm::vec3& oldPosition, const glm::vec3& newPosition);

    /**
     * Query all objects within a radius of a given position
     * @param position Query position
     * @param radius Search radius
     * @return Vector of object IDs found within the radius
     */
    std::vector<int> queryRadius(const glm::vec3& position, float radius) const;

    /**
     * Query all objects in adjacent cells (3x3x3 cube) around a position
     * @param position Query position
     * @return Vector of object IDs found in nearby cells
     */
    std::vector<int> queryNearby(const glm::vec3& position) const;

    /**
     * Clear all objects from the hash
     */
    void clear();

    /**
     * Get the number of objects currently in the hash
     * @return Total object count
     */
    int getObjectCount() const;

    /**
     * Get cell size
     * @return Current cell size
     */
    float getCellSize() const { return cellSize; }

    /**
     * Set cell size (clears existing data)
     * @param newCellSize New size for cells
     */
    void setCellSize(float newCellSize);

private:
    struct CellKey {
        int x, y, z;

        CellKey(int x, int y, int z) : x(x), y(y), z(z) {}

        bool operator==(const CellKey& other) const {
            return x == other.x && y == other.y && z == other.z;
        }
    };

    struct CellKeyHash {
        std::size_t operator()(const CellKey& key) const {
            // Hash function combining three integers
            std::hash<int> hasher;
            std::size_t h1 = hasher(key.x);
            std::size_t h2 = hasher(key.y);
            std::size_t h3 = hasher(key.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };

    /**
     * Convert a world position to a cell key
     * @param position World position
     * @return Cell key for the position
     */
    CellKey getCell(const glm::vec3& position) const;

    float cellSize;
    std::unordered_map<CellKey, std::vector<int>, CellKeyHash> grid;
};

#endif // SPATIAL_HASH_H
