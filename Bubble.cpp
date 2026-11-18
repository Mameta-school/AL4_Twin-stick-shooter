#include "Bubble.h"
#include "MathUtility.h"
#include "WorldMatrix.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

void Bubble::Initialize(Model* model, const Vector3& position, const Vector3& direction) {
	model_ = model;
	direction_ = direction;
	radius_ = kRadius;
	// 泡ごとに揺れ方をずらす
	phase_ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * kPi;

	worldTransform_.Initialize();
	color_.Initialize();
	color_.SetColor({ 0.5f, 0.8f, 1.0f, 0.5f }); // 水色・半透明
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = { radius_, radius_, radius_ };
	WorldMatrix::UpdateWorldMatrix(worldTransform_);
}

void Bubble::Update() {
	// 減速
	speed_ = max(speed_ * kDeceleration, kMinSpeed);

	// 進行方向 + 進行方向に垂直な向きへのゆらゆら
	phase_ += kSwayFrequency;
	const Vector3 perpendicular = { direction_.z, 0.0f, -direction_.x };
	const float sway = std::cos(phase_) * kSwayAmplitude;
	worldTransform_.translation_.x += direction_.x * speed_ + perpendicular.x * sway;
	worldTransform_.translation_.z += direction_.z * speed_ + perpendicular.z * sway;

	// 泡っぽく少しだけ膨らんだり縮んだりする
	const float pulse = radius_ * (1.0f + 0.08f * std::sin(phase_ * 2.0f));
	worldTransform_.scale_ = { pulse, pulse, pulse };

	if (mergeCooldown_ > 0) {
		mergeCooldown_--;
	}

	if (--life_ <= 0) {
		isDead_ = true;
	}
	WorldMatrix::UpdateWorldMatrix(worldTransform_);
}

void Bubble::Draw(const Camera& camera) { model_->Draw(worldTransform_, camera, &color_); }

bool Bubble::CanMerge(const Bubble& other) const {
	if (isDead_ || other.isDead_) {
		return false;
	}
	// ばらまかれた直後は合体しない
	if (mergeCooldown_ > 0 || other.mergeCooldown_ > 0) {
		return false;
	}

	// 合体後の半径(面積を足す)が上限を超えるなら合体しない
	const float mergedRadius = std::sqrt(radius_ * radius_ + other.radius_ * other.radius_);
	if (mergedRadius > kMaxRadius) {
		return false;
	}

	// 十分に重なっているか
	const float dx = worldTransform_.translation_.x - other.worldTransform_.translation_.x;
	const float dz = worldTransform_.translation_.z - other.worldTransform_.translation_.z;
	const float r = (radius_ + other.radius_) * kMergeRatio;
	return dx * dx + dz * dz <= r * r;
}

void Bubble::Merge(Bubble& other) {
	// 面積を重みにして混ぜる(大きい泡のほうに引っ張られる)
	const float weightA = radius_ * radius_;
	const float weightB = other.radius_ * other.radius_;
	const float total = weightA + weightB;

	// 位置
	worldTransform_.translation_.x = (worldTransform_.translation_.x * weightA + other.worldTransform_.translation_.x * weightB) / total;
	worldTransform_.translation_.z = (worldTransform_.translation_.z * weightA + other.worldTransform_.translation_.z * weightB) / total;

	// 進む勢い(向き×速さ)
	const float vx = (direction_.x * speed_ * weightA + other.direction_.x * other.speed_ * weightB) / total;
	const float vz = (direction_.z * speed_ * weightA + other.direction_.z * other.speed_ * weightB) / total;
	const float newSpeed = std::sqrt(vx * vx + vz * vz);
	if (newSpeed > 0.0001f) { // 打ち消し合って止まったときは向きを変えない
		direction_ = { vx / newSpeed, 0.0f, vz / newSpeed };
		speed_ = newSpeed;
	}

	// 大きさと寿命
	radius_ = std::sqrt(total);
	life_ = max(life_, other.life_);

	// 取り込まれた側は消える
	other.isDead_ = true;
}