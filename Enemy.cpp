#include "Enemy.h"
#include "WorldMatrix.h"
#include <cmath>

void Enemy::Initialize(Model* model, const Vector3& position) {
	model_ = model;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = { kRadius, kRadius, kRadius };
	WorldMatrix::UpdateWorldMatrix(worldTransform_);

	shellTransform_.Initialize();
	shellTransform_.scale_ = { kTrappedRadius, kTrappedRadius, kTrappedRadius };
	WorldMatrix::UpdateWorldMatrix(shellTransform_);
}

void Enemy::Trap() {
	isTrapped_ = true;
	trapTimer_ = kTrapFrames;
	floatPhase_ = 0.0f;
}

void Enemy::Update(const Vector3& targetPos) {
	if (isTrapped_) {
		// 閉じ込め中
		floatPhase_ += 0.1f;
		worldTransform_.translation_.y = std::sin(floatPhase_) * kFloatHeight + kFloatHeight;

		// 時間で泡から脱出
		trapTimer_--;
		if (trapTimer_ <= 0) {
			isTrapped_ = false;
			worldTransform_.translation_.y = 0.0f;
		}
	}
	else {
		// プレイヤーに向かって歩く
		const float dx = targetPos.x - worldTransform_.translation_.x;
		const float dz = targetPos.z - worldTransform_.translation_.z;
		const float length = std::sqrt(dx * dx + dz * dz);
		if (length > 0.001f) {
			worldTransform_.translation_.x += dx / length * kMoveSpeed;
			worldTransform_.translation_.z += dz / length * kMoveSpeed;
		}
	}

	WorldMatrix::UpdateWorldMatrix(worldTransform_);

	// 泡は敵と同じ位置に置く
	shellTransform_.translation_ = worldTransform_.translation_;
	WorldMatrix::UpdateWorldMatrix(shellTransform_);
}

void Enemy::Draw(const Camera& camera) {
	if (isTrapped_) {
		model_->Draw(shellTransform_, camera);
	}
	else {
		model_->Draw(worldTransform_, camera);
	}
}