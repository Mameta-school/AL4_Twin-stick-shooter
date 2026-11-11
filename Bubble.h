#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

// 減速しながら、ゆらゆら漂う泡
class Bubble {
public:
	static constexpr float kRadius = 0.6f;

	void Initialize(Model* model, const Vector3& position, const Vector3& direction);
	void Update();
	void Draw(const Camera& camera);

	bool IsDead() const { return isDead_; }
	void Kill() { isDead_ = true; }
	const Vector3& GetPosition() const { return worldTransform_.translation_; }

private:
	static constexpr float kStartSpeed = 0.6f;     // 撃ち出しの速さ(1フレーム)
	static constexpr float kDeceleration = 0.95f;  // 毎フレームの減速率
	static constexpr float kMinSpeed = 0.03f;      // 最低速度(ふわふわ漂う速さ)
	static constexpr float kSwayAmplitude = 0.03f; // 横揺れの大きさ
	static constexpr float kSwayFrequency = 0.15f; // 横揺れの速さ
	static constexpr int kLifeFrames = 180;        // 寿命(3秒)

	Model* model_ = nullptr;
	WorldTransform worldTransform_;
	Vector3 direction_ = { 0.0f, 0.0f, 1.0f };
	float speed_ = kStartSpeed;
	float phase_ = 0.0f;
	int life_ = kLifeFrames;
	bool isDead_ = false;
};