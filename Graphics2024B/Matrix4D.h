#pragma once


struct VECTOR4D
{
        union
        {
                struct
                {
                        float x, y, z, w;
                };
                struct
                {
                        float r, g, b, a;
                };
                struct
                {
                        long lx, ly, lz, lw;
                };
                float v[4];
                long l[4];
        };
};
struct MATRIX4D
{
        union
        {
                struct
                {
                        float m00, m01, m02, m03;
                        float m10, m11, m12, m13;
                        float m20, m21, m22, m23;
                        float m30, m31, m32, m33;
                };
                float m[4][4];
                float v[16];
                VECTOR4D r[4];
        };
};


MATRIX4D Zero();
MATRIX4D Identity();
MATRIX4D Translation(float dx, float dy, float dz);
MATRIX4D Scaling(float sx, float sy, float sz);
MATRIX4D RotationX(float theta);
MATRIX4D RotationY(float theta);
MATRIX4D RotationZ(float theta);
MATRIX4D Transpose(const MATRIX4D& M);


MATRIX4D operator*(const MATRIX4D& A, const MATRIX4D& B);
VECTOR4D operator*(const VECTOR4D& V, const MATRIX4D& M);
VECTOR4D operator+(const VECTOR4D& A, const VECTOR4D& B);
VECTOR4D operator-(const VECTOR4D& A, const VECTOR4D& B);


float Dot(const VECTOR4D& A,const VECTOR4D& B);
VECTOR4D Cross3(const VECTOR4D& A, const VECTOR4D& B);
VECTOR4D Normalize(const VECTOR4D& A);
MATRIX4D ViewMatrix(const VECTOR4D& Eye, const VECTOR4D& Target,const VECTOR4D& Up);


MATRIX4D PerspectiveMatrix(float a, float b, float zn, float zf);
MATRIX4D PerspectiveFOVMatrix(float theta, float zn, float zf);
MATRIX4D IsometricMatrix(float a, float b, float zn, float zf);


MATRIX4D ViewInverse(const MATRIX4D& View); //Solo para matrices de vista!!!!


MATRIX4D ScaleAxis(const float s,const VECTOR4D& vAxis);
MATRIX4D Reflection(const VECTOR4D& Plane);
MATRIX4D RotationAxis(const float theta, const VECTOR4D& vAxis);
float         Inverse(const MATRIX4D& M, MATRIX4D& R);
MATRIX4D Orthogonalize(const MATRIX4D& M);


VECTOR4D Lerp(const VECTOR4D& A, const VECTOR4D& B, const float u); //Linear interpolation