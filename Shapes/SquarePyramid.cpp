template<int n>
SquarePyramid<n>::SquarePyramid(){
    base = Square<n>();
    for (int i = 0; i < 4; ++i) sides[i] = Triangle<n>();
}

template<int n>
SquarePyramid<n>::SquarePyramid(const glm::vec<n,float>& baseCenter, float baseSize, float height, int axis){
    // Build a square base centered at baseCenter in the plane perpendicular to axis
    float half = baseSize / 2.0f;
    glm::vec<n,float> tl = baseCenter;
    glm::vec<n,float> tr = baseCenter;
    glm::vec<n,float> br = baseCenter;
    glm::vec<n,float> bl = baseCenter;
    // Default: axis == 2 -> Z axis (change Z coordinate for front/back)
    if (axis == 2) {
        tl[0] -= half; tl[1] += half;
        tr[0] += half; tr[1] += half;
        br[0] += half; br[1] -= half;
        bl[0] -= half; bl[1] -= half;
    } else if (axis == 1) {
        tl[0] -= half; tl[2] += half;
        tr[0] += half; tr[2] += half;
        br[0] += half; br[2] -= half;
        bl[0] -= half; bl[2] -= half;
    } else {
        tl[1] -= half; tl[2] += half;
        tr[1] += half; tr[2] += half;
        br[1] += half; br[2] -= half;
        bl[1] -= half; bl[2] -= half;
    }
    base = Square<n>(tl, tr, br, bl);
    glm::vec<n,float> apex = baseCenter;
    if (axis == 0) apex[0] += height;
    else if (axis == 1) apex[1] += height;
    else apex[2] += height;
    // Make the four triangular sides
    float* bp = base.getPoints();
    // base.getPoints gives tl,tr,br,bl (n floats each)
    glm::vec<n,float> bverts[4];
    for (int v = 0; v < 4; ++v) {
        for (int j = 0; j < n; ++j) bverts[v][j] = bp[v*n + j];
    }
    delete[] bp;
    sides[0] = Triangle<n>(bverts[0], bverts[1], apex);
    sides[1] = Triangle<n>(bverts[1], bverts[2], apex);
    sides[2] = Triangle<n>(bverts[2], bverts[3], apex);
    sides[3] = Triangle<n>(bverts[3], bverts[0], apex);
}

template<int n>
SquarePyramid<n>::SquarePyramid(const glm::vec<n,float>& apex, const glm::vec<n,float>& baseCenter, float baseSize){
    // Reuse the other constructor: infer axis = 2 (Z) by default
    *this = SquarePyramid<n>(baseCenter, baseSize, sqrtf( (apex[0]-baseCenter[0])*(apex[0]-baseCenter[0]) + (apex[1]-baseCenter[1])*(apex[1]-baseCenter[1]) + (apex[2]-baseCenter[2])*(apex[2]-baseCenter[2]) ), 2);
}

template<int n>
SquarePyramid<n>::SquarePyramid(const Square<n>& baseSquare, const glm::vec<n,float>& apex){
    base = baseSquare;
    float* bp = base.getPoints();
    glm::vec<n,float> bverts[4];
    for (int v = 0; v < 4; ++v) {
        for (int j = 0; j < n; ++j) bverts[v][j] = bp[v*n + j];
    }
    delete[] bp;
    sides[0] = Triangle<n>(bverts[0], bverts[1], apex);
    sides[1] = Triangle<n>(bverts[1], bverts[2], apex);
    sides[2] = Triangle<n>(bverts[2], bverts[3], apex);
    sides[3] = Triangle<n>(bverts[3], bverts[0], apex);
}

template<int n>
SquarePyramid<n>::SquarePyramid(const SquarePyramid<n>& other){
    base = other.base;
    for (int i = 0; i < 4; ++i) sides[i] = other.sides[i];
    float* c = other.getColour();
    for (int i = 0; i < 4; ++i) this->colour[i] = c[i];
    delete[] c;
}

template<int n>
SquarePyramid<n>& SquarePyramid<n>::operator*=(const glm::mat<n,n,float>& m){
    base *= m;
    for (int i = 0; i < 4; ++i) sides[i] *= m;
    return *this;
}

template<int n>
SquarePyramid<n>* SquarePyramid<n>::operator*(const glm::mat<n,n,float>& m) const{
    SquarePyramid<n>* result = new SquarePyramid<n>(*this);
    *result *= m;
    return result;
}

template<int n>
float* SquarePyramid<n>::getPoints() const{
    // Concatenate base points then sides points for compatibility
    int b = base.getNumPoints();
    int s = 0;
    for (int i = 0; i < 4; ++i) s += sides[i].getNumPoints();
    int total = b + s;
    float* result = new float[total];
    int idx = 0;
    float* bp = base.getPoints();
    for (int i = 0; i < b; ++i) result[idx++] = bp[i];
    delete[] bp;
    for (int i = 0; i < 4; ++i) {
        float* sp = sides[i].getPoints();
        int spn = sides[i].getNumPoints();
        for (int j = 0; j < spn; ++j) result[idx++] = sp[j];
        delete[] sp;
    }
    return result;
}

template<int n>
int SquarePyramid<n>::getNumPoints() const{
    int total = base.getNumPoints();
    for (int i = 0; i < 4; ++i) total += sides[i].getNumPoints();
    return total;
}

template<int n>
void SquarePyramid<n>::draw(bool wireframe){
    base.draw(wireframe);
    for (int i = 0; i < 4; ++i) sides[i].draw(wireframe);
}

template<int n>
void SquarePyramid<n>::createGLBuffers(GLenum usage){
    base.createGLBuffers(usage);
    for (int i = 0; i < 4; ++i) sides[i].createGLBuffers(usage);
}

template<int n>
void SquarePyramid<n>::updateGLBuffers(GLenum usage){
    base.updateGLBuffers(usage);
    for (int i = 0; i < 4; ++i) sides[i].updateGLBuffers(usage);
}

template<int n>
GLenum SquarePyramid<n>::glDrawMode() const{
    return GL_TRIANGLES;
}

template<int n>
void SquarePyramid<n>::print() const{
    std::cout << "SquarePyramid composed of base and 4 sides" << std::endl;
    base.print();
    for (int i = 0; i < 4; ++i) sides[i].print();
}

template<int n>
void SquarePyramid<n>::zoom(int percent){
    base.zoom(percent);
    for (int i = 0; i < 4; ++i) sides[i].zoom(percent);
}

template<int n>
void SquarePyramid<n>::rotate(int degrees){
    base.rotate(degrees);
    for (int i = 0; i < 4; ++i) sides[i].rotate(degrees);
}

template<int n>
std::string SquarePyramid<n>::fprint() const{
    return "SquarePyramid(composed)";
}


template<int n>
void SquarePyramid<n>::setColour(int r, int g, int b, float a){
	Shape<n>::setColour(r,g,b,a);
	base.setColour(r,g,b,a);
	sides[0].setColour(r,g,b,a);
	sides[1].setColour(r,g,b,a);
	sides[2].setColour(r,g,b,a);
	sides[3].setColour(r,g,b,a);
}
