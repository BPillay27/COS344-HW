#include "transformation.h"

template<int n>
glm::mat<n,n,float> roty(int m){
    float theta = (float)m * 3.14159265358979323846f / 180.0f;
    glm::mat<n,n,float> R;
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            
            if (i==j) {
                R[i][j] = 1.0f;
            } else {
                R[i][j] = 0.0f;
            }
        }
    }
    if(n>=3){
        float c = cosf(theta);
        float s = sinf(theta);
        R[0][0]= c; R[0][2]= s;
        R[2][0]= -s; R[2][2]= c;
    }
    return glm::transpose(R);
}

template<int n>
glm::mat<n,n,float> rotx(int m){
    float theta = (float)m * 3.14159265358979323846f / 180.0f;
    glm::mat<n,n,float> R;
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            if (i==j) {
                R[i][j] = 1.0f;
            } else {
                R[i][j] = 0.0f;
            }
        }
    }
    if(n>=3){
        float c = cosf(theta);
        float s = sinf(theta);
        R[1][1]= c; R[1][2]= -s;
        R[2][1]= s; R[2][2]= c;
    }
    return glm::transpose(R);
}

template<int n>
glm::mat<n,n,float> rotz(int m){
    float theta = (float)m * 3.14159265358979323846f / 180.0f;
    glm::mat<n,n,float> R;
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            if(i==j){
                R[i][j]=1.0f;
            }else{
                R[i][j]=0.0f;
            }
        }
    }
    if(n>=3){
        float c = cosf(theta);
        float s = sinf(theta);
        R[0][0]= c; R[0][1]= -s;
        R[1][0]= s; R[1][1]= c;
    }
    return glm::transpose(R);
}

template<int n>
glm::mat<n,n,float> shearx(int xy,int xz){
    glm::mat<n,n,float> S;
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            if (i==j) {
                S[i][j] = 1.0f;
            } else {
                S[i][j] = 0.0f;
            }
        }
    }
    if(n>=3){
        S[0][1] = (float)xy;
        S[0][2] = (float)xz;
    }
    return glm::transpose(S);
}

template<int n>
glm::mat<n,n,float> sheary(int yx,int yz){
    glm::mat<n,n,float> S;
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            S[i][j]=(i==j)?1.0f:0.0f;
        }
    }
    if(n>=3){
        S[1][0] = (float)yx;
        S[1][2] = (float)yz;
    }
    return glm::transpose(S);
}

template<int n>
glm::mat<n,n,float> shearz(int zx,int zy){
    glm::mat<n,n,float> S;
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            S[i][j] = (i==j) ? 1.0f : 0.0f;
        }
    }
    if(n>=3){
        S[2][0] = (float)zx;
        S[2][1] = (float)zy;
    }
    return glm::transpose(S);
}

template<int n>
glm::mat<n,n,float> translation(float x, float y, float z) {
    glm::mat<n,n,float> T;

    // Start with identity
    for(int i=0; i<n; i++) {
        for(int j=0; j<n; j++) {
            T[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }

    // Handle homogeneous coordinates
    if(n == 3) {
        // 2D homogeneous transform (x,y translation)
        T[0][2] = x;
        T[1][2] = y;
    } else if(n == 4) {
        // 3D homogeneous transform (x,y,z translation)
        T[0][3] = x;
        T[1][3] = y;
        T[2][3] = z;
    } else if(n > 4) {
        // General case: put translation in last column
        T[0][n-1] = x;
        T[1][n-1] = y;
        T[2][n-1] = z;
    }

    return glm::transpose(T);
}


template<int n>
glm::mat<n,n,float> scaling(float m){
    float s = m;
    glm::mat<n,n,float> S;
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            if (i==j) {
                S[i][j] = 1.0f;
            } else {
                S[i][j] = 0.0f;
            }
        }
    }
    if(n>=4){
        for(int i=0;i<n;i++){
            if (i==n-1) {
                S[i][i] = 1.0f;
            } else {
                S[i][i] = s;
            }
        }
    } else {
        for(int i=0;i<n;i++){
            S[i][i] = s;
        }
    }
    return glm::transpose(S);
}


