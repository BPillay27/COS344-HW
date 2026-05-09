
template<int n>
Sphere<n>::Sphere(){
    center = glm::vec<n,float>();
    radius = 1.0f;
    stacks = 16;
    slices = 32;
}

template<int n>
Sphere<n>::Sphere(const glm::vec<n,float>& c, float r, int s, int l){
    center = c;
    radius = r;
    stacks = (s>2)?s:16;
    slices = (l>2)?l:32;
}

template<int n>
Sphere<n>::Sphere(const Sphere<n>& other){
    this->center = other.center;
    this->radius = other.radius;
    this->stacks = other.stacks;
    this->slices = other.slices;
    {
        float* c = other.getColour();
        for (int i = 0; i < 4; ++i) this->colour[i] = c[i];
        delete[] c;
    }
}

template<int n>
Sphere<n>& Sphere<n>::operator*=(const glm::mat<n,n,float>& m){
    glm::vec<n,float> hc = center;
    if (n >= 3) {
        hc[n-1] = 1.0f;  // Set homogeneous coordinate
    }
    glm::vec<n,float> newCenter = m * hc;
    if (n >= 3) {
        newCenter[n-1] = 0.0f;  // Reset homogeneous coordinate for direction
    }
    center = newCenter;
    return *this;
}

template<int n>
Sphere<n>* Sphere<n>::operator*(const glm::mat<n,n,float>& m) const{
    Sphere<n>* result = new Sphere<n>(*this);
    *result *= m;
    return result;
}

template<int n>
float* Sphere<n>::getPoints() const{
    // build triangle list for stacks x slices
    int tris = stacks * slices * 2; // two triangles per quad
    int verts = tris * 3;
    int totalFloats = verts * n;
    float* result = new float[totalFloats];
    int idx = 0;

    for (int s = 0; s < stacks; ++s){
        float phi1 = M_PI * ( (float)s / stacks - 0.5f );
        float phi2 = M_PI * ( (float)(s+1) / stacks - 0.5f );
        for (int t = 0; t < slices; ++t){
            float theta1 = 2.0f * M_PI * ( (float)t / slices );
            float theta2 = 2.0f * M_PI * ( (float)(t+1) / slices );

            this->pushVertex(result, idx, phi1, theta1);
            this->pushVertex(result, idx, phi2, theta1);
            this->pushVertex(result, idx, phi1, theta2);

            this->pushVertex(result, idx, phi1, theta2);
            this->pushVertex(result, idx, phi2, theta1);
            this->pushVertex(result, idx, phi2, theta2);
        }
    }
    return result;
}

template<int n>
void Sphere<n>::pushVertex(float* result, int &idx, float phi, float theta) const{
    // position
    float px = radius * cosf(phi) * cosf(theta) + center[0];
    float py = radius * cosf(phi) * sinf(theta) + center[1];
    float pz = radius * sinf(phi) + center[2];
    result[idx++] = px;
    result[idx++] = py;
    result[idx++] = pz;
    if (n > 3) result[idx++] = 1.0f;
}

template<int n>
int Sphere<n>::getNumPoints() const{
    int tris = stacks * slices * 2;
    int verts = tris * 3;
    return verts * n;
}

template<int n>
float* Sphere<n>::getTexCoords() const {
    int tris = stacks * slices * 2;
    int verts = tris * 3;
    float* result = new float[verts * 2];
    int idx = 0;
    for (int s = 0; s < stacks; ++s){
        float phi1 = M_PI * ( (float)s / stacks - 0.5f );
        float phi2 = M_PI * ( (float)(s+1) / stacks - 0.5f );
        for (int t = 0; t < slices; ++t){
            float theta1 = 2.0f * M_PI * ( (float)t / slices );
            float theta2 = 2.0f * M_PI * ( (float)(t+1) / slices );

            float u = theta1 / (2.0f * M_PI);
            if (u < 0.0f) u += 1.0f;
            float v = (phi1 + (M_PI * 0.5f)) / M_PI;
            result[idx++] = u; result[idx++] = v;

            u = theta1 / (2.0f * M_PI);
            if (u < 0.0f) u += 1.0f;
            v = (phi2 + (M_PI * 0.5f)) / M_PI;
            result[idx++] = u; result[idx++] = v;

            u = theta2 / (2.0f * M_PI);
            if (u < 0.0f) u += 1.0f;
            v = (phi1 + (M_PI * 0.5f)) / M_PI;
            result[idx++] = u; result[idx++] = v;

            u = theta2 / (2.0f * M_PI);
            if (u < 0.0f) u += 1.0f;
            v = (phi1 + (M_PI * 0.5f)) / M_PI;
            result[idx++] = u; result[idx++] = v;

            u = theta1 / (2.0f * M_PI);
            if (u < 0.0f) u += 1.0f;
            v = (phi2 + (M_PI * 0.5f)) / M_PI;
            result[idx++] = u; result[idx++] = v;

            u = theta2 / (2.0f * M_PI);
            if (u < 0.0f) u += 1.0f;
            v = (phi2 + (M_PI * 0.5f)) / M_PI;
            result[idx++] = u; result[idx++] = v;
        }
    }
    return result;
}

template<int n>
int Sphere<n>::getNumTexCoords() const {
    int tris = stacks * slices * 2;
    int verts = tris * 3;
    return verts * 2;
}

template<int n>
float* Sphere<n>::getNormals() const {
    int tris = stacks * slices * 2;
    int verts = tris * 3;
    float* result = new float[verts * 3];
    int idx = 0;
    for (int s = 0; s < stacks; ++s){
        float phi1 = M_PI * ( (float)s / stacks - 0.5f );
        float phi2 = M_PI * ( (float)(s+1) / stacks - 0.5f );
        for (int t = 0; t < slices; ++t){
            float theta1 = 2.0f * M_PI * ( (float)t / slices );
            float theta2 = 2.0f * M_PI * ( (float)(t+1) / slices );

            float px = radius * cosf(phi1) * cosf(theta1) + center[0];
            float py = radius * cosf(phi1) * sinf(theta1) + center[1];
            float pz = radius * sinf(phi1) + center[2];
            float nx = (px - center[0]) / radius;
            float ny = (py - center[1]) / radius;
            float nz = (pz - center[2]) / radius;
            float len = sqrtf(nx*nx + ny*ny + nz*nz);
            if (len>0.0f) { nx/=len; ny/=len; nz/=len; }
            result[idx++] = nx; result[idx++] = ny; result[idx++] = nz;

            px = radius * cosf(phi2) * cosf(theta1) + center[0];
            py = radius * cosf(phi2) * sinf(theta1) + center[1];
            pz = radius * sinf(phi2) + center[2];
            nx = (px - center[0]) / radius;
            ny = (py - center[1]) / radius;
            nz = (pz - center[2]) / radius;
            len = sqrtf(nx*nx + ny*ny + nz*nz);
            if (len>0.0f) { nx/=len; ny/=len; nz/=len; }
            result[idx++] = nx; result[idx++] = ny; result[idx++] = nz;

            px = radius * cosf(phi1) * cosf(theta2) + center[0];
            py = radius * cosf(phi1) * sinf(theta2) + center[1];
            pz = radius * sinf(phi1) + center[2];
            nx = (px - center[0]) / radius;
            ny = (py - center[1]) / radius;
            nz = (pz - center[2]) / radius;
            len = sqrtf(nx*nx + ny*ny + nz*nz);
            if (len>0.0f) { nx/=len; ny/=len; nz/=len; }
            result[idx++] = nx; result[idx++] = ny; result[idx++] = nz;

            // second triangle
            px = radius * cosf(phi1) * cosf(theta2) + center[0];
            py = radius * cosf(phi1) * sinf(theta2) + center[1];
            pz = radius * sinf(phi1) + center[2];
            nx = (px - center[0]) / radius;
            ny = (py - center[1]) / radius;
            nz = (pz - center[2]) / radius;
            len = sqrtf(nx*nx + ny*ny + nz*nz);
            if (len>0.0f) { nx/=len; ny/=len; nz/=len; }
            result[idx++] = nx; result[idx++] = ny; result[idx++] = nz;

            px = radius * cosf(phi2) * cosf(theta1) + center[0];
            py = radius * cosf(phi2) * sinf(theta1) + center[1];
            pz = radius * sinf(phi2) + center[2];
            nx = (px - center[0]) / radius;
            ny = (py - center[1]) / radius;
            nz = (pz - center[2]) / radius;
            len = sqrtf(nx*nx + ny*ny + nz*nz);
            if (len>0.0f) { nx/=len; ny/=len; nz/=len; }
            result[idx++] = nx; result[idx++] = ny; result[idx++] = nz;

            px = radius * cosf(phi2) * cosf(theta2) + center[0];
            py = radius * cosf(phi2) * sinf(theta2) + center[1];
            pz = radius * sinf(phi2) + center[2];
            nx = (px - center[0]) / radius;
            ny = (py - center[1]) / radius;
            nz = (pz - center[2]) / radius;
            len = sqrtf(nx*nx + ny*ny + nz*nz);
            if (len>0.0f) { nx/=len; ny/=len; nz/=len; }
            result[idx++] = nx; result[idx++] = ny; result[idx++] = nz;
        }
    }
    return result;
}

template<int n>
int Sphere<n>::getNumNormals() const {
    int tris = stacks * slices * 2;
    int verts = tris * 3;
    return verts * 3;
}

template<int n>
glm::vec<n,float> Sphere<n>::normalAtParams(float phi, float theta) const {
    glm::vec<n,float> out;
    // unit normal in local sphere coordinates (before center offset)
    float nx = cosf(phi) * cosf(theta);
    float ny = cosf(phi) * sinf(theta);
    float nz = sinf(phi);
    out[0] = nx; out[1] = ny; out[2] = nz;
    if (n > 3) out[3] = 0.0f;
    // normalize (should already be unit, but keep robust)
    float len = 0.0f;
    for (int i = 0; i < 3; ++i) len += out[i]*out[i];
    len = sqrtf(len);
    if (len > 0.0f) for (int i = 0; i < 3; ++i) out[i] /= len;
    return out;
}

template<int n>
glm::vec<n,float> Sphere<n>::normalAtPoint(const glm::vec<n,float>& p) const {
    glm::vec<n,float> out;
    // vector from center to point
    for (int i = 0; i < 3; ++i) out[i] = p[i] - center[i];
    // normalize
    float len = 0.0f;
    for (int i = 0; i < 3; ++i) len += out[i]*out[i];
    len = sqrtf(len);
    if (len > 0.0f) for (int i = 0; i < 3; ++i) out[i] /= len;
    if (n > 3) out[3] = 0.0f;
    return out;
}

template<int n>
void Sphere<n>::print() const{
    std::cout << "_ Sphere Center _" << std::endl; center.print();
    std::cout << "radius: " << radius << " stacks:" << stacks << " slices:" << slices << std::endl;
}

template<int n>
std::string Sphere<n>::fprint() const{
    std::ostringstream ss; ss << std::fixed << std::setprecision(6);
    ss << center[0] << ' ';
    if (n>1) ss << center[1] << ' ';
    if (n>2) ss << center[2] << ' ';
    ss << radius << ' ' << stacks << ' ' << slices << ' ';
    int r = (int)roundf(this->colour[0]*255.0f);
    int g = (int)roundf(this->colour[1]*255.0f);
    int b = (int)roundf(this->colour[2]*255.0f);
    ss << r << ' ' << g << ' ' << b << ' ' << this->colour[3];
    return ss.str();
}

template<int n>
void Sphere<n>::zoom(int percent){
    float factor = percent / 100.0f;
    radius *= factor;
}

template<int n>
void Sphere<n>::rotate(int degrees){
    // rotate center by matrix around origin
    *this *= rotz<n>(degrees);
}

template<int n>
void Sphere<n>::createGLBuffers(GLenum usage){
    Shape<n>::createGLBuffers(usage);

    // Build line indices for wireframe: every triangle's three edges
    int verts = getNumPoints() / n;
    int triangles = verts / 3;
    std::vector<GLuint> indices;
    indices.reserve(triangles * 6);
    for (int t = 0; t < triangles; ++t) {
        GLuint b = (GLuint)(t * 3);
        indices.push_back(b);
        indices.push_back(b + 1);
        indices.push_back(b + 1);
        indices.push_back(b + 2);
        indices.push_back(b + 2);
        indices.push_back(b);
    }
    if (!indices.empty()) this->setLineIndices(indices);
}

template<int n>
void Sphere<n>::updateGLBuffers(GLenum usage){
    Shape<n>::updateGLBuffers(usage);
}

template<int n>
GLenum Sphere<n>::glDrawMode() const{ return GL_TRIANGLES; }

template<int n>
void Sphere<n>::draw(bool wireframe){
    if (this->VAO == 0u) this->createGLBuffers();
    Shape<n>::draw(wireframe);
}
