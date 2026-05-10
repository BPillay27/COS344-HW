#include <iostream>
#include <sstream>
#include <cmath>

template<int n>
Imported<n>::Imported() : filepath("") {
    // Default constructor
}

template<int n>
Imported<n>::Imported(const std::string& filePath) : filepath(filePath) {
    loadGLTF(filePath);
}

template<int n>
Imported<n>::Imported(const Imported<n>& other) : Shape<n>(other), filepath(other.filepath) {
    meshes = other.meshes;
}

template<int n>
void Imported<n>::loadGLTF(const std::string& filePath) {
    Assimp::Importer importer;
    
    // Load scene with post-processing
    const aiScene* scene = importer.ReadFile(filePath,
        aiProcess_Triangulate |           // Convert all polygons to triangles
        aiProcess_GenNormals |            // Generate normals if not present
        aiProcess_FlipUVs |               // Flip UV coordinates if needed
        aiProcess_JoinIdenticalVertices | // Remove duplicate vertices
        aiProcess_SortByPType);           // Split meshes by primitive type
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    
    // Process all nodes in the scene
    processNode(scene->mRootNode, scene);
}

template<int n>
void Imported<n>::processNode(aiNode* node, const aiScene* scene) {
    // Process all meshes at this node
    for (GLuint i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh);
    }
    
    // Process children
    for (GLuint i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene);
    }
}

template<int n>
void Imported<n>::processMesh(aiMesh* aiMesh) {
    Mesh mesh;
    
    // Load vertex positions
    for (GLuint i = 0; i < aiMesh->mNumVertices; ++i) {
        glm::vec<n,float> vertex;
        
        // Set all coordinates
        for (int j = 0; j < n; ++j) {
            vertex[j] = 0.0f;
        }
        
        // Copy position from Assimp
        vertex[0] = aiMesh->mVertices[i].x;
        vertex[1] = aiMesh->mVertices[i].y;
        vertex[2] = aiMesh->mVertices[i].z;
        if (n > 3) vertex[n-1] = 1.0f;  // Homogeneous coordinate
        
        mesh.vertices.push_back(vertex);
    }
    
    // Load normals
    for (GLuint i = 0; i < aiMesh->mNumVertices; ++i) {
        if (aiMesh->HasNormals()) {
            mesh.normals.push_back(glm::vec3(
                aiMesh->mNormals[i].x,
                aiMesh->mNormals[i].y,
                aiMesh->mNormals[i].z
            ));
        } else {
            mesh.normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
        }
    }
    
    // Load indices
    for (GLuint i = 0; i < aiMesh->mNumFaces; ++i) {
        aiFace face = aiMesh->mFaces[i];
        for (GLuint j = 0; j < face.mNumIndices; ++j) {
            mesh.indices.push_back(face.mIndices[j]);
        }
    }
    
    meshes.push_back(mesh);
}

template<int n>
Imported<n>& Imported<n>::operator*=(const glm::mat<n,n,float>& m) {
    // Transform all mesh vertices
    for (auto& mesh : meshes) {
        for (auto& vertex : mesh.vertices) {
            glm::vec<n,float> pos = vertex;
            if (n > 3) pos[n-1] = 1.0f;
            vertex = m * pos;
        }
    }
    this->updateGLBuffers();
    return *this;
}

template<int n>
Imported<n>* Imported<n>::operator*(const glm::mat<n,n,float>& m) const {
    Imported<n>* result = new Imported<n>(*this);
    *result *= m;
    return result;
}

template<int n>
float* Imported<n>::getPoints() const {
    int totalVerts = 0;
    for (const auto& mesh : meshes) {
        totalVerts += mesh.vertices.size();
    }
    
    if (totalVerts == 0) return nullptr;
    
    float* points = new float[totalVerts * n];
    int idx = 0;
    
    for (const auto& mesh : meshes) {
        for (const auto& vertex : mesh.vertices) {
            for (int i = 0; i < n; ++i) {
                points[idx++] = vertex[i];
            }
        }
    }
    
    return points;
}

template<int n>
int Imported<n>::getNumPoints() const {
    int total = 0;
    for (const auto& mesh : meshes) {
        total += mesh.vertices.size();
    }
    return total;
}

template<int n>
void Imported<n>::draw(bool wireframe) {
    if (this->VAO == 0) return;
    
    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glDrawElements(GL_TRIANGLES, this->vertexCount, GL_UNSIGNED_INT, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

template<int n>
void Imported<n>::createGLBuffers(GLenum usage) {
    // Gather all mesh data
    std::vector<glm::vec<n,float>> allVertices;
    std::vector<GLuint> allIndices;
    std::vector<glm::vec3> allNormals;
    
    GLuint vertexOffset = 0;
    for (const auto& mesh : meshes) {
        allVertices.insert(allVertices.end(), mesh.vertices.begin(), mesh.vertices.end());
        allNormals.insert(allNormals.end(), mesh.normals.begin(), mesh.normals.end());
        
        for (GLuint idx : mesh.indices) {
            allIndices.push_back(idx + vertexOffset);
        }
        vertexOffset += mesh.vertices.size();
    }
    
    this->vertexCount = allIndices.size();
    
    if (allVertices.empty()) return;
    
    // Create VAO
    glGenVertexArrays(1, &this->VAO);
    glBindVertexArray(this->VAO);
    
    // Create VBO for positions
    glGenBuffers(1, &this->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(glm::vec<n,float>), allVertices.data(), usage);
    
    // Position attribute
    glVertexAttribPointer(0, n, GL_FLOAT, GL_FALSE, sizeof(glm::vec<n,float>), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Create EBO for indices
    glGenBuffers(1, &this->EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, allIndices.size() * sizeof(GLuint), allIndices.data(), usage);
    
    // Create VBO for normals
    if (!allNormals.empty()) {
        glGenBuffers(1, &this->normVBO);
        glBindBuffer(GL_ARRAY_BUFFER, this->normVBO);
        glBufferData(GL_ARRAY_BUFFER, allNormals.size() * sizeof(glm::vec3), allNormals.data(), usage);
        
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(1);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

template<int n>
void Imported<n>::updateGLBuffers(GLenum usage) {
    // Regenerate GL buffers with updated mesh data
    deleteGLBuffers();
    createGLBuffers(usage);
}

template<int n>
GLenum Imported<n>::glDrawMode() const {
    return GL_TRIANGLES;
}

template<int n>
void Imported<n>::print() const {
    std::cout << "Imported Model: " << filepath << std::endl;
    std::cout << "Meshes: " << meshes.size() << std::endl;
    for (size_t i = 0; i < meshes.size(); ++i) {
        std::cout << "  Mesh " << i << ": " << meshes[i].vertices.size() << " vertices, "
                  << meshes[i].indices.size() << " indices" << std::endl;
    }
}

template<int n>
void Imported<n>::zoom(int percent) {
    float scale = 1.0f + (percent / 100.0f);
    glm::mat<n,n,float> scaleMat(1.0f);
    
    for (int i = 0; i < 3 && i < n; ++i) {
        scaleMat[i][i] = scale;
    }
    
    *this *= scaleMat;
}

template<int n>
void Imported<n>::rotate(int degrees) {
    // Rotate around Z axis
    float rad = degrees * 3.14159265358979323846f / 180.0f;
    glm::mat<n,n,float> rotMat(1.0f);
    
    float c = cosf(rad);
    float s = sinf(rad);
    
    rotMat[0][0] = c;
    rotMat[0][1] = -s;
    rotMat[1][0] = s;
    rotMat[1][1] = c;
    
    *this *= rotMat;
}

template<int n>
std::string Imported<n>::fprint() const {
    std::stringstream ss;
    ss << "Imported{" << filepath << ", meshes=" << meshes.size() << "}";
    return ss.str();
}
