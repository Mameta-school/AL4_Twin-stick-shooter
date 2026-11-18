#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

// プレイヤーを追いかける敵。泡に閉じ込められている間は動かない(描画は泡の中で行う)
class Enemy {
public:
	static constexpr float kRadius = 0.8f;
	static constexpr float kMass = 1.0f; // 重さ(泡の重さは、中の敵の合計)

	void Initialize(Model* model, const Vector3& position);
	void Update(const Vector3& targetPos);
	void Draw(const Camera& camera);

	void Kill() { isDead_ = true; }
	void SetCaptured(bool captured) { isCaptured_ = captured; }
	void SetPosition(const Vector3& position); // 行列も更新する

	bool IsDead() const { return isDead_; }
	bool IsCaptured() const { return isCaptured_; }
	const Vector3& GetPosition() const { return worldTransform_.translation_; }

private:
	static constexpr float kMoveSpeed = 0.08f; // 追いかける速さ(プレイヤーより遅い)

	Model* model_ = nullptr;
	WorldTransform worldTransform_;
	ObjectColor color_;
	bool isCaptured_ = false; // 泡に入っているか(入っている間は動かない)
	bool isDead_ = false;
};