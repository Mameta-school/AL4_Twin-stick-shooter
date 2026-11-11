#include "Player.h"
#include "WorldMatrix.h"
#include <cmath>

void Player::Initialize(Model* model, const Vector3& position) {
	model_ = model;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	WorldMatrix::UpdateWorldMatrix(worldTransform_);
}

void Player::Update(const Vector3& aimPos) {
	Input* input = Input::GetInstance();

	// --- 移動 ---
	Vector3 move = { 0.0f, 0.0f, 0.0f };
	if (input->PushKey(DIK_W)) {
		move.z += 1.0f;
	}
	if (input->PushKey(DIK_S)) {
		move.z -= 1.0f;
	}
	if (input->PushKey(DIK_D)) {
		move.x += 1.0f;
	}
	if (input->PushKey(DIK_A)) {
		move.x -= 1.0f;
	}

	const float length = std::sqrt(move.x * move.x + move.z * move.z);
	if (length > 0.0f) {
		worldTransform_.translation_.x += move.x / length * kMoveSpeed;
		worldTransform_.translation_.z += move.z / length * kMoveSpeed;
	}

	// --- エイム ---
	const float dx = aimPos.x - worldTransform_.translation_.x;
	const float dz = aimPos.z - worldTransform_.translation_.z;
	const float aimLength = std::sqrt(dx * dx + dz * dz);
	if (aimLength > 0.001f) {
		aimDirection_ = { dx / aimLength, 0.0f, dz / aimLength };
		worldTransform_.rotation_.y = std::atan2(aimDirection_.x, aimDirection_.z);
	}

	WorldMatrix::UpdateWorldMatrix(worldTransform_);
}

void Player::Draw(const Camera& camera) { 
	model_->Draw(worldTransform_, camera); 
}