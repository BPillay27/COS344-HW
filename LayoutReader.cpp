#include "LayoutReader.h"
#include "Shape3D.h"
#include "Figure.h"

// Shape includes
#include "shapes/Sphere.h"
#include "shapes/Cylinder.h"
#include "shapes/Cube.h"
#include "shapes/Circle.h"
#include "shapes/Cone.h"
#include "shapes/Triangle.h"
#include "shapes/Square.h"
#include "shapes/TriangularPrism.h"
#include "shapes/SquarePyramid.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

/**
 * Compact format using fprint data:
 * ShapeType | fprint_data | r g b a | shininess | colormap | alphamap | displacementmap | x y z
 * 
 * Example:
 * Sphere | 0.000000 0.000000 0.000000 1.000000 16 32 | 255 255 255 255 | 32.0 | | | | 0.0 0.0 0.0
 */

bool LayoutReader::saveFigure(const std::string& filename, const Figure& figure) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return false;
    }

    for (int i = 0; i < figure.getNumShapes(); ++i) {
        Shape3D* shape3d = figure.getShape(i);
        if (!shape3d) continue;

        Shape<4>* shape = shape3d->getShape();
        if (!shape) continue;

        // Get metadata
        glm::vec4 pos = shape3d->getPosition();
        float* color = shape->getColour();
        float shininess = shape->getShininess();
        std::string colorMap = shape->getColorMapFile();
        std::string alphaMap = shape->getAlphaMapFile();
        std::string displacementMap = shape->getDisplacementMapFile();
        
        // Get fprint WITHOUT color (we'll save color separately)
        std::string fprintData = shape->fprint();
        
        // Remove color from fprint data (it's at the end, last 4 space-separated values)
        // We'll extract just the shape-specific parameters
        std::istringstream fprintStream(fprintData);
        std::string fprintNoColor;
        std::string token;
        int colorValueCount = 0;
        std::vector<std::string> tokens;
        while (fprintStream >> token) {
            tokens.push_back(token);
        }
        // Remove last 4 tokens (r g b a from fprint)
        if (tokens.size() >= 4) {
            tokens.erase(tokens.end() - 4, tokens.end());
        }
        // Reconstruct fprint without color
        for (size_t j = 0; j < tokens.size(); ++j) {
            if (j > 0) fprintNoColor += " ";
            fprintNoColor += tokens[j];
        }

        // Format: ShapeType | fprint_data | r g b a | shininess | colormap | alphamap | displacementmap | x y z
        file << getShapeTypeName(shape) << " | "
             << fprintNoColor << " | "
             << (int)(color[0] * 255) << " " << (int)(color[1] * 255) << " " 
             << (int)(color[2] * 255) << " " << (int)(color[3] * 255) << " | "
             << shininess << " | "
             << colorMap << " | "
             << alphaMap << " | "
             << displacementMap << " | "
             << pos.x << " " << pos.y << " " << pos.z << "\n";
    }

    file.close();
    std::cout << "Figure saved to: " << filename << std::endl;
    return true;
}

bool LayoutReader::loadFigure(const std::string& filename, Figure& outFigure) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for reading: " << filename << std::endl;
        return false;
    }

    std::string line;
    int shapesLoaded = 0;

    while (std::getline(file, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty() || line[0] == '#') continue;  // Skip empty lines and comments

        // Split by '|' delimiter
        std::vector<std::string> parts;
        std::istringstream iss(line);
        std::string part;
        while (std::getline(iss, part, '|')) {
            // Trim each part
            part.erase(0, part.find_first_not_of(" \t"));
            part.erase(part.find_last_not_of(" \t") + 1);
            if (!part.empty()) {
                parts.push_back(part);
            }
        }

        if (parts.size() < 8) {
            std::cerr << "Invalid format in line: " << line << std::endl;
            continue;
        }

        std::string shapeType = parts[0];
        std::string fprintData = parts[1];
        
        // Parse color (r g b a)
        float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        std::istringstream colorStream(parts[2]);
        int r, g, b, a;
        if (colorStream >> r >> g >> b >> a) {
            color[0] = r / 255.0f;
            color[1] = g / 255.0f;
            color[2] = b / 255.0f;
            color[3] = a / 255.0f;
        }
        
        float shininess = std::stof(parts[3]);
        std::string colorMap = parts[4];
        std::string alphaMap = parts[5];
        std::string displacementMap = parts[6];

        // Parse position from last part
        std::istringstream posStream(parts[7]);
        float x, y, z;
        if (!(posStream >> x >> y >> z)) {
            std::cerr << "Failed to parse position: " << parts[6] << std::endl;
            continue;
        }

        // Create shape from type and fprint data
        Shape<4>* newShape = createShapeFromFprint(shapeType, fprintData);
        if (!newShape) {
            std::cerr << "Failed to create shape of type: " << shapeType << std::endl;
            continue;
        }

        // Apply additional properties
        newShape->setShininess(shininess);
        newShape->setColour((int)(color[0] * 255), (int)(color[1] * 255), 
                           (int)(color[2] * 255), color[3]);
        newShape->setTextureMap(colorMap, alphaMap, displacementMap);

        // Create Shape3D wrapper and add to figure
        Shape3D* shape3d = new Shape3D(newShape);
        shape3d->move(x, y, z);
        outFigure.addShape3D(shape3d);
        shapesLoaded++;
    }

    file.close();
    std::cout << "Loaded " << shapesLoaded << " shapes from: " << filename << std::endl;
    return shapesLoaded > 0;
}

std::string LayoutReader::getShapeTypeName(Shape<4>* shape) {
    if (dynamic_cast<Sphere<4>*>(shape)) return "Sphere";
    if (dynamic_cast<Cylinder<4>*>(shape)) return "Cylinder";
    if (dynamic_cast<Cube<4>*>(shape)) return "Cube";
    if (dynamic_cast<Circle<4>*>(shape)) return "Circle";
    if (dynamic_cast<Cone<4>*>(shape)) return "Cone";
    if (dynamic_cast<Triangle<4>*>(shape)) return "Triangle";
    if (dynamic_cast<Square<4>*>(shape)) return "Square";
    if (dynamic_cast<TriangularPrism<4>*>(shape)) return "TriangularPrism";
    if (dynamic_cast<SquarePyramid<4>*>(shape)) return "SquarePyramid";
    return "Unknown";
}

Shape<4>* LayoutReader::createShapeFromFprint(const std::string& typeName, const std::string& fprintData) {
    std::istringstream iss(fprintData);
    
    if (typeName == "Sphere") {
        // Format: center.x center.y center.z radius stacks slices (no color - separate)
        float x, y, z, radius;
        int stacks, slices;
        if (!(iss >> x >> y >> z >> radius >> stacks >> slices)) {
            return nullptr;
        }
        Sphere<4>* sphere = new Sphere<4>(glm::vec4(x, y, z, 1.0f), radius, stacks, slices);
        return sphere;
    }
    
    if (typeName == "Cylinder") {
        // Format: center.x center.y center.z radius height resolution angleOffset (no color - separate)
        float x, y, z, radius, height, angleOffset;
        int resolution;
        if (!(iss >> x >> y >> z >> radius >> height >> resolution >> angleOffset)) {
            return nullptr;
        }
        Cylinder<4>* cylinder = new Cylinder<4>(glm::vec4(x, y, z, 1.0f), radius, height, resolution);
        return cylinder;
    }
    
    if (typeName == "Cube") {
        // Create with default params
        Cube<4>* cube = new Cube<4>();
        return cube;
    }
    
    if (typeName == "Circle") {
        // Format: center.x center.y center.z radius segments (no color - separate)
        float x, y, z, radius;
        int segments;
        if (!(iss >> x >> y >> z >> radius >> segments)) {
            return nullptr;
        }
        Circle<4>* circle = new Circle<4>(glm::vec4(x, y, z, 1.0f), radius, segments);
        return circle;
    }
    
    if (typeName == "Cone") {
        // Format: apex.x apex.y apex.z baseCenter.x baseCenter.y baseCenter.z radius height stacks (no color - separate)
        float apex_x, apex_y, apex_z, base_x, base_y, base_z, radius, height;
        int stacks;
        if (!(iss >> apex_x >> apex_y >> apex_z >> base_x >> base_y >> base_z >> radius >> height >> stacks)) {
            return nullptr;
        }
        Cone<4>* cone = new Cone<4>(glm::vec4(apex_x, apex_y, apex_z, 1.0f), radius, height, stacks);
        return cone;
    }
    
    if (typeName == "Triangle") {
        // Default triangle
        Triangle<4>* triangle = new Triangle<4>();
        return triangle;
    }
    
    if (typeName == "Square") {
        // Format: center.x center.y center.z height width (no color - separate)
        float x, y, z, height, width;
        if (!(iss >> x >> y >> z >> height >> width)) {
            return nullptr;
        }
        Square<4>* square = new Square<4>(glm::vec4(x, y, z, 1.0f), height, width);
        return square;
    }
    
    if (typeName == "TriangularPrism") {
        // TriangularPrism is abstract - skip
        std::cerr << "TriangularPrism is abstract and cannot be loaded" << std::endl;
        return nullptr;
    }
    
    if (typeName == "SquarePyramid") {
        // Format: apex.x apex.y apex.z baseCenter.x baseCenter.y baseCenter.z baseSize (no color - separate)
        float apex_x, apex_y, apex_z, base_x, base_y, base_z, baseSize;
        if (!(iss >> apex_x >> apex_y >> apex_z >> base_x >> base_y >> base_z >> baseSize)) {
            return nullptr;
        }
        SquarePyramid<4>* pyramid = new SquarePyramid<4>(
            glm::vec4(apex_x, apex_y, apex_z, 1.0f),
            glm::vec4(base_x, base_y, base_z, 1.0f),
            baseSize
        );
        return pyramid;
    }
    
    std::cerr << "Unknown shape type: " << typeName << std::endl;
    return nullptr;
}
