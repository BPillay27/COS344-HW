

template <int n>
Square<n>::Square(const glm::vec<n,float>& center, float height, float width){
    tl=glm::vec<n,float>(center);
    tr=glm::vec<n,float>(center);
    br=glm::vec<n,float>(center);
    bl=glm::vec<n,float>(center);
    tl[0]-=width/2;
    tl[1]+=height/2;
    tr[0]+=width/2;
    tr[1]+=height/2;
    br[0]+=width/2;
    br[1]-=height/2;
    bl[0]-=width/2;
    bl[1]-=height/2;
}

template<int n>
Square<n>::Square(){
    tl = glm::vec<n,float>();
    tr = glm::vec<n,float>();
    br = glm::vec<n,float>();
    bl = glm::vec<n,float>();
}

template <int n>
Square<n>::Square(const glm::vec<n,float>& tl, const glm::vec<n,float>& tr, const glm::vec<n,float>& br, const glm::vec<n,float>& bl){
    this->tl=tl;
    this->tr=tr;
    this->br=br;
    this->bl=bl;
}

template <int n>
    Square<n>::Square(const Square<n> &two) : Shape<n>(two) {
    this->tl=two.tl;
    this->tr=two.tr;
    this->br=two.br;
    this->bl=two.bl;
}

template <int n>
Square<n>& Square<n>::operator*=(const glm::mat<n,n,float> &m){
    
    
    
    

    tl = m * tl;
    tr = m * tr;
    br = m * br;
    bl = m * bl;

    return *this;
}

template <int n>
Square<n>* Square<n>::operator*(const glm::mat<n,n,float> &m) const{
    Square<n>* result=new Square<n>(*this);
    *result*=m;
    return result;
}

template <int n>
float* Square<n>::getPoints() const{
    float* result=new float[4*n];
    int total=0;
    for(int i=0;i<n;i++){
        result[i]=tl[i];
        total++;
    }
    for(int i=0;i<n;i++){
        result[total]=tr[i];
        total++;
    }
    for(int i=0;i<n;i++){
        result[total]=br[i];
        total++;
    }
    for(int i=0;i<n;i++){
        result[total]=bl[i];
        total++;
    }
    return result;
}

template <int n>
int Square<n>::getNumPoints() const{
    return 4*n;
}

template <int n>
float* Square<n>::getNormals() const{
    float* result = new float[12];

    glm::vec<n, float> edge1 = tr - tl;
    glm::vec<n, float> edge2 = br - tl;
    glm::vec3 normal = glm::cross(glm::vec3(edge2), glm::vec3(edge1));
    float length = glm::length(normal);
    if (length > 0.0f) {
        normal /= length;
    } else {
        normal = glm::vec3(0.0f, 0.0f, 1.0f);
    }

    for (int vertex = 0; vertex < 4; ++vertex) {
        for (int component = 0; component < 3; ++component) {
            result[vertex * 3 + component] = normal[component];
        }
    }

    return result;
}

template <int n>
int Square<n>::getNumNormals() const{
    return 12;
}

template<int n>
void Square<n>::zoom(int percent){
    float factor = percent / 100.0f;
    // compute centroid
    glm::vec<n,float> center;
    for (int i = 0; i < n; ++i) {
        center[i] = (tl[i] + tr[i] + br[i] + bl[i]) / 4.0f;
    }

    // translate to origin, scale, translate back
    *this *= translation<n>(-center[0], -center[1], 0.0f);
    *this *= scaling<n>(factor);
    *this *= translation<n>(center[0], center[1], 0.0f);
}

template<int n>
void Square<n>::rotate(int degrees){
    // compute centroid
    glm::vec<n,float> center;
    for (int i = 0; i < n; ++i) {
        center[i] = (tl[i] + tr[i] + br[i] + bl[i]) / 4.0f;
    }

    // rotate about centroid: translate -> rotate -> translate back
    *this *= translation<n>(-center[0], -center[1], 0.0f);
    *this *= rotz<n>(degrees);
    *this *= translation<n>(center[0], center[1], 0.0f);
}

template<int n>
std::string Square<n>::fprint() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6);
    float* pts = getPoints();
    for (int i = 0; i < 12; ++i) ss << pts[i] << ' ';
    delete[] pts;
    int r = (int)roundf(this->colour[0]*255.0f);
    int g = (int)roundf(this->colour[1]*255.0f);
    int b = (int)roundf(this->colour[2]*255.0f);
    ss << r << ' ' << g << ' ' << b << ' ' << this->colour[3];
    return ss.str();
}

template<int n>
void Square<n>::draw() {
    if (this->VAO == 0u) this->createGLBuffers();
    int verts = getNumPoints() / n;
    glBindVertexArray(this->VAO);
    glDisableVertexAttribArray(1);
    glVertexAttrib4f(1, this->colour[0], this->colour[1], this->colour[2], this->colour[3]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, verts);
    glBindVertexArray(0);
}

template<int n>
GLenum Square<n>::glDrawMode() const {
    return GL_TRIANGLE_FAN;
}

template<int n>
void Square<n>::createGLBuffers(GLenum usage){
    Shape<n>::createGLBuffers(usage);
}
