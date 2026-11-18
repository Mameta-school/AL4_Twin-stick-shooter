#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

// 敵入りの泡を割るための、速くてまっすぐ飛ぶ弾
class PopBullet {
public:
	static constexpr float kRadius = 0.3f; // 当たり判定の半径

	void Initialize(Model* model, const Vector3& position, const Vector3& direction);
	void Update();
	void Draw(const Camera& camera);

	bool IsDead() const { return isDead_; }
	void Kill() { isDead_ = true; }
	const Vector3& GetPosition() const { return worldTransform_.translation_; }

private:
	static constexpr float kSpeed = 0.9f;  // 1フレームに進む距離
	static constexpr int kLifeFrames = 45; // 寿命(0.75秒 ≒ 約40の距離)

	Model* model_ = nullptr;
	WorldTransform worldTransform_;
	ObjectColor color_;
	Vector3 direction_ = { 0.0f, 0.0f, 1.0f };
	int life_ = kLifeFrames;
	bool isDead_ = false;
};