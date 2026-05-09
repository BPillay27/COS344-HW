template<int n>
TriangularPrism<n>::TriangularPrism(){
    triangle1 = Triangle<n>();
    triangle2 = Triangle<n>();
    side1 = Square<n>();
    side2 = Square<n>();
    side3 = Square<n>();
}

template<int n>
TriangularPrism<n>::TriangularPrism(const Triangle<n>& t1, const Triangle<n>& t2, const Square<n>& s1, const Square<n>& s2, const Square<n>& s3){
    triangle1 = t1;
    triangle2 = t2;
    side1 = s1;
    side2 = s2;
    side3 = s3;
}

template<int n>
TriangularPrism<n>::TriangularPrism(const Triangle<n>& t1, const Triangle<n>& t2){
    triangle1 = t1;
    triangle2 = t2;

    float* p1 = t1.getPoints();
    float* p2 = t2.getPoints();

    glm::vec<n,float> v1[3];
    glm::vec<n,float> v2[3];
    for (int vi = 0; vi < 3; ++vi) {
        for (int j = 0; j < n; ++j) {
            v1[vi][j] = p1[vi * n + j];
            v2[vi][j] = p2[vi * n + j];
        }
    }

    side1 = Square<n>(v1[0], v1[1], v2[1], v2[0]);
    side2 = Square<n>(v1[1], v1[2], v2[2], v2[1]);
    side3 = Square<n>(v1[2], v1[0], v2[0], v2[2]);
    delete[] p1;
    delete[] p2;
}

template<int n>
TriangularPrism<n>::TriangularPrism(const TriangularPrism<n>& other){
    triangle1 = other.triangle1;
    triangle2 = other.triangle2;
    side1 = other.side1;
    side2 = other.side2;
    side3 = other.side3;
    this->VAO = other.VAO;
    this->VBO = other.VBO;
    this->EBO = other.EBO;
}

template<int n>
TriangularPrism<n>& TriangularPrism<n>::operator*=(const glm::mat<n,n,float>& m){
    triangle1 *= m;
    triangle2 *= m;
    side1 *= m;
    side2 *= m;
    side3 *= m;
    return *this;
}

template<int n>
TriangularPrism<n>* TriangularPrism<n>::operator*(const glm::mat<n,n,float>& m) const{
    TriangularPrism<n>* result = new TriangularPrism<n>(*this);
    (*result) *= m;
    return result;
}

template<int n>
float* TriangularPrism<n>::getPoints() const{
    int t1Points = triangle1.getNumPoints();
    int t2Points = triangle2.getNumPoints();
    int s1Points = side1.getNumPoints();
    int s2Points = side2.getNumPoints();
    int s3Points = side3.getNumPoints();
    
    int totalPoints = t1Points + t2Points + s1Points + s2Points + s3Points;
    float* result = new float[totalPoints];
    
    float* t1Data = triangle1.getPoints();
    float* t2Data = triangle2.getPoints();
    float* s1Data = side1.getPoints();
    float* s2Data = side2.getPoints();
    float* s3Data = side3.getPoints();
    
    int idx = 0;
    for(int i = 0; i < t1Points; i++) result[idx++] = t1Data[i];
    for(int i = 0; i < t2Points; i++) result[idx++] = t2Data[i];
    for(int i = 0; i < s1Points; i++) result[idx++] = s1Data[i];
    for(int i = 0; i < s2Points; i++) result[idx++] = s2Data[i];
    for(int i = 0; i < s3Points; i++) result[idx++] = s3Data[i];
    
    delete[] t1Data;
    delete[] t2Data;
    delete[] s1Data;
    delete[] s2Data;
    delete[] s3Data;
    
    return result;
}

template<int n>
int TriangularPrism<n>::getNumPoints() const{
    return triangle1.getNumPoints() + triangle2.getNumPoints() + 
           side1.getNumPoints() + side2.getNumPoints() + side3.getNumPoints();
}

template<int n>
void TriangularPrism<n>::draw(bool wireframe){
    if (this->VAO == 0u) this->createGLBuffers();
    
    int t1Offset = 0;
    int t1Count = triangle1.getNumPoints() / n;
    
    int t2Offset = t1Count;
    int t2Count = triangle2.getNumPoints() / n;
    
    int s1Offset = t2Offset + t2Count;
    int s1Count = side1.getNumPoints() / n;
    
    int s2Offset = s1Offset + s1Count;
    int s2Count = side2.getNumPoints() / n;
    
    int s3Offset = s2Offset + s2Count;
    int s3Count = side3.getNumPoints() / n;
    
    glBindVertexArray(this->VAO);
    glDisableVertexAttribArray(1);
    glVertexAttrib4f(1, this->colour[0], this->colour[1], this->colour[2], this->colour[3]);
    
    glDrawArrays(GL_TRIANGLES, t1Offset, t1Count);
    glDrawArrays(GL_TRIANGLES, t2Offset, t2Count);
    glDrawArrays(GL_TRIANGLES, s1Offset, s1Count);
    glDrawArrays(GL_TRIANGLES, s2Offset, s2Count);
    glDrawArrays(GL_TRIANGLES, s3Offset, s3Count);
    
    glBindVertexArray(0);
}

template<int n>
void TriangularPrism<n>::createGLBuffers(GLenum usage){
    triangle1.createGLBuffers(usage);
    triangle2.createGLBuffers(usage);
    side1.createGLBuffers(usage);
    side2.createGLBuffers(usage);
    side3.createGLBuffers(usage);
}

template<int n>
void TriangularPrism<n>::print() const{
    std::cout << "TriangularPrism with:" << std::endl;
    triangle1.print();
    triangle2.print();
    side1.print();
    side2.print();
    side3.print();
}

template<int n>
std::string TriangularPrism<n>::fprint() const{
    return "TriangularPrism";
}

template<int n>
void TriangularPrism<n>::zoom(int percent){
    float scale = 1.0f + (percent / 100.0f);
    glm::mat<n,n,float> scaleMatrix;
    scaleMatrix.identity();
    for(int i = 0; i < n-1; i++) scaleMatrix[i][i] = scale;
    (*this) *= scaleMatrix;
}

template<int n>
void TriangularPrism<n>::rotate(int degrees){
    float rad = degrees * M_PI / 180.0f;
    glm::mat<n,n,float> rotMatrix;
    rotMatrix.identity();
    rotMatrix[0][0] = cosf(rad);
    rotMatrix[0][1] = -sinf(rad);
    rotMatrix[1][0] = sinf(rad);
    rotMatrix[1][1] = cosf(rad);
    (*this) *= rotMatrix;
}
