#ifndef LAYOUT_READER_H
#define LAYOUT_READER_H

#include <string>
#include <vector>
#include <glm/glm.hpp>

// Forward declarations
class Shape3D;
class Figure;
template<int n> class Shape;

/**
 * @class LayoutReader
 * @brief Saves and loads golf course layouts to/from text files using fprint format
 * 
 * File format (one shape per line):
 * ShapeType | fprint_data | r g b a | shininess | colormap | alphamap | displacementmap | x y z
 * 
 * Supports serialization of:
 * - Shape type and fprint parameters
 * - Color (RGBA)
 * - Position
 * - Shininess
 * - Texture map filenames (color, alpha, displacement)
 */
class LayoutReader {
public:
    /**
     * Save Figure to a text file
     * @param filename Output file path
     * @param figure Figure to save
     * @return true if successful, false otherwise
     */
    static bool saveFigure(const std::string& filename, const Figure& figure);

    /**
     * Load Figure from a text file
     * @param filename Input file path
     * @param outFigure Figure to populate with loaded shapes
     * @return true if successful, false otherwise
     */
    static bool loadFigure(const std::string& filename, Figure& outFigure);

private:
    /**
     * Get string name of shape type
     * @param shape Shape pointer
     * @return Type name string (e.g., "Sphere", "Cylinder")
     */
    static std::string getShapeTypeName(Shape<4>* shape);

    /**
     * Create a new shape instance from type name and fprint data
     * @param typeName Shape type name
     * @param fprintData fprint() format data string
     * @return Pointer to new shape of given type (initialized from fprint data)
     */
    static Shape<4>* createShapeFromFprint(const std::string& typeName, const std::string& fprintData);
};

#endif // LAYOUT_READER_H
