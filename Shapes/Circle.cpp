template<int n>
Circle<n>::Circle(const glm::vec<n,float>& center, float radius, int resolution){
    this->center = center;
    this->radius = radius;
    if(resolution > 2 ){
        this->resolution = resolution;
    }else{
        this->resolution = 32;
    }
    this->angleOffset = 0.0f;
}

template<int n>
Circle<n>::Circle(){
    center = glm::vec<n,float>();
    radius = 1.0f;
    resolution = 32;
    angleOffset = 0.0f;
}

template <int n>
Circle<n>::Circle(const Circle<n> &two){
    this->center=two.center;
    this->radius=two.radius;
    this->resolution=two.resolution;
    {
        float* c = two.getColour();
        for (int i = 0; i < 4; ++i) this->colour[i] = c[i];
        delete[] c;
    }
    this->angleOffset = two.angleOffset;
}

template<int n>
Circle<n>& Circle<n>::operator*=(const glm::mat<n,n,float>& m){
    // Transform center
    glm::vec<n,float> hc = center;
    if (n >= 3) {
        hc[n-1] = 1.0f;  // Set homogeneous coordinate
    }
    glm::vec<n,float> newCenter = m * hc;
    if (n >= 3) {
        newCenter[n-1] = 0.0f;  // Reset homogeneous coordinate for direction
    }
    
    // Compute angle delta by transforming a radius offset point
    int iA = 0, iB = 1;  // default to XY plane
    if (n == 3) {
        iA = 0; iB = 1;  // Circle is in XY plane (Z is constant)
    } else if (n == 4) {
        iA = 0; iB = 1;  // Circle is in XY plane for 4D
    }
    
    // Create offset point at radius distance along iA axis
    glm::vec<n,float> offset;
    for (int i = 0; i < n; ++i) offset[i] = 0.0f;
    offset[iA] = radius;
    
    // Transform the offset point
    glm::vec<n,float> pMat = center + offset;
    if (n >= 3) {
        pMat[n-1] = 1.0f;  // Set homogeneous coordinate
    }
    glm::vec<n,float> newP = m * pMat;
    if (n >= 3) {
        newP[n-1] = 0.0f;  // Reset homogeneous coordinate for direction
    }
    
    // Compute the transformed offset vector
    glm::vec<n,float> transformedVec;
    for (int i = 0; i < n; ++i) transformedVec[i] = newP[i] - newCenter[i];
    
    // Compute angle delta
    float delta = atan2f(transformedVec[iB], transformedVec[iA]);
    angleOffset += delta;
    
    center = newCenter;
    return *this;
}

template<int n>
Circle<n>* Circle<n>::operator*(const glm::mat<n,n,float>& m) const{
    Circle<n>* result=new Circle<n>(*this);
    *result*=m;
    return result;
}

template<int n>
float* Circle<n>::getPoints() const{
    int verts = resolution + 2;
    int totalFloats = verts * n;
    float* result = new float[totalFloats];

    // center vertex
    for (int i = 0; i < n; ++i) {
        result[i] = center[i];
    }

    for (int k = 0; k <= resolution; ++k) {
        float a = (2.0f * M_PI * k) / resolution + angleOffset;
        int vertIndex = k + 1; // +1 to skip center
        int base = vertIndex * n;
        if (n >= 2) {
            result[base + 0] = center[0] + cosf(a) * radius;
            result[base + 1] = center[1] + sinf(a) * radius;
        }
        for (int j = 2; j < n; ++j) {
            result[base + j] = center[j];
        }
    }

    return result;
}

template<int n>
int Circle<n>::getNumPoints() const{
    return (resolution + 2) * n;
}

template<int n>
void Circle<n>::print() const{
    std::cout << "_ Center _ " << std::endl;
    center.print();
    std::cout << "_ Radius _ " << std::endl;
    std::cout << radius << std::endl;
}

template<int n>
void Circle<n>::zoom(int percent){
    float factor = percent / 100.0f;
    radius *= factor;
}

template<int n>
void Circle<n>::rotate(int degrees){
    // Advance the sampling offset so the polygon fan appears rotated.
    float delta = (float)degrees * (float)M_PI / 180.0f;
    angleOffset += delta;
    // keep angleOffset in reasonable range
    if (angleOffset > 2.0f * M_PI || angleOffset < -2.0f * M_PI) {
        angleOffset = fmodf(angleOffset, 2.0f * M_PI);
    }
    //std::cout << "Circle::rotate called: degrees=" << degrees << ", angleOffset=" << angleOffset << std::endl;
}


template<int n>
std::string Circle<n>::fprint() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6);
    // center
    for (int i = 0; i < n; ++i) ss << center[i] << ' ';
    ss << radius << ' ' << resolution << ' ' << angleOffset << ' ';
    int r = (int)roundf(this->colour[0]*255.0f);
    int g = (int)roundf(this->colour[1]*255.0f);
    int b = (int)roundf(this->colour[2]*255.0f);
    ss << r << ' ' << g << ' ' << b << ' ' << this->colour[3];
    return ss.str();
}

template<int n>
GLenum Circle<n>::glDrawMode() const {
    return GL_TRIANGLE_FAN;
}

template<int n>
void Circle<n>::createGLBuffers(GLenum usage){
    Shape<n>::createGLBuffers(usage);
    
    // Create wireframe indices: radii + perimeter
    std::vector<GLuint> indices;
    
    // Radii: center (index 0) to each perimeter point (1..resolution)
    for (int k = 1; k <= resolution; ++k) {
        indices.push_back(0);
        indices.push_back((GLuint)k);
    }
    
    // Perimeter edges: connect consecutive perimeter points
    for (int k = 1; k < resolution; ++k) {
        indices.push_back((GLuint)k);
        indices.push_back((GLuint)(k + 1));
    }
    
    // Closing edge: last to first perimeter point
    indices.push_back((GLuint)resolution);
    indices.push_back(1u);
    
    this->setLineIndices(indices);
}

template<int n>
void Circle<n>::draw(bool wireframe){
    if (this->VAO == 0u) this->createGLBuffers();
    
    glBindVertexArray(this->VAO);
    glDisableVertexAttribArray(1);
    glVertexAttrib4f(1, this->colour[0], this->colour[1], this->colour[2], this->colour[3]);
    
    if (wireframe && this->EBO != 0u && this->lineIndexCount > 0) {
        glDrawElements(GL_LINES, this->lineIndexCount, GL_UNSIGNED_INT, 0);
    } else {
        int numVerts = getNumPoints() / n;
        glDrawArrays(GL_TRIANGLE_FAN, 0, numVerts);
    }
    
    glBindVertexArray(0);
}

