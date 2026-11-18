#pragma once
#include "Enemy.h"
#include "KamataEngine.h"
#include <vector>

using namespace KamataEngine;

// 敵を閉じ込めた泡。撃った泡の大きさ(容量)を引き継ぎ、空きがあれば近くの敵を吸い込む。重なると合体する
// ※ 敵そのものは GameScene が持っている。ここでは「どの敵が入っているか」を覚えるだけ
class EnemyBubble {
public:
	// velocity: 撃った泡から引き継ぐ速度 / capacity: 撃った泡の大きさ(小泡何個ぶんか) = 入れられる敵の数
	void Initialize(Model* model, const Vector3& position, const Vector3& velocity, int capacity);
	// floorLimit: 床の端(これより外には出ない)
	void Update(float floorLimit);
	void Draw(const Camera& camera);

	void AddEnemy(Enemy* enemy);                // 敵を1体入れる
	bool CanMerge(const EnemyBubble& other) const; // 重なっていて合体できるか
	void Merge(EnemyBubble& other);             // other の敵を全部取り込む(other は消える)
	int Pop();                                  // 割る。中の敵を全員倒して、倒した数を返す

	bool IsDead() const { return isDead_; }
	float GetRadius() const { return radius_; }
	float GetMass() const; // 重さ(泡そのものの重さ + 中の敵の重さの合計)
	// あと何体入れられるか(0なら満員)
	int GetEmptySlots() const { return capacity_ - static_cast<int>(enemies_.size()); }
	bool HasRoom() const { return GetEmptySlots() > 0; }
	const Vector3& GetPosition() const { return position_; }

private:
	void UpdateRadius();   // 中の敵の数から大きさを決める
	void ReleaseEnemies(); // 時間切れ: 敵を逃がす

	static constexpr float kBaseRadius = 1.4f; // 敵1体のときの半径
	static constexpr float kMaxRadius = 6.0f;  // どれだけ入っても超えない半径
	static constexpr int kTrapFrames = 300;    // 閉じ込めている時間(5秒)
	static constexpr float kFloatHeight = 0.3f; // 浮かぶ上下幅
	static constexpr float kMergeRatio = 0.9f; // 半径の和のこの割合まで近づくと合体
	static constexpr float kDeceleration = 0.96f; // 重さ1のときの減速率(1に近いほど長く滑る)。重いほど滑りやすくなる
	static constexpr int kWarningFrames = 60;      // 脱出まであと何フレームで点滅を始めるか
	static constexpr float kStopSpeed = 0.005f;   // これ以下の速さになったら止める

	Model* model_ = nullptr;
	WorldTransform worldTransform_;
	ObjectColor color_;
	Vector3 position_ = { 0.0f, 0.0f, 0.0f }; // 泡の中心(高さ0。浮かぶ分は描画だけに反映)
	Vector3 velocity_ = { 0.0f, 0.0f, 0.0f }; // 滑る速度(だんだん減速する)
	std::vector<Enemy*> enemies_;           // 入っている敵(所有はしない)

	int capacity_ = 1; // 入れられる敵の数(撃った泡の大きさを引き継ぐ)
	float radius_ = kBaseRadius;
	float floatPhase_ = 0.0f;
	int timer_ = kTrapFrames;
	bool isDead_ = false;
};