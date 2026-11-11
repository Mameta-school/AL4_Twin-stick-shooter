#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

class Player {
public:
	void Initialize(Model* model, const Vector3& position);
	void Update(const Vector3& aimPos);
	void Draw(const Camera& camera);

	const Vector3& GetPosition() const { return worldTransform_.translation_; }
	const Vector3& GetAimDirection() const { return aimDirection_; }

private:
	static constexpr float kMoveSpeed = 0.2f;	// 移動速度

	Model* model_ = nullptr;
	WorldTransform worldTransform_;
	Vector3 aimDirection_ = { 0.0f, 0.0f, 1.0f };	// 狙う方向
};