
template<int n>
Triangle<n>::Triangle(const glm::vec<n,float>& p1, const glm::vec<n,float>& p2, const glm::vec<n,float>& p3){
    this->p1=p1;
    this->p2=p2;
    this->p3=p3;
}

template<int n>
Triangle<n>::Triangle(){
    p1 = glm::vec<n,float>();
    p2 = glm::vec<n,float>();
    p3 = glm::vec<n,float>();
}


template<int n>
Triangle<n>::Triangle(const Triangle<n> &t) : Shape<n>(t) {
    this->p1 = t.p1;
    this->p2 = t.p2;
    this->p3 = t.p3;
}

template<int n>
Triangle<n>& Triangle<n>::operator*=(const glm::mat<n,n,float> &m){
    
    
    

    p1 = m * p1;
    p2 = m * p2;
    p3 = m * p3;

    return *this;
}

template<int n>
Triangle<n>* Triangle<n>::operator*(const glm::mat<n,n,float> &m) const{
    Triangle<n>* result=new Triangle<n>(*this);
    *result*=m;
    return result;
}

template<int n>
float* Triangle<n>::getPoints() const{
    float* result=new float[3*n];
    int total=0;
    for(int i=0;i<n;i++){
        result[i]=p1[i];
        total++;
    }
    for(int i=0;i<n;i++){
        result[total]=p2[i];
        total++;
    }
    for(int i=0;i<n;i++){
        result[total]=p3[i];
        total++;
    }
    return result;
}

template<int n>
int Triangle<n>::getNumPoints() const{
    return 3*n;
}

template<int n>
float* Triangle<n>::getNormals() const{
    float* result = new float[9];

    glm::vec<n, float> edge1 = p2 - p1;
    glm::vec<n, float> edge2 = p3 - p1;
    glm::vec3 normal = glm::cross(glm::vec3(edge2), glm::vec3(edge1));
    float length = glm::length(normal);
    if (length > 0.0f) {
        normal /= length;
    } else {
        normal = glm::vec3(0.0f, 0.0f, 1.0f);
    }

    for (int vertex = 0; vertex < 3; ++vertex) {
        for (int component = 0; component < 3; ++component) {
            result[vertex * 3 + component] = normal[component];
        }
    }

    return result;
}

template<int n>
int Triangle<n>::getNumNormals() const{
    return 9;
}

template<int n>
void Triangle<n>::zoom(int percent){
    float factor = percent / 100.0f;
    // compute centroid
    glm::vec<n,float> center;
    for (int i = 0; i < n; ++i) {
        center[i] = (p1[i] + p2[i] + p3[i]) / 3.0f;
    }

    // translate to origin, scale, translate back
    *this *= translation<n>(-center[0], -center[1], 0.0f);
    *this *= scaling<n>(factor);
    *this *= translation<n>(center[0], center[1], 0.0f);
}

template<int n>
void Triangle<n>::rotate(int degrees){
    // compute centroid
    glm::vec<n,float> center;
    for (int i = 0; i < n; ++i) {
        center[i] = (p1[i] + p2[i] + p3[i]) / 3.0f;
    }

    // rotate about centroid: translate -> rotate -> translate back
    *this *= translation<n>(-center[0], -center[1], 0.0f);
    *this *= rotz<n>(degrees);
    *this *= translation<n>(center[0], center[1], 0.0f);
}

template<int n>
std::string Triangle<n>::fprint() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6);
    float* pts = getPoints();
    for (int i = 0; i < 9; ++i) ss << pts[i] << ' ';
    delete[] pts;

    int r = (int)roundf(this->colour[0]*255.0f);
    int g = (int)roundf(this->colour[1]*255.0f);
    int b = (int)roundf(this->colour[2]*255.0f);
    ss << r << ' ' << g << ' ' << b << ' ' << this->colour[3];
    return ss.str();
}

template<int n>
GLenum Triangle<n>::glDrawMode() const {
    return GL_TRIANGLES;
}

template<int n>
void Triangle<n>::createGLBuffers(GLenum usage){
    Shape<n>::createGLBuffers(usage);
}

template<int n>
void Triangle<n>::draw(){
    if (this->VAO == 0u) this->createGLBuffers();
    
    glBindVertexArray(this->VAO);
    glDisableVertexAttribArray(1);
    glVertexAttrib4f(1, this->colour[0], this->colour[1], this->colour[2], this->colour[3]);
    
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    glBindVertexArray(0);
}

