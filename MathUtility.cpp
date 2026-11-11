#include "MathUtility.h"
#include <cmath>
#include <cstdlib>

Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = {};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				result.m[i][j] += m1.m[i][k] * m2.m[k][j];
			}
		}
	}
	return result;
}

Matrix4x4 MakeRotateXMatrix(float angle) {
	const float c = std::cos(angle);
	const float s = std::sin(angle);
	return {
		1, 0,  0, 0,
		0, c,  s, 0,
		0, -s, c, 0,
		0, 0,  0, 1
	};
}

Matrix4x4 MakeRotateYMatrix(float angle) {
	const float c = std::cos(angle);
	const float s = std::sin(angle);
	return {
		c, 0, -s, 0,
		0, 1, 0,  0,
		s, 0, c,  0,
		0, 0, 0,  1
	};
}

Matrix4x4 MakeRotateZMatrix(float angle) {
	const float c = std::cos(angle);
	const float s = std::sin(angle);
	return {
		c,  s, 0, 0,
		-s, c, 0, 0,
		0,  0, 1, 0,
		0,  0, 0, 1
	};
}

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotation, const Vector3& translation) {
	const Matrix4x4 matScale = {
		scale.x, 0, 0, 0,
		0, scale.y, 0, 0,
		0, 0, scale.z, 0,
		0, 0, 0, 1
	};
	const Matrix4x4 matTrans = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		translation.x, translation.y, translation.z, 1
	};

	Matrix4x4 result = Multiply(matScale, MakeRotateXMatrix(rotation.x));
	result = Multiply(result, MakeRotateYMatrix(rotation.y));
	result = Multiply(result, MakeRotateZMatrix(rotation.z));
	return Multiply(result, matTrans);
}

Vector3 Transform(const Vector3& v, const Matrix4x4& m) {
	return {
		v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + m.m[3][0],
		v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + m.m[3][1],
		v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + m.m[3][2]
	};
}

bool IsCollision(const AABB& a, const AABB& b) {
	return a.min.x <= b.max.x && a.max.x >= b.min.x &&
		a.min.y <= b.max.y && a.max.y >= b.min.y &&
		a.min.z <= b.max.z && a.max.z >= b.min.z;
}

float Lerp(float a, float b, float t) {
	return a + (b - a) * t;
}

Vector3 Lerp(const Vector3& a, const Vector3& b, float t) {
	return { Lerp(a.x, b.x, t), Lerp(a.y, b.y, t), Lerp(a.z, b.z, t) };
}

float RandomSigned() {
	return rand() / static_cast<float>(RAND_MAX) * 2.0f - 1.0f;
}