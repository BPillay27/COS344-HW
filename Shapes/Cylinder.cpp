template<int n>
Cylinder<n>::Cylinder(const glm::vec<n,float>& center, float radius, float height, int resolution, int axis){
    this->center = center;
    this->radius = radius;
    this->height = height;
    if(resolution > 2 ) this->resolution = resolution;
    else this->resolution = 32;
    this->angleOffset = 0.0f;
    this->axis = axis;
    
    for (int i = 0; i < n; ++i) {
        this->basisU[i] = 0.0f;
        this->basisV[i] = 0.0f;
    }
    
    if (axis == 0) {
        this->basisU[1] = 1.0f;
        this->basisV[2] = 1.0f;
    } else if (axis == 1) {
        this->basisU[0] = 1.0f;
        this->basisV[2] = 1.0f;
    } else {
        this->basisU[0] = 1.0f;
        this->basisV[1] = 1.0f;
    }
}

template<int n>
Cylinder<n>::Cylinder(const glm::vec<n,float>& c1, const glm::vec<n,float>& c2, float radius, int resolution){
    this->center = glm::vec<n,float>();
    for (int i = 0; i < n; ++i) this->center[i] = (c1[i] + c2[i]) / 2.0f;
    
    glm::vec<n,float> axisDir;
    float distSq = 0.0f;
    for (int i = 0; i < n; ++i) {
        axisDir[i] = c2[i] - c1[i];
        distSq += axisDir[i] * axisDir[i];
    }
    this->height = sqrtf(distSq);
    
    float invLen = (distSq > 0) ? 1.0f / sqrtf(distSq) : 1.0f;
    for (int i = 0; i < n; ++i) axisDir[i] *= invLen;
    

    int best = 0; 
    float bestv = fabsf(c2[0] - c1[0]);
    for (int i = 1; i < n; ++i) {
        float v = fabsf(c2[i] - c1[i]);
        if (v > bestv) { best = i; bestv = v; }
    }
    this->axis = best;
    
    glm::vec<n,float> temp;
    for (int i = 0; i < n; ++i) temp[i] = 0.0f;
    if (fabsf(axisDir[0]) < 0.9f) {
        temp[0] = 1.0f;
    } else {
        temp[1] = 1.0f;
    }
    
    this->basisU = glm::vec<n,float>();
    for (int i = 0; i < n; ++i) this->basisU[i] = 0.0f;
    if (n >= 3) {
        this->basisU[0] = axisDir[1] * temp[2] - axisDir[2] * temp[1];
        this->basisU[1] = axisDir[2] * temp[0] - axisDir[0] * temp[2];
        this->basisU[2] = axisDir[0] * temp[1] - axisDir[1] * temp[0];
    }
    float ulenSq = 0.0f;
    for (int i = 0; i < n; ++i) ulenSq += this->basisU[i] * this->basisU[i];
    float uinvLen = (ulenSq > 0) ? 1.0f / sqrtf(ulenSq) : 1.0f;
    for (int i = 0; i < n; ++i) this->basisU[i] *= uinvLen;
    
    this->basisV = glm::vec<n,float>();
    for (int i = 0; i < n; ++i) this->basisV[i] = 0.0f;
    if (n >= 3) {
        this->basisV[0] = axisDir[1] * this->basisU[2] - axisDir[2] * this->basisU[1];
        this->basisV[1] = axisDir[2] * this->basisU[0] - axisDir[0] * this->basisU[2];
        this->basisV[2] = axisDir[0] * this->basisU[1] - axisDir[1] * this->basisU[0];
    }
    
    this->radius = radius;
    if(resolution > 2 ) this->resolution = resolution;
    else this->resolution = 32;
    this->angleOffset = 0.0f;
}

template<int n>
Cylinder<n>::Cylinder(){
    center = glm::vec<n,float>();
    radius = 1.0f;
    height = 1.0f;
    resolution = 32;
    angleOffset = 0.0f;
    this->axis = 2;
    for (int i = 0; i < n; ++i) {
        this->basisU[i] = 0.0f;
        this->basisV[i] = 0.0f;
    }
    this->basisU[0] = 1.0f;
    this->basisV[1] = 1.0f;
}

template<int n>
Cylinder<n>::Cylinder(const Cylinder<n> &two) : Shape<n>(two) {
    this->center=two.center;
    this->radius=two.radius;
    this->height=two.height;
    this->resolution=two.resolution;
    this->angleOffset = two.angleOffset;
    this->axis = two.axis;
    this->basisU = two.basisU;
    this->basisV = two.basisV;
}

template<int n>
Cylinder<n>& Cylinder<n>::operator*=(const glm::mat<n,n,float>& m){
    glm::vec<n,float> hc = center;
    if (n >= 3) {
        hc[n-1] = 1.0f;
    }
    glm::vec<n,float> newCenterMat = m * hc;
    glm::vec<n,float> newCenter = newCenterMat;
    if (n >= 3) {
        newCenter[n-1] = 0.0f;
    }

    glm::vec<n,float> basisUMat = this->basisU;
    glm::vec<n,float> basisVMat = this->basisV;
    glm::vec<n,float> newBasisU = m * basisUMat;
    glm::vec<n,float> newBasisV = m * basisVMat;
    

    float uLen = 0.0f, vLen = 0.0f;
    for (int i = 0; i < n; ++i) {
        uLen += newBasisU[i] * newBasisU[i];
        vLen += newBasisV[i] * newBasisV[i];
    }
    uLen = sqrtf(uLen);
    vLen = sqrtf(vLen);
    if (uLen > 0.0f) for (int i = 0; i < n; ++i) newBasisU[i] /= uLen;
    if (vLen > 0.0f) for (int i = 0; i < n; ++i) newBasisV[i] /= vLen;
    
    this->basisU = newBasisU;
    this->basisV = newBasisV;

    int iA = 0, iB = 1;
    if (this->axis == 0) { iA = 1; iB = 2; }
    else if (this->axis == 1) { iA = 0; iB = 2; }
    else { iA = 0; iB = 1; }

    glm::vec<n,float> offset;
    for (int i = 0; i < n; ++i) offset[i] = 0.0f;
    offset[iA] = radius;

    glm::vec<n,float> pMat = center + offset;
    if (n >= 3) {
        pMat[n-1] = 1.0f;
    }
    glm::vec<n,float> newP = m * pMat;
    if (n >= 3) {
        newP[n-1] = 0.0f;
    }
    glm::vec<n,float> transformedVec;
    for (int i = 0; i < n; ++i) transformedVec[i] = newP[i] - newCenter[i];

    float delta = 0.0f;
    if (n > 1) delta = atan2f(transformedVec[iB], transformedVec[iA]);
    angleOffset += delta;

    center = newCenter;
    return *this;
}

template<int n>
Cylinder<n>* Cylinder<n>::operator*(const glm::mat<n,n,float>& m) const{
    Cylinder<n>* result=new Cylinder<n>(*this);
    *result*=m;
    return result;
}

template<int n>
float* Cylinder<n>::getPoints() const{
    int capVerts = 1 + (resolution + 1);
    int sideVerts = 2 * (resolution + 1);
    int totalVerts = capVerts + capVerts + sideVerts;
    int totalFloats = totalVerts * n;
    float* result = new float[totalFloats];
    
    glm::vec<n,float> axisDir;
    for (int i = 0; i < n; ++i) axisDir[i] = 0.0f;
    if (n >= 3) {
        axisDir[0] = this->basisU[1] * this->basisV[2] - this->basisU[2] * this->basisV[1];
        axisDir[1] = this->basisU[2] * this->basisV[0] - this->basisU[0] * this->basisV[2];
        axisDir[2] = this->basisU[0] * this->basisV[1] - this->basisU[1] * this->basisV[0];
    }
    
    glm::vec<n,float> topCenter, bottomCenter;
    for (int i = 0; i < n; ++i) {
        topCenter[i] = center[i] + axisDir[i] * height / 2.0f;
        bottomCenter[i] = center[i] - axisDir[i] * height / 2.0f;
    }
    
    int idx = 0;

    auto writePosition = [&](const glm::vec<n,float>& posVec){
        result[idx++] = posVec[0];
        result[idx++] = posVec[1];
        result[idx++] = posVec[2];
        if (n > 3) result[idx++] = 1.0f;
    };

    glm::vec<n,float> topNormal = axisDir;
    writePosition(topCenter);

    for (int k = 0; k <= resolution; ++k) {
        float a = (2.0f * 3.14159265358979323846f * k) / resolution + angleOffset;
        float ca = cosf(a);
        float sa = sinf(a);
        glm::vec<n,float> p;
        for (int j = 0; j < n; ++j) {
            float uComp = this->basisU[j] * ca;
            float vComp = this->basisV[j] * sa;
            p[j] = topCenter[j] + radius * (uComp + vComp);
        }
        writePosition(p);
    }

    glm::vec<n,float> bottomNormal;
    for (int i = 0; i < n; ++i) bottomNormal[i] = -axisDir[i];
    writePosition(bottomCenter);

    for (int k = resolution; k >= 0; --k) {
        float a = (2.0f * 3.14159265358979323846f * k) / resolution + angleOffset;
        float ca = cosf(a);
        float sa = sinf(a);
        glm::vec<n,float> p;
        for (int j = 0; j < n; ++j) {
            float uComp = this->basisU[j] * ca;
            float vComp = this->basisV[j] * sa;
            p[j] = bottomCenter[j] + radius * (uComp + vComp);
        }
        writePosition(p);
    }

    for (int k = 0; k <= resolution; ++k) {
        float a = (2.0f * 3.14159265358979323846f * k) / resolution + angleOffset;
        float ca = cosf(a);
        float sa = sinf(a);
        glm::vec<n,float> pt, pb, nrm;
        for (int j = 0; j < n; ++j) {
            float uComp = this->basisU[j] * ca;
            float vComp = this->basisV[j] * sa;
            pt[j] = topCenter[j] + radius * (uComp + vComp);
            pb[j] = bottomCenter[j] + radius * (uComp + vComp);
        }
        for (int j = 0; j < 3; ++j) nrm[j] = pt[j] - ((topCenter[j] + bottomCenter[j]) * 0.5f);
        writePosition(pt);
        writePosition(pb);
    }

    return result;
}

template<int n>
int Cylinder<n>::getNumPoints() const{
    int capVerts = 1 + (resolution + 1);
    int sideVerts = 2 * (resolution + 1);
    int totalVerts = capVerts + capVerts + sideVerts;
    return totalVerts * n;
}

template<int n>
float* Cylinder<n>::getTexCoords() const {
    int capVerts = 1 + (resolution + 1);
    int sideVerts = 2 * (resolution + 1);
    int totalVerts = capVerts + capVerts + sideVerts;
    float* result = new float[totalVerts * 2];
    int idx = 0;

    result[idx++] = 0.5f; result[idx++] = 1.0f;
    for (int k = 0; k <= resolution; ++k) {
        float u = (float)k / (float)resolution;
        result[idx++] = u; result[idx++] = 1.0f;
    }
    result[idx++] = 0.5f; result[idx++] = 0.0f;
    for (int k = resolution; k >= 0; --k) {
        float u = (float)k / (float)resolution;
        result[idx++] = u; result[idx++] = 0.0f;
    }
    for (int k = 0; k <= resolution; ++k) {
        float u = (float)k / (float)resolution;
        result[idx++] = u; result[idx++] = 1.0f;
        result[idx++] = u; result[idx++] = 0.0f;
    }

    return result;
}

template<int n>
int Cylinder<n>::getNumTexCoords() const {
    int capVerts = 1 + (resolution + 1);
    int sideVerts = 2 * (resolution + 1);
    int totalVerts = capVerts + capVerts + sideVerts;
    return totalVerts * 2;
}

template<int n>
float* Cylinder<n>::getNormals() const {
    int capVerts = 1 + (resolution + 1);
    int sideVerts = 2 * (resolution + 1);
    int totalVerts = capVerts + capVerts + sideVerts;
    float* result = new float[totalVerts * 3];
    int idx = 0;

    glm::vec<n,float> axisDir;
    for (int i = 0; i < n; ++i) axisDir[i] = 0.0f;
    if (n >= 3) {
        axisDir[0] = this->basisU[1] * this->basisV[2] - this->basisU[2] * this->basisV[1];
        axisDir[1] = this->basisU[2] * this->basisV[0] - this->basisU[0] * this->basisV[2];
        axisDir[2] = this->basisU[0] * this->basisV[1] - this->basisU[1] * this->basisV[0];
    }
    glm::vec<n,float> topCenter, bottomCenter;
    for (int i = 0; i < n; ++i) {
        topCenter[i] = center[i] + axisDir[i] * height / 2.0f;
        bottomCenter[i] = center[i] - axisDir[i] * height / 2.0f;
    }

    float nx = axisDir[0], ny = axisDir[1], nz = axisDir[2];
    float len = sqrtf(nx*nx + ny*ny + nz*nz);
    if (len>0.0f) { nx/=len; ny/=len; nz/=len; }
    result[idx++] = nx; result[idx++] = ny; result[idx++] = nz;

    for (int k = 0; k <= resolution; ++k) {
        result[idx++] = nx; result[idx++] = ny; result[idx++] = nz;
    }

    result[idx++] = -nx; result[idx++] = -ny; result[idx++] = -nz;

    for (int k = resolution; k >= 0; --k) {
        result[idx++] = -nx; result[idx++] = -ny; result[idx++] = -nz;
    }

    for (int k = 0; k <= resolution; ++k) {
        float a = (2.0f * 3.14159265358979323846f * k) / resolution + angleOffset;
        float ca = cosf(a);
        float sa = sinf(a);
        glm::vec<n,float> pt;
        for (int j = 0; j < n; ++j) {
            float uComp = this->basisU[j] * ca;
            float vComp = this->basisV[j] * sa;
            pt[j] = topCenter[j] + radius * (uComp + vComp);
        }
        float cx = (topCenter[0] + bottomCenter[0]) * 0.5f;
        float cy = (topCenter[1] + bottomCenter[1]) * 0.5f;
        float cz = (topCenter[2] + bottomCenter[2]) * 0.5f;
        float rx = pt[0] - cx;
        float ry = pt[1] - cy;
        float rz = pt[2] - cz;
        float rlen = sqrtf(rx*rx + ry*ry + rz*rz);
        if (rlen > 0.0f) { rx/=rlen; ry/=rlen; rz/=rlen; }
        result[idx++] = rx; result[idx++] = ry; result[idx++] = rz;
        result[idx++] = rx; result[idx++] = ry; result[idx++] = rz;
    }

    return result;
}

template<int n>
int Cylinder<n>::getNumNormals() const {
    int capVerts = 1 + (resolution + 1);
    int sideVerts = 2 * (resolution + 1);
    int totalVerts = capVerts + capVerts + sideVerts;
    return totalVerts * 3;
}

template<int n>
glm::vec<n,float> Cylinder<n>::normalAtAngle(float angle) const {
    glm::vec<n,float> out;
    float ca = cosf(angle);
    float sa = sinf(angle);
    for (int i = 0; i < n; ++i) out[i] = 0.0f;
    for (int i = 0; i < n; ++i) out[i] = this->basisU[i] * ca + this->basisV[i] * sa;
    float len = 0.0f;
    for (int i = 0; i < 3; ++i) len += out[i]*out[i];
    len = sqrtf(len);
    if (len > 0.0f) for (int i = 0; i < 3; ++i) out[i] /= len;
    if (n > 3) out[3] = 0.0f;
    return out;
}

template<int n>
glm::vec<n,float> Cylinder<n>::normalAtPoint(const glm::vec<n,float>& p) const {
    glm::vec<n,float> axisDir;
    for (int i = 0; i < n; ++i) axisDir[i] = 0.0f;
    if (n >= 3) {
        axisDir[0] = this->basisU[1] * this->basisV[2] - this->basisU[2] * this->basisV[1];
        axisDir[1] = this->basisU[2] * this->basisV[0] - this->basisU[0] * this->basisV[2];
        axisDir[2] = this->basisU[0] * this->basisV[1] - this->basisU[1] * this->basisV[0];
    }
    glm::vec<n,float> cLineCenter;
    for (int i = 0; i < n; ++i) cLineCenter[i] = center[i];
    glm::vec<n,float> v;
    for (int i = 0; i < n; ++i) v[i] = p[i] - center[i];
    float dot = 0.0f;
    for (int i = 0; i < 3; ++i) dot += v[i] * axisDir[i];
    glm::vec<n,float> radial;
    for (int i = 0; i < 3; ++i) radial[i] = v[i] - dot * axisDir[i];
    float rlen = 0.0f;
    for (int i = 0; i < 3; ++i) rlen += radial[i]*radial[i];
    rlen = sqrtf(rlen);
    glm::vec<n,float> out;
    for (int i = 0; i < 3; ++i) out[i] = (rlen > 0.0f) ? radial[i] / rlen : 0.0f;
    if (n > 3) out[3] = 0.0f;
    return out;
}

template<int n>
void Cylinder<n>::draw(){
    if (this->VAO == 0u) this->createGLBuffers();

    int capVerts = 1 + (resolution + 1);
    int sideVerts = 2 * (resolution + 1);
    
    glBindVertexArray(this->VAO);
    // Ensure shader has the base colour uniform set for filled fragments
    GLint currentProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    if (currentProgram != 0) {
        GLint loc = glGetUniformLocation((GLuint)currentProgram, "uBaseColor");
        if (loc != -1) {
            float baseCol[4] = { this->colour[0], this->colour[1], this->colour[2], this->colour[3] };
            glUniform4fv(loc, 1, baseCol);
        }
    }
    
    // FIX: Changed from attribute 1 (aTexCoords) to attribute 3 (aColor).
    glDisableVertexAttribArray(3);
    glVertexAttrib4f(3, this->colour[0], this->colour[1], this->colour[2], this->colour[3]);
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, capVerts);
    glDrawArrays(GL_TRIANGLE_FAN, capVerts, capVerts);
    glDrawArrays(GL_TRIANGLE_STRIP, 2 * capVerts, sideVerts);
    
    glBindVertexArray(0);
}

template<int n>
void Cylinder<n>::createGLBuffers(GLenum usage){
    Shape<n>::createGLBuffers(usage);
}

template<int n>
GLenum Cylinder<n>::glDrawMode() const {
    return GL_TRIANGLE_STRIP;
}

template<int n>
void Cylinder<n>::print() const{
    std::cout << "_ Center _ " << std::endl;
    std::cout << "Center: (" << center[0];
    for(int i = 1; i < n; i++) std::cout << ", " << center[i];
    std::cout << ")" << std::endl;
    std::cout << "_ Radius _ " << std::endl;
    std::cout << radius << std::endl;
    std::cout << "_ Height _ " << std::endl;
    std::cout << height << std::endl;
}

template<int n>
void Cylinder<n>::zoom(int percent){
    float factor = percent / 100.0f;
    radius *= factor;
    height *= factor;
}

template<int n>
void Cylinder<n>::rotate(int degrees){
    float delta = (float)degrees * (float)3.14159265358979323846f / 180.0f;
    angleOffset += delta;
    if (angleOffset > 2.0f * 3.14159265358979323846f || angleOffset < -2.0f * 3.14159265358979323846f) {
        angleOffset = fmodf(angleOffset, 2.0f * 3.14159265358979323846f);
    }
}

template<int n>
std::string Cylinder<n>::fprint() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6);
    for (int i = 0; i < n; ++i) ss << center[i] << ' ';
    ss << radius << ' ' << height << ' ' << resolution << ' ' << angleOffset << ' ';
    int r = (int)roundf(this->colour[0]*255.0f);
    int g = (int)roundf(this->colour[1]*255.0f);
    int b = (int)roundf(this->colour[2]*255.0f);
    ss << r << ' ' << g << ' ' << b << ' ' << this->colour[3];
    return ss.str();
}

template<int n>
void Cylinder<n>::setResolution(int r){
    if (r > 2) this->resolution = r;
    else this->resolution = 3;
    // Resolution change affects vertex ordering and line indices; fully rebuild GL buffers
    if (this->VAO != 0u) {
        this->deleteGLBuffers();
        this->createGLBuffers();
    }
}

template<int n>
int Cylinder<n>::getResolution() const { return this->resolution; }

