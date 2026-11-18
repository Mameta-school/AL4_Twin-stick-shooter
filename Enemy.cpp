#include "Enemy.h"
#include "WorldMatrix.h"
#include <cmath>

void Enemy::Initialize(Model* model, const Vector3& position) {
	model_ = model;
	worldTransform_.Initialize();
	color_.Initialize();
	color_.SetColor({ 0.9f, 0.25f, 0.25f, 1.0f }); // 赤
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = { kRadius, kRadius, kRadius };
	WorldMatrix::UpdateWorldMatrix(worldTransform_);
}

void Enemy::SetPosition(const Vector3& position) {
	worldTransform_.translation_ = position;
	WorldMatrix::UpdateWorldMatrix(worldTransform_);
}

void Enemy::Update(const Vector3& targetPos) {
	if (isCaptured_) {
		return;
	}

	// プレイヤーに向かって歩く
	const float dx = targetPos.x - worldTransform_.translation_.x;
	const float dz = targetPos.z - worldTransform_.translation_.z;
	const float length = std::sqrt(dx * dx + dz * dz);
	if (length > 0.001f) {
		worldTransform_.translation_.x += dx / length * kMoveSpeed;
		worldTransform_.translation_.z += dz / length * kMoveSpeed;
	}

	WorldMatrix::UpdateWorldMatrix(worldTransform_);
}

void Enemy::Draw(const Camera& camera) {
	// 泡に入っていても描く(泡は半透明なので、中の敵が見える)
	model_->Draw(worldTransform_, camera, &color_);
}