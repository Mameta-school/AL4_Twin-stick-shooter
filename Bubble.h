#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

// 減速しながら、ゆらゆら漂う泡。重なると合体して大きくなる
class Bubble {
public:
	static constexpr float kRadius = 0.6f;      // 撃ったときの半径
	static constexpr float kMaxRadius = 4.0f;   // 合体で大きくなれる上限

	void Initialize(Model* model, const Vector3& position, const Vector3& direction);
	void Update();
	void Draw(const Camera& camera);

	// 合体できるか(重なっていて、合体後も上限を超えない)
	bool CanMerge(const Bubble& other) const;
	// other を取り込む(other は消える)
	void Merge(Bubble& other);

	bool IsDead() const { return isDead_; }
	void Kill() { isDead_ = true; }
	// 速さを変える(ばらまいた小泡は、撃ったときより遅く出す)
	void SetSpeed(float speed) { speed_ = speed; }
	// 指定フレームのあいだ、ほかの泡と合体しない(ばらまいた直後に合体しないため)
	void SetMergeCooldown(int frames) { mergeCooldown_ = frames; }
	float GetRadius() const { return radius_; }
	// 今の速度(向き×速さ)。敵を閉じ込めるとき、敵入りの泡に引き継ぐ
	Vector3 GetVelocity() const { return { direction_.x * speed_, 0.0f, direction_.z * speed_ }; }
	// 小泡何個ぶんの大きさか(= 一度に閉じ込められる敵の数)
	int GetCapacity() const {
		int capacity = static_cast<int>(GetMass() + 0.5f);
		if (capacity < 1) {
			capacity = 1;
		}
		return capacity;
	}
	// 重さ(小泡1個 = 1.0。大きくなるほど面積に比例して重い)
	float GetMass() const {
		const float ratio = radius_ / kRadius;
		return ratio * ratio;
	}
	const Vector3& GetPosition() const { return worldTransform_.translation_; }

private:
	static constexpr float kStartSpeed = 0.6f;     // 撃ち出しの速さ(1フレーム)
	static constexpr float kDeceleration = 0.95f;  // 毎フレームの減速率
	static constexpr float kMinSpeed = 0.03f;      // 最低速度(ふわふわ漂う速さ)
	static constexpr float kSwayAmplitude = 0.03f; // 横揺れの大きさ
	static constexpr float kSwayFrequency = 0.15f; // 横揺れの速さ
	static constexpr int kLifeFrames = 180;        // 寿命(3秒)
	static constexpr float kMergeRatio = 0.8f;     // 半径の和のこの割合まで近づくと合体

	Model* model_ = nullptr;
	WorldTransform worldTransform_;
	ObjectColor color_;
	Vector3 direction_ = { 0.0f, 0.0f, 1.0f };
	float speed_ = kStartSpeed;
	float radius_ = kRadius;
	float phase_ = 0.0f; // 揺れの位相
	int life_ = kLifeFrames;
	int mergeCooldown_ = 0; // 合体できるようになるまでの残りフレーム
	bool isDead_ = false;
};