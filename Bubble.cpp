#include "Bubble.h"
#include "WorldMatrix.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

void Bubble::Initialize(Model* model, const Vector3& position, const Vector3& direction) {
	model_ = model;
	direction_ = direction;
	// 泡ごとに揺れ方をずらす
	phase_ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 6.2831853f;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = { kRadius, kRadius, kRadius };
	WorldMatrix::UpdateWorldMatrix(worldTransform_);
}

void Bubble::Update() {
	// 減速
	speed_ = max(speed_ * kDeceleration, kMinSpeed);

	// 進行方向
	phase_ += kSwayFrequency;
	const Vector3 perpendicular = { direction_.z, 0.0f, -direction_.x };
	const float sway = std::cos(phase_) * kSwayAmplitude;
	worldTransform_.translation_.x += direction_.x * speed_ + perpendicular.x * sway;
	worldTransform_.translation_.z += direction_.z * speed_ + perpendicular.z * sway;

	// 泡っぽく少しだけ膨らんだり縮んだりする
	const float pulse = kRadius * (1.0f + 0.08f * std::sin(phase_ * 2.0f));
	worldTransform_.scale_ = { pulse, pulse, pulse };

	if (--life_ <= 0) {
		isDead_ = true;
	}
	WorldMatrix::UpdateWorldMatrix(worldTransform_);
}

void Bubble::Draw(const Camera& camera) { model_->Draw(worldTransform_, camera); }