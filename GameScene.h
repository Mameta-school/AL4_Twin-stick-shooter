#pragma once
#include "Bubble.h"
#include "Enemy.h"
#include "KamataEngine.h"
#include "Player.h"
#include <vector>

using namespace KamataEngine;

class GameScene {
public:
	~GameScene();
	void Initialize();
	void Update();
	void Draw();

private:
	// マウス座標
	Vector3 GetMouseWorldPos() const;
	// プレイヤーの周りに敵を1体出す
	void SpawnEnemy();

	static constexpr float kCameraFollowRate = 0.1f; // カメラ追従のなめらかさ
	static constexpr int kShotInterval = 10;         // 連射間隔(フレーム)
	static constexpr int kSpawnInterval = 120;       // 敵の出現間隔(フレーム)
	static constexpr int kMaxEnemies = 20;           // 同時に存在できる敵の数
	static constexpr float kSpawnDistance = 14.0f;   // プレイヤーからの出現距離
	static constexpr float kFloorLimit = 19.0f;      // 床の端(ここより外には出さない)

	Camera camera_;
	Model* model_ = nullptr;
	Player player_;
	std::vector<Bubble*> bubbles_;
	std::vector<Enemy*> enemies_;
	int shotTimer_ = 0;
	int spawnTimer_ = 0;

	WorldTransform floorTransform_;
};