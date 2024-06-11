#ifndef VECMATH_H
#define VECMATH_H

typedef struct vec2  { float x, y; }    vec2;
typedef struct vec3  { float x, y, z; } vec3;
typedef struct vec3I { int   x, y, z; } vec3I;

void mult4x4mat(float m1[16], float m2[16], float *outputLocation)
{
	float result[16] =
	{
		(m1[0]*m2[0])  + (m1[1]*m2[4])  + (m1[2]*m2[8])   + (m1[3]*m2[12]),   (m1[0]*m2[1])  + (m1[1]*m2[5])  + (m1[2]*m2[9])   + (m1[3]*m2[13]),
		(m1[0]*m2[2])  + (m1[1]*m2[6])  + (m1[2]*m2[10])  + (m1[3]*m2[14]),   (m1[0]*m2[3])  + (m1[1]*m2[7])  + (m1[2]*m2[11])  + (m1[3]*m2[15]), 
	
		(m1[4]*m2[0])  + (m1[5]*m2[4])  + (m1[6]*m2[8])   + (m1[7]*m2[12]),   (m1[4]*m2[1])  + (m1[5]*m2[5])  + (m1[6]*m2[9])   + (m1[7]*m2[13]),
		(m1[4]*m2[2])  + (m1[5]*m2[6])  + (m1[6]*m2[10])  + (m1[7]*m2[14]),   (m1[4]*m2[3])  + (m1[5]*m2[7])  + (m1[6]*m2[11])  + (m1[7]*m2[15]), 
	
		(m1[8]*m2[0])  + (m1[9]*m2[4])  + (m1[10]*m2[8])  + (m1[11]*m2[12]),  (m1[8]*m2[1])  + (m1[9]*m2[5])  + (m1[10]*m2[9])  + (m1[11]*m2[13]),
		(m1[8]*m2[2])  + (m1[9]*m2[6])  + (m1[10]*m2[10]) + (m1[11]*m2[14]),  (m1[8]*m2[3])  + (m1[9]*m2[7])  + (m1[10]*m2[11]) + (m1[11]*m2[15]), 
	
		(m1[12]*m2[0]) + (m1[13]*m2[4]) + (m1[14]*m2[8])  + (m1[15]*m2[12]),  (m1[12]*m2[1]) + (m1[13]*m2[5]) + (m1[14]*m2[9])  + (m1[15]*m2[13]),
		(m1[12]*m2[2]) + (m1[13]*m2[6]) + (m1[14]*m2[10]) + (m1[15]*m2[14]),  (m1[12]*m2[3]) + (m1[13]*m2[7]) + (m1[14]*m2[11]) + (m1[15]*m2[15]), 
	};

	for(int i = 0; i < 16; i++)
	{
		outputLocation[i] = result[i];
	}
}

void multVec4mat(float mat[16], float vec[4], float *output)
{
	float result[4] = {
		(vec[0]*mat[0])  + (vec[1]*mat[4])  + (vec[2]*mat[8])  + (vec[3]*mat[12]),
		(vec[0]*mat[1])  + (vec[1]*mat[5])  + (vec[2]*mat[9])  + (vec[3]*mat[13]),
		(vec[0]*mat[2])  + (vec[1]*mat[6])  + (vec[2]*mat[10]) + (vec[3]*mat[14]),
		(vec[0]*mat[3])  + (vec[1]*mat[7])  + (vec[2]*mat[12]) + (vec[3]*mat[15]) 
	};

	for(int i = 0; i < 4; i++)
	{
		output[i] = result[i];
	}
}

void translate4x4matrix(float inMatrix[16], float xTrans, float yTrans, float zTrans)
{
	float translationMatrix[16] = {
	    1.0f, 0.0f, 0.0f, xTrans,
	    0.0f, 1.0f, 0.0f, yTrans,
	    0.0f, 0.0f, 1.0f, zTrans,
	    0.0f, 0.0f, 0.0f, 1.0f
	};

	float outMatrix[16];

	mult4x4mat(translationMatrix, inMatrix, outMatrix);
	for(int i = 0; i < 16; i++)
	{
		inMatrix[i] = outMatrix[i];
	}
}

void rotate4x4matrix(float inMatrix[16], float xRotDeg, float yRotDeg, float zRotDeg)
{
	float xRot = xRotDeg*3.141592f/180.0f;
	float yRot = yRotDeg*3.141592f/180.0f;
	float zRot = zRotDeg*3.141592f/180.0f;

	float xRotMat[16] = {
	    1.0f, 0.0f,       0.0f,      0.0f,
	    0.0f, cos(xRot), -sin(xRot), 0.0f,
	    0.0f, sin(xRot),  cos(xRot), 0.0f,
	    0.0f, 0.0f,       0.0f,      1.0f
	};

	float yRotMat[16] = {
	    cos(yRot), 0.0f, sin(yRot), 0.0f,
	    0.0f,      1.0f, 0.0f,      0.0f,
	   -sin(yRot), 0.0f, cos(yRot), 0.0f,
	    0.0f,      0.0f, 0.0f,      1.0f
	};
	
	float zRotMat[16] = {
	    cos(zRot), -sin(zRot), 0.0f, 0.0f,
	    sin(zRot),  cos(zRot), 0.0f, 0.0f,
	    0.0f,       0.0f,      1.0f, 0.0f,
	    0.0f,       0.0f,      0.0f, 1.0f,
	};

	float outMatrix[16];
	mult4x4mat(xRotMat, yRotMat, yRotMat);
	mult4x4mat(yRotMat, zRotMat, zRotMat);
	mult4x4mat(zRotMat, inMatrix, outMatrix);

	for(int i = 0; i < 16; i++)
	{
		inMatrix[i] = outMatrix[i];
	}
}

void rotateVec4(float vec[4], float xRot, float yRot, float zRot)
{
	float rotMat[16] = {
	    1.0f, 0.0f, 0.0f, 0.0f,
	    0.0f, 1.0f, 0.0f, 0.0f,
	    0.0f, 0.0f, 1.0f, 0.0f,
	    0.0f, 0.0f, 0.0f, 1.0f
	};

	rotate4x4matrix(rotMat, xRot, yRot, zRot);
	
	float outVec[4];
	multVec4mat(rotMat, vec, outVec);

	for(int i = 0; i < 4; i++)
	{
		vec[i] = outVec[i];
	}
}

void project4x4matrix(float mat[16], float fovDeg, float near, float far)
{
	float fovScale = 1.0f/(tan(fovDeg*3.141592f/360));
	
	float proj[16] = {
	    fovScale, 0.0f,      0.0f,                      0.0f,
	    0.0f,     fovScale,  0.0f,                      0.0f,
	    0.0f,     0.0f,     -(far/(far-near)),         -1.0f,
	    0.0f,     0.0f,     -((far*near)/(far-near)),   0.0f
	};

	mult4x4mat(mat, proj, mat);
}

#endif
