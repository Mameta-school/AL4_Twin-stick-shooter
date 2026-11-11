#include "GameScene.h"
#include "MathUtility.h"
#include "WorldMatrix.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace {
	// プレイヤーからのカメラ相対位置(斜め上・手前)
	const Vector3 kCameraOffset = { 0.0f, 30.0f, -20.0f };

	// 円と円の当たり判定(XZ平面)
	bool IsHitCircle(const Vector3& a, float radiusA, const Vector3& b, float radiusB) {
		const float dx = a.x - b.x;
		const float dz = a.z - b.z;
		const float r = radiusA + radiusB;
		return dx * dx + dz * dz <= r * r;
	}
}

GameScene::~GameScene() {
	for (Bubble* b : bubbles_) {
		delete b;
	}
	for (Enemy* e : enemies_) {
		delete e;
	}
	delete model_;
}

void GameScene::Initialize() {
	model_ = Model::Create();

	// プレイヤーを先に初期化(カメラの位置がプレイヤー基準のため)
	player_.Initialize(model_, { 0.0f, 0.0f, 0.0f });

	camera_.Initialize();
	camera_.rotation_ = { ToRadians(56.0f), 0.0f, 0.0f };
	camera_.translation_ = {
		player_.GetPosition().x + kCameraOffset.x,
		player_.GetPosition().y + kCameraOffset.y,
		player_.GetPosition().z + kCameraOffset.z };
	camera_.UpdateMatrix();

	// 床
	floorTransform_.Initialize();
	floorTransform_.translation_ = { 0.0f, -1.2f, 0.0f };
	floorTransform_.scale_ = { 20.0f, 0.2f, 20.0f };
	WorldMatrix::UpdateWorldMatrix(floorTransform_);

	spawnTimer_ = kSpawnInterval;
}

Vector3 GameScene::GetMouseWorldPos() const {
	const Vector2 mouse = Input::GetInstance()->GetMousePosition();
	const float width = static_cast<float>(WinApp::kWindowWidth);
	const float height = static_cast<float>(WinApp::kWindowHeight);

	// 画面座標 → NDC (-1〜1, 上が+)
	const float nx = mouse.x / width * 2.0f - 1.0f;
	const float ny = 1.0f - mouse.y / height * 2.0f;

	// カメラ空間でのレイの向き → カメラの回転でワールド空間へ
	const float tanHalfFov = std::tan(camera_.fovAngleY * 0.5f);
	const Vector3 rayCamera = { nx * tanHalfFov * (width / height), ny * tanHalfFov, 1.0f };
	const Vector3 ray = Transform(rayCamera, MakeRotateXMatrix(camera_.rotation_.x));

	// レイと地面(y = プレイヤーの高さ)の交点
	const float groundY = player_.GetPosition().y;
	if (ray.y >= -0.0001f) {
		return { player_.GetPosition().x, groundY, player_.GetPosition().z + 1.0f };
	}
	const float t = (groundY - camera_.translation_.y) / ray.y;
	return { camera_.translation_.x + ray.x * t, groundY, camera_.translation_.z + ray.z * t };
}

void GameScene::SpawnEnemy() {
	// プレイヤーの周り(kSpawnDistance離れた円周上)のランダムな方向に出す
	const float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * kPi;
	Vector3 position = {
		player_.GetPosition().x + std::sin(angle) * kSpawnDistance,
		0.0f,
		player_.GetPosition().z + std::cos(angle) * kSpawnDistance };

	// 床の外に出ないようにする
	position.x = std::clamp(position.x, -kFloorLimit, kFloorLimit);
	position.z = std::clamp(position.z, -kFloorLimit, kFloorLimit);

	Enemy* enemy = new Enemy();
	enemy->Initialize(model_, position);
	enemies_.push_back(enemy);
}

void GameScene::Update() {
	player_.Update(GetMouseWorldPos());

	// --- 泡を撃つ (SPACE長押しで連射) ---
	if (shotTimer_ > 0) {
		shotTimer_--;
	}
	if (Input::GetInstance()->PushKey(DIK_SPACE) && shotTimer_ <= 0) {
		shotTimer_ = kShotInterval;
		const Vector3& dir = player_.GetAimDirection();
		const Vector3 muzzle = { player_.GetPosition().x + dir.x, player_.GetPosition().y, player_.GetPosition().z + dir.z };
		Bubble* bubble = new Bubble();
		bubble->Initialize(model_, muzzle, dir);
		bubbles_.push_back(bubble);
	}

	// --- 敵の出現 ---
	spawnTimer_--;
	if (spawnTimer_ <= 0) {
		spawnTimer_ = kSpawnInterval;
		if (static_cast<int>(enemies_.size()) < kMaxEnemies) {
			SpawnEnemy();
		}
	}

	// --- 更新 ---
	for (Bubble* b : bubbles_) {
		b->Update();
	}
	for (Enemy* e : enemies_) {
		e->Update(player_.GetPosition());
	}

	// --- 泡と敵の当たり判定 ---
	for (Bubble* bubble : bubbles_) {
		if (bubble->IsDead()) {
			continue;
		}
		for (Enemy* enemy : enemies_) {
			if (enemy->IsDead()) {
				continue;
			}
			if (IsHitCircle(bubble->GetPosition(), Bubble::kRadius, enemy->GetPosition(), enemy->GetRadius())) {
				if (enemy->IsTrapped()) {
					enemy->Kill(); // 閉じ込め中に当てると割れて倒れる
				}
				else {
					enemy->Trap(); // 通常時に当てると閉じ込める
				}
				bubble->Kill(); // 泡は1個消費
				break;          // 1つの泡で複数の敵には当たらない
			}
		}
	}

	// --- 死んだものを削除 ---
	for (std::vector<Bubble*>::iterator it = bubbles_.begin(); it != bubbles_.end();) {
		if ((*it)->IsDead()) {
			delete* it;
			it = bubbles_.erase(it);
		}
		else {
			++it;
		}
	}
	for (std::vector<Enemy*>::iterator it = enemies_.begin(); it != enemies_.end();) {
		if ((*it)->IsDead()) {
			delete* it;
			it = enemies_.erase(it);
		}
		else {
			++it;
		}
	}

	// --- カメラ追従 ---
	const Vector3 target = {
		player_.GetPosition().x + kCameraOffset.x,
		player_.GetPosition().y + kCameraOffset.y,
		player_.GetPosition().z + kCameraOffset.z };
	camera_.translation_ = Lerp(camera_.translation_, target, kCameraFollowRate);
	camera_.UpdateMatrix();
}

void GameScene::Draw() {
	Model::PreDraw();
	model_->Draw(floorTransform_, camera_);
	player_.Draw(camera_);
	for (Bubble* b : bubbles_) {
		b->Draw(camera_);
	}
	for (Enemy* e : enemies_) {
		e->Draw(camera_);
	}
	Model::PostDraw();
}