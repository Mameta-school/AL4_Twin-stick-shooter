#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

class Enemy {
public:
	static constexpr float kRadius = 0.8f;			// 通常時の当たり判定
	static constexpr float kTrappedRadius = 1.4f;	// 泡に入っているときの当たり判定

	void Initialize(Model* model, const Vector3& position);
	void Update(const Vector3& targetPos);
	void Draw(const Camera& camera);

	void Trap();
	void Kill() { isDead_ = true; }

	bool IsTrapped() const { return isTrapped_; }
	bool IsDead() const { return isDead_; }
	float GetRadius() const { return isTrapped_ ? kTrappedRadius : kRadius; }
	const Vector3& GetPosition() const { return worldTransform_.translation_; }

private:
	static constexpr float kMoveSpeed = 0.08f;	// 追いかける速さ
	static constexpr int kTrapFrames = 300;		// 閉じ込められている時間
	static constexpr float kFloatHeight = 0.3f;	// 浮かぶ上下幅

	Model* model_ = nullptr;
	WorldTransform worldTransform_;	// 敵本体
	WorldTransform shellTransform_;	// 閉じ込めている泡

	bool isTrapped_ = false;
	bool isDead_ = false;
	int trapTimer_ = 0;
	float floatPhase_ = 0.0f;
};