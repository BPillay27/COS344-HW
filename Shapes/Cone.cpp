template<int n>
Cone<n>::Cone(){
    apex = glm::vec<n,float>();
    baseCenter = glm::vec<n,float>();
    radius = 1.0f;
    height = 1.0f;
    resolution = 32;
    angleOffset = 0.0f;
    axis = 2;
}

template<int n>
Cone<n>::Cone(const glm::vec<n,float>& baseCenter, float radius, float height, int resolution, int axis){
    this->baseCenter = baseCenter;
    this->radius = radius;
    this->height = height;
    this->resolution = (resolution > 2) ? resolution : 32;
    this->angleOffset = 0.0f;
    this->axis = axis;
    // compute apex by moving along axis
    this->apex = glm::vec<n,float>(baseCenter);
    this->apex[axis] = baseCenter[axis] + height;
}

template<int n>
Cone<n>::Cone(const glm::vec<n,float>& apex, const glm::vec<n,float>& baseCenter, float radius, int resolution){
    this->apex = apex;
    this->baseCenter = baseCenter;
    this->radius = radius;
    this->resolution = (resolution > 2) ? resolution : 32;
    this->angleOffset = 0.0f;
    // choose dominant axis between apex and baseCenter
    int best = 0; float bestv = fabsf(apex[0] - baseCenter[0]);
    for (int i = 1; i < 3 && i < n; ++i) {
        float v = fabsf(apex[i] - baseCenter[i]);
        if (v > bestv) { best = i; bestv = v; }
    }
    this->axis = best;
    this->height = fabsf(apex[this->axis] - baseCenter[this->axis]);
}

template<int n>
Cone<n>::Cone(const Cone<n> &two){
    this->apex = two.apex;
    this->baseCenter = two.baseCenter;
    this->radius = two.radius;
    this->height = two.height;
    this->resolution = two.resolution;
    this->angleOffset = two.angleOffset;
    this->axis = two.axis;
    {
        float* c = two.getColour();
        for (int i = 0; i < 4; ++i) this->colour[i] = c[i];
        delete[] c;
    }
}

template<int n>
Cone<n>& Cone<n>::operator*=(const glm::mat<n,n,float>& m){
    // compute new base center and apex
    glm::vec<n,float> ha = apex;
    glm::vec<n,float> hb = baseCenter;
    if (n >= 3) {
        ha[n-1] = 1.0f;   // Set homogeneous coordinate for apex
        hb[n-1] = 1.0f;   // Set homogeneous coordinate for baseCenter
    }
    glm::vec<n,float> newBaseMat = m * hb;
    glm::vec<n,float> newBase = newBaseMat;
    if (n >= 3) {
        newBase[n-1] = 0.0f;  // Reset homogeneous coordinate for direction
    }
    glm::vec<n,float> newApexMat = m * ha;
    glm::vec<n,float> newApex = newApexMat;
    if (n >= 3) {
        newApex[n-1] = 0.0f;  // Reset homogeneous coordinate for direction
    }

    // determine plane indices matching getPoints()
    int iA = 0, iB = 1;
    if (axis == 0) { iA = 1; iB = 2; }
    else if (axis == 1) { iA = 0; iB = 2; }
    else { iA = 0; iB = 1; }

    // sample offset on perimeter (radius along iA)
    glm::vec<n,float> offset; for (int i = 0; i < n; ++i) offset[i] = 0.0f;
    offset[iA] = radius;

    glm::vec<n,float> pMat = baseCenter + offset;
    if (n >= 3) {
        pMat[n-1] = 1.0f;  // Set homogeneous coordinate
    }
    glm::vec<n,float> newP = m * pMat;
    if (n >= 3) {
        newP[n-1] = 0.0f;  // Reset homogeneous coordinate for direction
    }
    glm::vec<n,float> transformedVec;
    for (int i = 0; i < n; ++i) transformedVec[i] = newP[i] - newBase[i];

    float delta = atan2f(transformedVec[iB], transformedVec[iA]);
    angleOffset += delta;

    apex = newApex;
    baseCenter = newBase;
    return *this;
}

template<int n>
Cone<n>* Cone<n>::operator*(const glm::mat<n,n,float>& m) const{
    Cone<n>* result = new Cone<n>(*this);
    *result *= m;
    return result;
}

template<int n>
float* Cone<n>::getPoints() const{
    // base fan: center + perimeter (resolution+1)
    int baseVerts = resolution + 2;
    // side fan: apex + perimeter (resolution+1)
    int sideVerts = resolution + 2;
    int totalVerts = baseVerts + sideVerts;
    int totalFloats = totalVerts * n;
    float* result = new float[totalFloats];
    int idx = 0;

    // compute positions along axis
    float basePos = baseCenter[axis];
    float apexPos = apex[axis];

    // plane indices
    int iA = 0, iB = 1;
    if (axis == 0) {
        iA = 1;
        iB = 2;
    }
    else if (axis == 1) {
        iA = 0; iB = 2; 
    }
    else {
        iA = 0;
        iB = 1;
    }

    // base center vertex
    for (int j = 0; j < n; ++j){
        if (j == axis) result[idx++] = basePos;
        else result[idx++] = baseCenter[j];
    }

    // base perimeter
    for (int k = 0; k <= resolution; ++k){
        float a = (2.0f * 3.14159265358979323846f * k) / resolution + angleOffset;
        float ca = cosf(a) * radius;
        float sa = sinf(a) * radius;
        for (int j = 0; j < n; ++j){
            if (j == iA) result[idx++] = baseCenter[iA] + ca;
            else if (j == iB) result[idx++] = baseCenter[iB] + sa;
            else if (j == axis) result[idx++] = basePos;
            else result[idx++] = baseCenter[j];
        }
    }

    // side fan: apex vertex first
    for (int j = 0; j < n; ++j){
        if (j == axis) result[idx++] = apexPos;
        else result[idx++] = apex[j];
    }

    // side perimeter (same perimeter points as base)
    for (int k = 0; k <= resolution; ++k){
        float a = (2.0f * 3.14159265358979323846f * k) / resolution + angleOffset;
        float ca = cosf(a) * radius;
        float sa = sinf(a) * radius;
        for (int j = 0; j < n; ++j){
            if (j == iA) result[idx++] = baseCenter[iA] + ca;
            else if (j == iB) result[idx++] = baseCenter[iB] + sa;
            else if (j == axis) result[idx++] = basePos;
            else result[idx++] = baseCenter[j];
        }
    }

    return result;
}

template<int n>
int Cone<n>::getNumPoints() const{
    int baseVerts = resolution + 2;
    int sideVerts = resolution + 2;
    int totalVerts = baseVerts + sideVerts;
    return totalVerts * n;
}

template<int n>
void Cone<n>::draw(bool wireframe){
    if (this->VAO == 0u) this->createGLBuffers();
    int baseVerts = resolution + 2;
    int sideVerts = resolution + 2;
    glBindVertexArray(this->VAO);
    glDisableVertexAttribArray(1);
    glVertexAttrib4f(1, this->colour[0], this->colour[1], this->colour[2], this->colour[3]);
    if (wireframe && this->EBO != 0u && this->lineIndexCount > 0) {
        glDrawElements(GL_LINES, this->lineIndexCount, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLE_FAN, 0, baseVerts);
        glDrawArrays(GL_TRIANGLE_FAN, baseVerts, sideVerts);
    }
    glBindVertexArray(0);
}

template<int n>
void Cone<n>::createGLBuffers(GLenum usage){
    Shape<n>::createGLBuffers(usage);
    int baseVerts = resolution + 2;
    int sideStart = baseVerts; // side apex then perimeter
    std::vector<GLuint> indices;
    // base fan triangles (center at 0)
    for (int i = 0; i <= resolution - 1; ++i){
        GLuint a = static_cast<GLuint>(0);
        GLuint b = static_cast<GLuint>(1 + i);
        GLuint c = static_cast<GLuint>(1 + ((i+1)%resolution));
        indices.push_back(a); indices.push_back(b);
        indices.push_back(b); indices.push_back(c);
        indices.push_back(c); indices.push_back(a);
    }
    // side fan triangles (apex at sideStart)
    for (int i = 0; i <= resolution - 1; ++i){
        GLuint a = static_cast<GLuint>(sideStart + 0);
        GLuint b = static_cast<GLuint>(sideStart + 1 + i);
        GLuint c = static_cast<GLuint>(sideStart + 1 + ((i+1)%resolution));
        indices.push_back(a); indices.push_back(b);
        indices.push_back(b); indices.push_back(c);
        indices.push_back(c); indices.push_back(a);
    }
    this->setLineIndices(indices);
}

template<int n>
GLenum Cone<n>::glDrawMode() const {
    return GL_TRIANGLE_FAN;
}

template<int n>
void Cone<n>::print() const{
    std::cout << "_ Apex _" << std::endl;
    apex.print();
    std::cout << "_ Base Centre _" << std::endl;
    baseCenter.print();
}

template<int n>
void Cone<n>::zoom(int percent){
    float factor = percent / 100.0f;
    // compute centroid between apex and baseCenter
    glm::vec<n,float> center;
    for (int i = 0; i < n; ++i) center[i] = (apex[i] + baseCenter[i]) / 2.0f;
    *this *= translation<n>(-center[0], -center[1], 0.0f);
    *this *= scaling<n>(factor);
    *this *= translation<n>(center[0], center[1], 0.0f);
}

template<int n>
void Cone<n>::rotate(int degrees){
    glm::vec<n,float> center;
    for (int i = 0; i < n; ++i) center[i] = (apex[i] + baseCenter[i]) / 2.0f;
    *this *= translation<n>(-center[0], -center[1], 0.0f);
    *this *= rotz<n>(degrees);
    *this *= translation<n>(center[0], center[1], 0.0f);
}

template<int n>
std::string Cone<n>::fprint() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6);
    for (int i = 0; i < n; ++i) ss << apex[i] << ' ';
    for (int i = 0; i < n; ++i) ss << baseCenter[i] << ' ';
    ss << radius << ' ' << height << ' ' << resolution << ' ' << angleOffset << ' ';
    int r = (int)roundf(this->colour[0]*255.0f);
    int g = (int)roundf(this->colour[1]*255.0f);
    int b = (int)roundf(this->colour[2]*255.0f);
    ss << r << ' ' << g << ' ' << b << ' ' << this->colour[3];
    return ss.str();
}
