#include "EnemyBubble.h"
#include "MathUtility.h"
#include "WorldMatrix.h"
#include <algorithm>
#include <cmath>

void EnemyBubble::Initialize(Model* model, const Vector3& position, const Vector3& velocity, int capacity) {
	model_ = model;
	position_ = position;
	velocity_ = velocity;
	capacity_ = max(capacity, 1);
	worldTransform_.Initialize();
	color_.Initialize();
	color_.SetColor({ 0.6f, 1.0f, 0.9f, 0.35f }); // 薄い水緑・半透明
	worldTransform_.translation_ = position;
	timer_ = kTrapFrames;
	UpdateRadius();
}

void EnemyBubble::AddEnemy(Enemy* enemy) {
	enemy->SetCaptured(true);
	enemy->SetPosition(position_);
	enemies_.push_back(enemy);
	UpdateRadius();
}

float EnemyBubble::GetMass() const {
	// 泡そのものの重さ(容量ぶん) + 中の敵の重さの合計
	return static_cast<float>(capacity_) + static_cast<float>(enemies_.size()) * Enemy::kMass;
}

void EnemyBubble::UpdateRadius() {
	// 容量が大きいほど面積が増える(半径は平方根で増える)。上限あり
	radius_ = min(kBaseRadius * std::sqrt(static_cast<float>(capacity_)), kMaxRadius);
}

void EnemyBubble::Update(float floorLimit) {
	// 引き継いだ速度で動いて、だんだん減速する
	position_.x += velocity_.x;
	position_.z += velocity_.z;
	// 重い泡ほど勢いが落ちにくい(重さ4なら、1フレームの減速が半分になる)
	const float deceleration = 1.0f - (1.0f - kDeceleration) / std::sqrt(GetMass());
	velocity_.x *= deceleration;
	velocity_.z *= deceleration;
	if (velocity_.x * velocity_.x + velocity_.z * velocity_.z < kStopSpeed * kStopSpeed) {
		velocity_ = { 0.0f, 0.0f, 0.0f };
	}

	// 床の端を越えたら、そこで止める(その向きの速度は0にする)
	if (position_.x > floorLimit) {
		position_.x = floorLimit;
		velocity_.x = 0.0f;
	}
	else if (position_.x < -floorLimit) {
		position_.x = -floorLimit;
		velocity_.x = 0.0f;
	}
	if (position_.z > floorLimit) {
		position_.z = floorLimit;
		velocity_.z = 0.0f;
	}
	else if (position_.z < -floorLimit) {
		position_.z = -floorLimit;
		velocity_.z = 0.0f;
	}

	// その場でふわふわ浮かぶ
	floatPhase_ += 0.1f;
	worldTransform_.translation_ = { position_.x, std::sin(floatPhase_) * kFloatHeight + kFloatHeight, position_.z };
	worldTransform_.scale_ = { radius_, radius_, radius_ };
	WorldMatrix::UpdateWorldMatrix(worldTransform_);

	// 中の敵は、泡の中に輪になって並べる(ゆっくり回る)
	const int count = static_cast<int>(enemies_.size());
	float ringRadius = 0.0f;
	if (count > 1) {
		ringRadius = radius_ * 0.45f;
	}
	for (int i = 0; i < count; i++) {
		const float angle = 2.0f * kPi * static_cast<float>(i) / static_cast<float>(count) + floatPhase_ * 0.3f;
		enemies_[i]->SetPosition({
			position_.x + std::sin(angle) * ringRadius,
			worldTransform_.translation_.y,
			position_.z + std::cos(angle) * ringRadius });
	}

	// 脱出が近づいたら、赤っぽく点滅して知らせる
	if (timer_ <= kWarningFrames && (timer_ / 6) % 2 == 0) {
		color_.SetColor({ 1.0f, 0.5f, 0.4f, 0.5f });
	}
	else {
		color_.SetColor({ 0.6f, 1.0f, 0.9f, 0.35f });
	}

	// 時間切れで脱出
	timer_--;
	if (timer_ <= 0) {
		ReleaseEnemies();
	}
}

void EnemyBubble::Draw(const Camera& camera) {
	// 半透明の泡。中の敵を先に描いてから、これを描く
	model_->Draw(worldTransform_, camera, &color_);
}

bool EnemyBubble::CanMerge(const EnemyBubble& other) const {
	if (isDead_ || other.isDead_) {
		return false;
	}
	const float dx = position_.x - other.position_.x;
	const float dz = position_.z - other.position_.z;
	const float r = (radius_ + other.radius_) * kMergeRatio;
	return dx * dx + dz * dz <= r * r;
}

void EnemyBubble::Merge(EnemyBubble& other) {
	// 位置と速度は、重さ(泡の大きさ + 中の敵)を重みにして混ぜる
	const float massA = GetMass();
	const float massB = other.GetMass();
	const float total = massA + massB;
	position_.x = (position_.x * massA + other.position_.x * massB) / total;
	position_.z = (position_.z * massA + other.position_.z * massB) / total;
	velocity_.x = (velocity_.x * massA + other.velocity_.x * massB) / total;
	velocity_.z = (velocity_.z * massA + other.velocity_.z * massB) / total;

	// 敵を全部引き取る
	for (Enemy* e : other.enemies_) {
		e->SetPosition(position_);
		enemies_.push_back(e);
	}
	other.enemies_.clear();
	other.isDead_ = true; // 取り込まれた側は消える

	// 容量は足し算(空きも引き継ぐ)
	capacity_ += other.capacity_;

	// 閉じ込めの残り時間は、長いほうに合わせる
	timer_ = max(timer_, other.timer_);
	UpdateRadius();
}

int EnemyBubble::Pop() {
	const int count = static_cast<int>(enemies_.size());
	for (Enemy* e : enemies_) {
		e->Kill();
	}
	enemies_.clear();
	isDead_ = true;
	return count;
}

void EnemyBubble::ReleaseEnemies() {
	// 逃げた敵が重ならないよう、泡の中心から円状に少しずつずらして出す
	const int count = static_cast<int>(enemies_.size());

	// 1体だけならずらさず、複数なら泡の半径の半分だけ中心から離す
	float spread = 0.0f;
	if (count > 1) {
		spread = radius_ * 0.5f;
	}

	for (int i = 0; i < count; i++) {
		const float angle = 2.0f * kPi * static_cast<float>(i) / static_cast<float>(count);
		enemies_[i]->SetPosition({ position_.x + std::sin(angle) * spread, 0.0f, position_.z + std::cos(angle) * spread });
		enemies_[i]->SetCaptured(false);
	}
	enemies_.clear();
	isDead_ = true;
}