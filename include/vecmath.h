#ifndef VECMATH_H
#define VECMATH_H

typedef struct vec2  { float x, y; }    vec2;
typedef struct vec3  { float x, y, z; } vec3;
typedef struct vec3I { int   x, y, z; } vec3I;

void mult4x4mat(float m1[16], float m2[16], float *outputLocation);
void multVec4mat(float mat[16], float vec[4], float *output);

void translate4x4matrix(float inMatrix[16], float xTrans, float yTrans, float zTrans);

void rotate4x4matrix(float inMatrix[16], float xRotDeg, float yRotDeg, float zRotDeg);
void rotateVec4(float vec[4], float xRot, float yRot, float zRot);

void project4x4matrix(float mat[16], float fovDeg, float near, float far);

#endif
