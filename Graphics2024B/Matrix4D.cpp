
#include "Matrix4D.h"
#include <math.h>
MATRIX4D Zero()
{
	MATRIX4D R;
	for (float& x : R.v)
		x = 0.0f;
	return R;
}
MATRIX4D Identity()
{
	MATRIX4D I;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			I.m[j][i] = j == i ? 1.0f : 0.0f;
	return I;
}
MATRIX4D Translation(float dx, float dy, float dz)
{
	MATRIX4D T = Identity();
	T.m30 = dx; T.m31 = dy; T.m32 = dz;
	return T;
}
MATRIX4D Scaling(float sx, float sy, float sz)
{
	MATRIX4D S = Identity();
	S.m00 = sx; S.m11 = sy; S.m22 = sz;
	return S;
}
MATRIX4D RotationZ(float theta)
{
	MATRIX4D R = Identity();
	R.m00 = cosf(theta);
	R.m01 = sinf(theta);
	R.m10 = -sinf(theta);
	R.m11 = cosf(theta);
	return R;
}
MATRIX4D RotationX(float theta)
{
	MATRIX4D R = Identity();
	R.m11 = cosf(theta);
	R.m12 = sinf(theta);
	R.m21 = -sinf(theta);
	R.m22 = cosf(theta);
	return R;
}
MATRIX4D RotationY(float theta)
{
	MATRIX4D R = Identity();
	R.m00 = cosf(theta);
	R.m22 = cosf(theta);
	R.m02 = -sinf(theta);
	R.m20 = sinf(theta);
	return R;
}

MATRIX4D operator*(const MATRIX4D& A, const MATRIX4D& B)
{
	MATRIX4D R = Zero();
	for (int j = 0; j < 4; j++)
		for (int i = 0; i < 4; i++)
			for (int k = 0; k < 4; k++)
				R.m[j][i] += A.m[j][k] * B.m[k][i];
	return R;
}
VECTOR4D operator*(const VECTOR4D& V,const MATRIX4D& M)
{
	VECTOR4D R = { 0,0,0,0 };
	for (int j = 0; j < 4; j++)
		for (int i = 0; i < 4; i++)
			R.v[j] += V.v[i] * M.m[i][j];
	return R;
}
MATRIX4D Transpose(const MATRIX4D& M)
{
	MATRIX4D T;
	for (int j = 0; j < 4; j++)
		for (int i = 0; i < 4; i++)
			T.m[i][j] = M.m[j][i];
	return T;
}

float Dot(const VECTOR4D& A,const VECTOR4D& B)
{
	return A.x*B.x + A.y*B.y + A.z*B.z + A.w*B.w;
}
VECTOR4D Cross3(const VECTOR4D& A,const VECTOR4D& B)
{
	return { 
		A.y*B.z - A.z*B.y,
		A.z*B.x - A.x*B.z,
		A.x*B.y - A.y*B.x,0 };
}
VECTOR4D Normalize(const VECTOR4D& A)
{
	float inv = 1.0f / sqrtf(Dot(A, A));
	return { A.x*inv,A.y*inv,A.z*inv,A.w*inv };
}
MATRIX4D ViewMatrix(const VECTOR4D& Eye,const VECTOR4D& Target,const  VECTOR4D& Up)
{
	VECTOR4D U, V, N;
	N.x = Target.x - Eye.x;
	N.y = Target.y - Eye.y;
	N.z = Target.z - Eye.z;
	N.w = 0;
	N = Normalize(N);
	U = Cross3(Up,N);
	U = Normalize(U);
	V = Cross3(N,U);
	MATRIX4D View = {
		U.x, V.x, N.x,0,
		U.y, V.y, N.y,0,
		U.z, V.z, N.z,0,
		-Dot(U,Eye),-Dot(V,Eye),-Dot(N,Eye),1 };
	return View;
}
MATRIX4D PerspectiveMatrix(float a, float b, float zn, float zf)
{
	float Q = zf / (zf - zn);
	MATRIX4D P = {
		a,0,  0,  0,
		0,b,  0,  0,
		0,0,  Q,  1,
		0,0,-Q*zn,0 };
	return P;
}
MATRIX4D PerspectiveFOVMatrix(float theta, float zn, float zf)
{
	return PerspectiveMatrix(1.0f / tanf(theta/2), 1.0f / tanf(theta/2), zn, zf);
}
MATRIX4D IsometricMatrix(float a, float b, float zn, float zf)
{
	float Q = 1.0f / (zf - zn);
	MATRIX4D P = {
		a,0,  0,  0,
		0,b,  0,  0,
		0,0,  Q,  0,
		0,0,-Q*zn,1 };
	return P;
}
MATRIX4D ViewInverse(const MATRIX4D& View)
{
	MATRIX4D I = Transpose(View);
	VECTOR4D InvPos = { View.m30,View.m31,View.m32,1.0f };
	VECTOR4D Col0 = { I.m00,I.m10,I.m20,0 };
	VECTOR4D Col1 = { I.m01,I.m11,I.m21,0 };
	VECTOR4D Col2 = { I.m02,I.m12,I.m22,0 };
	VECTOR4D Pos = { -Dot(Col0,InvPos),-Dot(Col1,InvPos),-Dot(Col2,InvPos),1 };
	I.m30 = Pos.x;
	I.m31 = Pos.y;
	I.m32 = Pos.z;
	I.m03 = 0;
	I.m13 = 0;
	I.m23 = 0;
	return I;
}

MATRIX4D ScaleAxis(const float s,const VECTOR4D& vAxis)
{
	MATRIX4D S;
	float _s_1 = s - 1.0f;

	S.m00 = vAxis.x*vAxis.x*_s_1 + 1.0f;
	S.m01 = vAxis.x*vAxis.y*_s_1;
	S.m02 = vAxis.x*vAxis.z*_s_1;
	S.m03 = 0;

	S.m10 = vAxis.y*vAxis.x*_s_1;
	S.m11 = vAxis.y*vAxis.y*_s_1 + 1.0f;
	S.m12 = vAxis.y*vAxis.z*_s_1;
	S.m13 = 0;

	S.m20 = vAxis.z*vAxis.x*_s_1;
	S.m21 = vAxis.z*vAxis.y*_s_1;
	S.m22 = vAxis.z*vAxis.z*_s_1 + 1.0f;
	S.m23 = 0;

	S.m30 = 0;
	S.m31 = 0;
	S.m32 = 0;
	S.m33 = 1;
	return S;
}

MATRIX4D Reflection(const VECTOR4D& Plane)
{
	MATRIX4D Reflect;
	Reflect = Identity();
	Reflect.m00 = 1 - 2 * Plane.x*Plane.x;
	Reflect.m01 = -2 * Plane.x*Plane.y;
	Reflect.m02 = -2 * Plane.x*Plane.z;

	Reflect.m10 = -2 * Plane.y*Plane.x;
	Reflect.m11 = 1 - 2 * Plane.y*Plane.y;
	Reflect.m12 = -2 * Plane.y*Plane.z;

	Reflect.m20 = -2 * Plane.z*Plane.x;
	Reflect.m21 = -2 * Plane.z*Plane.y;
	Reflect.m22 = 1 - 2 * Plane.z*Plane.z;

	Reflect.m30 = -2 * Plane.w*Plane.x;
	Reflect.m31 = -2 * Plane.w*Plane.y;
	Reflect.m32 = -2 * Plane.w*Plane.z;
	return Reflect;
}

MATRIX4D RotationAxis(const float theta,const VECTOR4D& vAxis)
{
	MATRIX4D R;
	float _cos = cosf(theta);
	float _1_cos = 1.0f - _cos;
	float _sin = sinf(theta);

	R.m00 = vAxis.x*vAxis.x*_1_cos + _cos;
	R.m01 = vAxis.x*vAxis.y*_1_cos + vAxis.z*_sin;
	R.m02 = vAxis.x*vAxis.z*_1_cos - vAxis.y*_sin;
	R.m03 = 0;

	R.m10 = vAxis.y*vAxis.x*_1_cos - vAxis.z*_sin;
	R.m11 = vAxis.y*vAxis.y*_1_cos + _cos;
	R.m12 = vAxis.y*vAxis.z*_1_cos + vAxis.x*_sin;
	R.m13 = 0;

	R.m20 = vAxis.z*vAxis.x*_1_cos + vAxis.y*_sin;
	R.m21 = vAxis.z*vAxis.y*_1_cos - vAxis.x*_sin;
	R.m22 = vAxis.z*vAxis.z*_1_cos + _cos;
	R.m23 = 0;

	R.m30 = 0;
	R.m31 = 0;
	R.m32 = 0;
	R.m33 = 1;
	return R;
}
float Inverse(const MATRIX4D& M, MATRIX4D& R)
{
	double inv[16], det;
	int i;

	inv[0] = M.v[5] * M.v[10] * M.v[15] -
		M.v[5] * M.v[11] * M.v[14] -
		M.v[9] * M.v[6] * M.v[15] +
		M.v[9] * M.v[7] * M.v[14] +
		M.v[13] * M.v[6] * M.v[11] -
		M.v[13] * M.v[7] * M.v[10];

	inv[4] = -M.v[4] * M.v[10] * M.v[15] +
		M.v[4] * M.v[11] * M.v[14] +
		M.v[8] * M.v[6] * M.v[15] -
		M.v[8] * M.v[7] * M.v[14] -
		M.v[12] * M.v[6] * M.v[11] +
		M.v[12] * M.v[7] * M.v[10];

	inv[8] = M.v[4] * M.v[9] * M.v[15] -
		M.v[4] * M.v[11] * M.v[13] -
		M.v[8] * M.v[5] * M.v[15] +
		M.v[8] * M.v[7] * M.v[13] +
		M.v[12] * M.v[5] * M.v[11] -
		M.v[12] * M.v[7] * M.v[9];

	inv[12] = -M.v[4] * M.v[9] * M.v[14] +
		M.v[4] * M.v[10] * M.v[13] +
		M.v[8] * M.v[5] * M.v[14] -
		M.v[8] * M.v[6] * M.v[13] -
		M.v[12] * M.v[5] * M.v[10] +
		M.v[12] * M.v[6] * M.v[9];

	inv[1] = -M.v[1] * M.v[10] * M.v[15] +
		M.v[1] * M.v[11] * M.v[14] +
		M.v[9] * M.v[2] * M.v[15] -
		M.v[9] * M.v[3] * M.v[14] -
		M.v[13] * M.v[2] * M.v[11] +
		M.v[13] * M.v[3] * M.v[10];

	inv[5] = M.v[0] * M.v[10] * M.v[15] -
		M.v[0] * M.v[11] * M.v[14] -
		M.v[8] * M.v[2] * M.v[15] +
		M.v[8] * M.v[3] * M.v[14] +
		M.v[12] * M.v[2] * M.v[11] -
		M.v[12] * M.v[3] * M.v[10];

	inv[9] = -M.v[0] * M.v[9] * M.v[15] +
		M.v[0] * M.v[11] * M.v[13] +
		M.v[8] * M.v[1] * M.v[15] -
		M.v[8] * M.v[3] * M.v[13] -
		M.v[12] * M.v[1] * M.v[11] +
		M.v[12] * M.v[3] * M.v[9];

	inv[13] = M.v[0] * M.v[9] * M.v[14] -
		M.v[0] * M.v[10] * M.v[13] -
		M.v[8] * M.v[1] * M.v[14] +
		M.v[8] * M.v[2] * M.v[13] +
		M.v[12] * M.v[1] * M.v[10] -
		M.v[12] * M.v[2] * M.v[9];

	inv[2] = M.v[1] * M.v[6] * M.v[15] -
		M.v[1] * M.v[7] * M.v[14] -
		M.v[5] * M.v[2] * M.v[15] +
		M.v[5] * M.v[3] * M.v[14] +
		M.v[13] * M.v[2] * M.v[7] -
		M.v[13] * M.v[3] * M.v[6];

	inv[6] = -M.v[0] * M.v[6] * M.v[15] +
		M.v[0] * M.v[7] * M.v[14] +
		M.v[4] * M.v[2] * M.v[15] -
		M.v[4] * M.v[3] * M.v[14] -
		M.v[12] * M.v[2] * M.v[7] +
		M.v[12] * M.v[3] * M.v[6];

	inv[10] = M.v[0] * M.v[5] * M.v[15] -
		M.v[0] * M.v[7] * M.v[13] -
		M.v[4] * M.v[1] * M.v[15] +
		M.v[4] * M.v[3] * M.v[13] +
		M.v[12] * M.v[1] * M.v[7] -
		M.v[12] * M.v[3] * M.v[5];

	inv[14] = -M.v[0] * M.v[5] * M.v[14] +
		M.v[0] * M.v[6] * M.v[13] +
		M.v[4] * M.v[1] * M.v[14] -
		M.v[4] * M.v[2] * M.v[13] -
		M.v[12] * M.v[1] * M.v[6] +
		M.v[12] * M.v[2] * M.v[5];

	inv[3] = -M.v[1] * M.v[6] * M.v[11] +
		M.v[1] * M.v[7] * M.v[10] +
		M.v[5] * M.v[2] * M.v[11] -
		M.v[5] * M.v[3] * M.v[10] -
		M.v[9] * M.v[2] * M.v[7] +
		M.v[9] * M.v[3] * M.v[6];

	inv[7] = M.v[0] * M.v[6] * M.v[11] -
		M.v[0] * M.v[7] * M.v[10] -
		M.v[4] * M.v[2] * M.v[11] +
		M.v[4] * M.v[3] * M.v[10] +
		M.v[8] * M.v[2] * M.v[7] -
		M.v[8] * M.v[3] * M.v[6];

	inv[11] = -M.v[0] * M.v[5] * M.v[11] +
		M.v[0] * M.v[7] * M.v[9] +
		M.v[4] * M.v[1] * M.v[11] -
		M.v[4] * M.v[3] * M.v[9] -
		M.v[8] * M.v[1] * M.v[7] +
		M.v[8] * M.v[3] * M.v[5];

	inv[15] = M.v[0] * M.v[5] * M.v[10] -
		M.v[0] * M.v[6] * M.v[9] -
		M.v[4] * M.v[1] * M.v[10] +
		M.v[4] * M.v[2] * M.v[9] +
		M.v[8] * M.v[1] * M.v[6] -
		M.v[8] * M.v[2] * M.v[5];

	det = M.v[0] * inv[0] + M.v[1] * inv[4] + M.v[2] * inv[8] + M.v[3] * inv[12];

	if (det == 0)
		return 0.0f;

	double invdet = 1.0 / det;

	for (i = 0; i < 16; i++)
		R.v[i] = (float)(inv[i] * invdet);

	return (float)det;
}

MATRIX4D Orthogonalize(const MATRIX4D& M)
{
	MATRIX4D R = M;
	VECTOR4D XAxis;
	VECTOR4D ZAxis = { M.m20, M.m21, M.m22, 0 };
	VECTOR4D YAxis = { M.m10, M.m11, M.m12, 0 };
	ZAxis=Normalize(ZAxis);
	XAxis=Cross3(YAxis, ZAxis);
	XAxis=Normalize(XAxis);
	YAxis=Cross3(ZAxis, XAxis);
	R.m00 = XAxis.x;
	R.m01 = XAxis.y;
	R.m02 = XAxis.z;
	R.m10 = YAxis.x;
	R.m11 = YAxis.y;
	R.m12 = YAxis.z;
	R.m20 = ZAxis.x;
	R.m21 = ZAxis.y;
	R.m22 = ZAxis.z;
	return R;
}

VECTOR4D Lerp(const VECTOR4D& A,const VECTOR4D& B, const float u) //Linear interpolation 
{
	//Interpolación lineal  X'=A+(B-A)*u
	return {
			A.x + (B.x - A.x)*u,
			A.y + (B.y - A.y)*u,
			A.z + (B.z - A.z)*u,
			A.w + (B.w - A.w)*u };
}

VECTOR4D operator+(const VECTOR4D& A, const VECTOR4D& B)
{
	return { A.x + B.x, A.y + B.y, A.z + B.z, A.w + B.w };
}

VECTOR4D operator-(const VECTOR4D& A, const VECTOR4D& B)
{
	return { A.x - B.x, A.y - B.y, A.z - B.z, A.w - B.w };
}