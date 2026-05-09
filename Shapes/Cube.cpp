template<int n>
Cube<n>::Cube(const glm::vec<n,float>& center, float height, float width, float depth){
	this->center = center;
	this->height = height;
	this->width = width;
	this->depth = depth;

	float hx = width/2.0f;
	float hy = height/2.0f;
	float hz = depth/2.0f;

	glm::vec<n,float> p000 = {center[0]-hx, center[1]-hy, center[2]-hz};
	glm::vec<n,float> p001 = {center[0]-hx, center[1]-hy, center[2]+hz};
	glm::vec<n,float> p010 = {center[0]-hx, center[1]+hy, center[2]-hz};
	glm::vec<n,float> p011 = {center[0]-hx, center[1]+hy, center[2]+hz};
	glm::vec<n,float> p100 = {center[0]+hx, center[1]-hy, center[2]-hz};
	glm::vec<n,float> p101 = {center[0]+hx, center[1]-hy, center[2]+hz};
	glm::vec<n,float> p110 = {center[0]+hx, center[1]+hy, center[2]-hz};
	glm::vec<n,float> p111 = {center[0]+hx, center[1]+hy, center[2]+hz};

	// Front face (z = +)
	f1 = Square<n>(p011, p111, p101, p001);
	// Back face (z = -) -- winding chosen consistently
	f2 = Square<n>(p110, p010, p000, p100);
	// Left face (x = -)
	f3 = Square<n>(p010, p011, p001, p000);
	// Right face (x = +)
	f4 = Square<n>(p111, p110, p100, p101);
	// Top face (y = +)
	f5 = Square<n>(p010, p110, p111, p011);
	// Bottom face (y = -)
	f6 = Square<n>(p001, p101, p100, p000);
}

/// @brief This is to make rectangular prisms that are not exactly on any of the axis
/// @tparam n 
/// @param f1 
/// @param f2 
template<int n>
Cube<n>::Cube(const Square<n>& f1, const Square<n>& f2){
	// extract corner points from each square (order: tl, tr, br, bl)
	float* pa = f1.getPoints();
	float* pb = f2.getPoints();

	glm::vec<n,float> va[4];
	glm::vec<n,float> vb[4];
	for (int k = 0; k < 4; ++k) {
		for (int i = 0; i < n; ++i) {
			va[k][i] = pa[k*n + i];
			vb[k][i] = pb[k*n + i];
		}
	}
	delete[] pa; delete[] pb;

	// compute centroids of both faces
	glm::vec<n,float> ca; glm::vec<n,float> cb;
	for (int i = 0; i < n; ++i) {
		ca[i] = (va[0][i] + va[1][i] + va[2][i] + va[3][i]) / 4.0f;
		cb[i] = (vb[0][i] + vb[1][i] + vb[2][i] + vb[3][i]) / 4.0f;
		// cube center is midpoint between centroids
		center[i] = (ca[i] + cb[i]) / 2.0f;
	}

	// store the two given faces
	this->f1 = f1;
	this->f2 = f2;

	// build the four side faces by connecting corresponding edges
	this->f3 = Square<n>(va[0], va[1], vb[1], vb[0]);
	this->f4 = Square<n>(va[1], va[2], vb[2], vb[1]);
	this->f5 = Square<n>(va[2], va[3], vb[3], vb[2]);
	this->f6 = Square<n>(va[3], va[0], vb[0], vb[3]);

	// inherit colour from first face
	{
		float* col = f1.getColour();
		for (int i = 0; i < 4; ++i) this->colour[i] = col[i];
		delete[] col;
	}
}

template<int n>
Cube<n>::Cube(){
	center = glm::vec<n,float>();
}

template<int n>
Cube<n>::Cube(const Cube<n>& two){
	this->f1 = two.f1;
	this->f2 = two.f2;
	this->f3 = two.f3;
	this->f4 = two.f4;
	this->f5 = two.f5;
	this->f6 = two.f6;
	this->center = two.center;

	{
		float* c = two.getColour();
		for (int i = 0; i < 4; ++i) this->colour[i] = c[i];
		delete[] c;
	}
}

template<int n>
Cube<n>& Cube<n>::operator*=(const glm::mat<n,n,float>& m){
	f1 *= m;
	f2 *= m;
	f3 *= m;
	f4 *= m;
	f5 *= m;
	f6 *= m;
	// update center if possible
	glm::vec<n,float> hc = center;
	center = glm::vec<n,float>(m * hc);
	return *this;
}

template<int n>
Cube<n>* Cube<n>::operator*(const glm::mat<n,n,float>& m) const{
	Cube<n>* result = new Cube<n>(*this);
	*result *= m;
	return result;
}

template<int n>
float* Cube<n>::getPoints() const{
	int perFace = f1.getNumPoints();
	int total = perFace * 6;
	float* result = new float[total];
	int idx = 0;
	float* pts;

	pts = f1.getPoints();
	for (int i=0;i<perFace;i++) result[idx++]=pts[i];
	delete[] pts;

	pts = f2.getPoints();
	for (int i=0;i<perFace;i++) result[idx++]=pts[i];
	delete[] pts;

	pts = f3.getPoints();
	for (int i=0;i<perFace;i++) result[idx++]=pts[i];
	delete[] pts;

	pts = f4.getPoints();
	for (int i=0;i<perFace;i++) result[idx++]=pts[i];
	delete[] pts;

	pts = f5.getPoints();
	for (int i=0;i<perFace;i++) result[idx++]=pts[i];
	delete[] pts;

	pts = f6.getPoints();
	for (int i=0;i<perFace;i++) result[idx++]=pts[i];
	delete[] pts;

	return result;
}

template<int n>
int Cube<n>::getNumPoints() const{
	return f1.getNumPoints() * 6;
}

template<int n>
void Cube<n>::draw(bool wireframe){
	// Delegate drawing to each face so faces' own EBOs/VAOs are used
	f1.draw(wireframe);
	f2.draw(wireframe);
	f3.draw(wireframe);
	f4.draw(wireframe);
	f5.draw(wireframe);
	f6.draw(wireframe);
}

template<int n>
void Cube<n>::createGLBuffers(GLenum usage){
	// Delegate buffer creation to each face so they maintain their own VAO/VBO/EBO
	f1.createGLBuffers(usage);
	f2.createGLBuffers(usage);
	f3.createGLBuffers(usage);
	f4.createGLBuffers(usage);
	f5.createGLBuffers(usage);
	f6.createGLBuffers(usage);
}

template<int n>
void Cube<n>::updateGLBuffers(GLenum usage){
	f1.updateGLBuffers(usage);
	f2.updateGLBuffers(usage);
	f3.updateGLBuffers(usage);
	f4.updateGLBuffers(usage);
	f5.updateGLBuffers(usage);
	f6.updateGLBuffers(usage);
}

template<int n>
void Cube<n>::setColour(int r, int g, int b, float a){
	Shape<n>::setColour(r,g,b,a);
	f1.setColour(r,g,b,a);
	f2.setColour(r,g,b,a);
	f3.setColour(r,g,b,a);
	f4.setColour(r,g,b,a);
	f5.setColour(r,g,b,a);
	f6.setColour(r,g,b,a);
}

template<int n>
GLenum Cube<n>::glDrawMode() const {
	return GL_TRIANGLE_FAN;
}

template<int n>
void Cube<n>::print() const{
	std::cout << "_ Cube Center _" << std::endl;
	center.print();
	std::cout << "_ Front Face _" << std::endl; f1.print();
	std::cout << "_ Back Face _" << std::endl; f2.print();
	std::cout << "_ Left Face _" << std::endl; f3.print();
	std::cout << "_ Right Face _" << std::endl; f4.print();
	std::cout << "_ Top Face _" << std::endl; f5.print();
	std::cout << "_ Bottom Face _" << std::endl; f6.print();
}

template<int n>
void Cube<n>::zoom(int percent){
	float factor = percent / 100.0f;
	*this *= translation<n>(-center[0], -center[1], -center[2]);
	*this *= scaling<n>(factor);
	*this *= translation<n>(center[0], center[1], center[2]);
}

template<int n>
void Cube<n>::rotate(int degrees){
	*this *= translation<n>(-center[0], -center[1], -center[2]);
	*this *= rotz<n>(degrees);
	*this *= translation<n>(center[0], center[1], center[2]);
}

template<int n>
std::string Cube<n>::fprint() const {
	std::ostringstream ss;
	ss << std::fixed << std::setprecision(6);
	float* pts = getPoints();
	int total = getNumPoints();
	for (int i = 0; i < total; ++i) ss << pts[i] << ' ';
	delete[] pts;
	int r = (int)roundf(this->colour[0]*255.0f);
	int g = (int)roundf(this->colour[1]*255.0f);
	int b = (int)roundf(this->colour[2]*255.0f);
	ss << r << ' ' << g << ' ' << b << ' ' << this->colour[3];
	return ss.str();
}
