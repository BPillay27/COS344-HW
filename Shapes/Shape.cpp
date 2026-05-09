template <int n>
void Shape<n>::setColour(int r, int g, int b, float a){
    colour[0]=r/255.0f;
    colour[1]=g/255.0f;
    colour[2]=b/255.0f;
    colour[3]=a;
}

template <int n>
float* Shape<n>::getColour() const{
    float* result = new float[4];
    result[0] = colour[0];
    result[1] = colour[1];
    result[2] = colour[2];
    result[3] = colour[3];
    return result;
}

template <int n>
Shape<n>::Shape(const Shape& other) {
    for (int i = 0; i < 4; ++i) this->colour[i] = other.colour[i];

    VAO = 0u; VBO = 0u; EBO = 0u;
    vertexCount = other.vertexCount;
    lineIndexCount = 0;
}

template <int n>
Shape<n>& Shape<n>::operator=(const Shape& other) {
    if (this != &other) {

        deleteGLBuffers();
        for (int i = 0; i < 4; ++i) this->colour[i] = other.colour[i];
        VAO = 0u; VBO = 0u; EBO = 0u;
        vertexCount = other.vertexCount;
        lineIndexCount = 0;
    }
    return *this;
}

template <int n>
Shape<n>::Shape(Shape&& other) noexcept {
    for (int i = 0; i < 4; ++i) this->colour[i] = other.colour[i];
    // steal GL handles
    VAO = other.VAO; VBO = other.VBO; EBO = other.EBO;
    vertexCount = other.vertexCount;
    lineIndexCount = other.lineIndexCount;
    other.VAO = other.VBO = other.EBO = 0u;
    other.vertexCount = 0;
    other.lineIndexCount = 0;
}

template <int n>
Shape<n>& Shape<n>::operator=(Shape&& other) noexcept {
    if (this != &other) {
        deleteGLBuffers();
        for (int i = 0; i < 4; ++i) this->colour[i] = other.colour[i];
        VAO = other.VAO; VBO = other.VBO; EBO = other.EBO;
        vertexCount = other.vertexCount;
        lineIndexCount = other.lineIndexCount;
        other.VAO = other.VBO = other.EBO = 0u;
        other.vertexCount = 0;
        other.lineIndexCount = 0;
    }
    return *this;
}

template <int n>
void Shape<n>::createGLBuffers(GLenum usage){
    if (VAO == 0u) glGenVertexArrays(1, &VAO);
    if (VBO == 0u) glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    float* pts = getPoints();
    int numFloats = getNumPoints();
    // Validate vertex data for NaN/Inf to catch corrupt transforms early
    for (int i = 0; i < numFloats; ++i) {
        if (!std::isfinite(pts[i])) {
            std::cerr << "Shape::createGLBuffers(): invalid vertex data (NaN/Inf) at index " << i << "\n";
            delete[] pts;
            return;
        }
    }
    int verts = 0;
    if (n > 0) verts = numFloats / n;
    glBufferData(GL_ARRAY_BUFFER, numFloats * sizeof(float), pts, usage);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, n * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Optional: upload separate texcoord VBO if provided by subclass
    float* tex = getTexCoords();
    int numTex = getNumTexCoords();
    // Validate texcoord count matches expected vertex count (2 components per vertex)
    if (numTex > 0) {
        int expectedTex = verts * 2;
        if (numTex != expectedTex) {
            std::cerr << "Shape::createGLBuffers(): texcoord count mismatch: got " << numTex
                      << " expected " << expectedTex << " (verts=" << verts << ")\n";
        }
    }
    if (tex != nullptr && numTex > 0) {
        if (texVBO == 0u) glGenBuffers(1, &texVBO);
        glBindBuffer(GL_ARRAY_BUFFER, texVBO);
        glBufferData(GL_ARRAY_BUFFER, numTex * sizeof(float), tex, usage);
        // attribute location 1: vec2 texcoord
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        delete[] tex;
    }

    float* norms = getNormals();
    int numNorms = getNumNormals();
    // Validate normal count matches expected vertex count (3 components per vertex)
    if (numNorms > 0) {
        int expectedNorms = verts * 3;
        if (numNorms != expectedNorms) {
            std::cerr << "Shape::createGLBuffers(): normal count mismatch: got " << numNorms
                      << " expected " << expectedNorms << " (verts=" << verts << ")\n";
        }
    }
    if (norms != nullptr && numNorms > 0) {
        if (normVBO == 0u) glGenBuffers(1, &normVBO);
        glBindBuffer(GL_ARRAY_BUFFER, normVBO);
        glBufferData(GL_ARRAY_BUFFER, numNorms * sizeof(float), norms, usage);
        // attribute location 2: vec3 normal
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        delete[] norms;
    }

    delete[] pts;
    vertexCount = verts;

    // If no per-vertex colors were uploaded, provide a constant attribute3 with the shape colour
    if (colorVBO == 0u) {
        // attribute location 3: vec3 color (constant)
        glDisableVertexAttribArray(3);
        glVertexAttrib3f(3, colour[0], colour[1], colour[2]);
    }

    glBindVertexArray(0);
}

template <int n>
void Shape<n>::updateGLBuffers(GLenum usage){
    if (VBO == 0u) return;
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    float* pts = getPoints();
    int numFloats = getNumPoints();
    // Validate vertex data before uploading
    for (int i = 0; i < numFloats; ++i) {
        if (!std::isfinite(pts[i])) {
            std::cerr << "Shape::updateGLBuffers(): invalid vertex data (NaN/Inf) at index " << i << "\n";
            delete[] pts;
            return;
        }
    }
    glBufferData(GL_ARRAY_BUFFER, numFloats * sizeof(float), nullptr, usage);
    glBufferSubData(GL_ARRAY_BUFFER, 0, numFloats * sizeof(float), pts);
    delete[] pts;

    // Update texcoords VBO if present or provided
    float* tex = getTexCoords();
    int numTex = getNumTexCoords();
    if (numTex > 0) {
        int verts = numFloats / n;
        int expectedTex = verts * 2;
        if (numTex != expectedTex) {
            std::cerr << "Shape::updateGLBuffers(): texcoord count mismatch: got " << numTex
                      << " expected " << expectedTex << " (verts=" << verts << ")\n";
        }
    }
    if (tex != nullptr && numTex > 0) {
        if (texVBO == 0u) glGenBuffers(1, &texVBO);
        glBindBuffer(GL_ARRAY_BUFFER, texVBO);
        glBufferData(GL_ARRAY_BUFFER, numTex * sizeof(float), nullptr, usage);
        glBufferSubData(GL_ARRAY_BUFFER, 0, numTex * sizeof(float), tex);
        delete[] tex;
    }

    // Update normals VBO if present or provided
    float* norms = getNormals();
    int numNorms = getNumNormals();
    if (numNorms > 0) {
        int verts = numFloats / n;
        int expectedNorms = verts * 3;
        if (numNorms != expectedNorms) {
            std::cerr << "Shape::updateGLBuffers(): normal count mismatch: got " << numNorms
                      << " expected " << expectedNorms << " (verts=" << verts << ")\n";
        }
    }
    if (norms != nullptr && numNorms > 0) {
        if (normVBO == 0u) glGenBuffers(1, &normVBO);
        glBindBuffer(GL_ARRAY_BUFFER, normVBO);
        glBufferData(GL_ARRAY_BUFFER, numNorms * sizeof(float), nullptr, usage);
        glBufferSubData(GL_ARRAY_BUFFER, 0, numNorms * sizeof(float), norms);
        delete[] norms;
    }
}

template <int n>
void Shape<n>::deleteGLBuffers(){
    if (EBO != 0u) { glDeleteBuffers(1, &EBO); EBO = 0u; }
    if (VBO != 0u) { glDeleteBuffers(1, &VBO); VBO = 0u; }
    if (texVBO != 0u) { glDeleteBuffers(1, &texVBO); texVBO = 0u; }
    if (normVBO != 0u) { glDeleteBuffers(1, &normVBO); normVBO = 0u; }
    if (colorVBO != 0u) { glDeleteBuffers(1, &colorVBO); colorVBO = 0u; }
    if (VAO != 0u) { glDeleteVertexArrays(1, &VAO); VAO = 0u; }
    vertexCount = 0;
}

template <int n>
void Shape<n>::setPerVertexColors(const float* colors, int numFloats, GLenum usage) {
    if (VAO == 0u) return; // must have VAO created first
    int verts = vertexCount;
    if (verts == 0) return;
    int expected = verts * 3;
    if (numFloats != expected) {
        std::cerr << "Shape::setPerVertexColors(): color count mismatch: got " << numFloats
                  << " expected " << expected << "\n";
        return;
    }

    if (colorVBO == 0u) glGenBuffers(1, &colorVBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, numFloats * sizeof(float), colors, usage);
    // attribute location 3: vec3 color
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

template <int n>
void Shape<n>::setLineIndices(const std::vector<GLuint>& indices) {
    if (VAO == 0u) return;
    
    if (EBO == 0u) glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    
    lineIndexCount =indices.size();
    glBindVertexArray(0);
}

template <int n>
void Shape<n>::draw(bool wireframe){
    if (VAO == 0u){
        return;
    }
    // Minimal draw path: bind VAO and issue draw call. Avoid expensive queries/logging.
    glBindVertexArray(VAO);
    if (wireframe && EBO != 0u && lineIndexCount > 0) {
        glDrawElements(GL_LINES, lineIndexCount, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(glDrawMode(), 0, vertexCount);
    }
    glBindVertexArray(0);
}

template <int n>
Shape<n>::~Shape() {
    deleteGLBuffers();
}

template <int n>
float* Shape<n>::getTexCoords() const { return nullptr; }

template <int n>
int Shape<n>::getNumTexCoords() const { return 0; }

template <int n>
float* Shape<n>::getNormals() const { return nullptr; }

template <int n>
int Shape<n>::getNumNormals() const { return 0; }




