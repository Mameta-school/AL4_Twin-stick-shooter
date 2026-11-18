#include "PopBullet.h"
#include "WorldMatrix.h"
#include <cmath>

void PopBullet::Initialize(Model* model, const Vector3& position, const Vector3& direction) {
	model_ = model;
	direction_ = direction;

	worldTransform_.Initialize();
	color_.Initialize();
	color_.SetColor({ 1.0f, 0.9f, 0.2f, 1.0f }); // 黄
	worldTransform_.translation_ = position;
	// 進行方向(Z軸)に細長いキューブにして、向きを進む方向へ回す
	worldTransform_.scale_ = { 0.15f, 0.15f, 0.6f };
	worldTransform_.rotation_.y = std::atan2(direction.x, direction.z);
	WorldMatrix::UpdateWorldMatrix(worldTransform_);
}

void PopBullet::Update() {
	worldTransform_.translation_.x += direction_.x * kSpeed;
	worldTransform_.translation_.z += direction_.z * kSpeed;

	if (--life_ <= 0) {
		isDead_ = true;
	}
	WorldMatrix::UpdateWorldMatrix(worldTransform_);
}

void PopBullet::Draw(const Camera& camera) { model_->Draw(worldTransform_, camera, &color_); }