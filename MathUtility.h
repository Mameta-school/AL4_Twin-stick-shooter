#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

// 円周率
inline constexpr float kPi = 3.14159265f;
// ラジアンに変換
inline constexpr float ToRadians(float degrees) { return degrees * kPi / 180.0f; }

struct AABB {
	Vector3 min;
	Vector3 max;
};

// 行列の積
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

// 回転行列
Matrix4x4 MakeRotateXMatrix(float angle);
Matrix4x4 MakeRotateYMatrix(float angle);
Matrix4x4 MakeRotateZMatrix(float angle);

// アフィン変換行列
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotation, const Vector3& translation);

// 点の変換
Vector3 Transform(const Vector3& v, const Matrix4x4& m);

// AABB同士の当たり判定
bool IsCollision(const AABB& a, const AABB& b);

// 線形補間
float Lerp(float a, float b, float t);
Vector3 Lerp(const Vector3& a, const Vector3& b, float t);