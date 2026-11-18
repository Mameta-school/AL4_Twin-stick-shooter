#pragma once
#include "Bubble.h"
#include "Enemy.h"
#include "EnemyBubble.h"
#include "KamataEngine.h"
#include "Player.h"
#include "PopBullet.h"
#include <vector>

using namespace KamataEngine;

// 撃つ弾の種類(SHIFTで切り替え)
enum class ShotMode {
	Bubble, // 泡: 敵を閉じ込める
	Pop,    // 破裂弾: 敵入りの泡を割る
};

class GameScene {
public:
	~GameScene();
	void Initialize();
	void Update();
	void Draw();

private:
	// マウス座標 → 地面(y=0)上のワールド座標
	Vector3 GetMouseWorldPos() const;
	// プレイヤーの周りに敵を1体出す
	void SpawnEnemy();
	// 撃った泡に重なっている敵を、泡の大きさぶんだけ閉じ込める
	void CaptureEnemies(Bubble* bubble);
	// center を中心に、半径 radius の少し内側から、count 個の小泡を円状にばらまく
	void SpawnScatterBubbles(const Vector3& center, float radius, int count);

	static constexpr float kCameraFollowRate = 0.1f; // カメラ追従のなめらかさ
	static constexpr int kShotInterval = 10;         // 泡の連射間隔(フレーム)
	static constexpr int kPopShotInterval = 20;      // 破裂弾の連射間隔(フレーム)
	static constexpr int kSpawnInterval = 120;       // 敵の出現間隔(フレーム)
	static constexpr int kMaxEnemies = 20;           // 同時に存在できる敵の数
	static constexpr float kSpawnDistance = 14.0f;   // プレイヤーからの出現距離
	static constexpr float kFloorLimit = 19.0f;      // 床の端(ここより外には出さない)
	static constexpr float kTileSize = 4.0f;        // 床タイル1枚の大きさ
	static constexpr int kMinPopCapacity = 2;       // この大きさ(小泡何個ぶん)以上の泡だけ、破裂弾で割れる
	static constexpr float kScatterSpeed = 0.4f;    // ばらまいた小泡の速さ
	static constexpr int kScatterPerEmptySlot = 1;  // 敵入りの泡が割れたとき、空いていた分1つにつき出る小泡の数
	static constexpr int kScatterPerEnemy = 2;      // 敵入りの泡が割れたとき、中の敵1体につき出る小泡の数
	static constexpr int kScatterMergeCooldown = 30; // ばらまいた小泡が、合体しない時間(フレーム)
	static constexpr int kTileCount = 10;          // 床タイルの数(縦・横とも)

	Camera camera_;
	Model* model_ = nullptr;       // キューブ(プレイヤー・敵・床・破裂弾)
	Model* sphereModel_ = nullptr; // 球(泡)
	Player player_;
	std::vector<Bubble*> bubbles_;           // プレイヤーが撃った泡
	std::vector<Enemy*> enemies_;            // 敵(泡に入っているものも含む)
	std::vector<EnemyBubble*> enemyBubbles_; // 敵を閉じ込めた泡
	std::vector<PopBullet*> popBullets_;     // プレイヤーが撃った破裂弾
	ShotMode shotMode_ = ShotMode::Bubble;   // 今選んでいる弾
	int shotTimer_ = 0;
	int spawnTimer_ = 0;

	// 床はチェッカー模様のタイル(カメラが追従するので、動いている感覚を出すため)
	WorldTransform floorTiles_[kTileCount * kTileCount];
	ObjectColor floorColorA_;
	ObjectColor floorColorB_;

	WorldTransform modeMarker_; // 頭上の目印(今の弾の種類を示す)
	ObjectColor markerBubbleColor_;
	ObjectColor markerPopColor_;

	WorldTransform aimMarker_; // マウスが指している地面の位置
	ObjectColor aimColor_;
};