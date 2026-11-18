#include "GameScene.h"
#include "MathUtility.h"
#include "WorldMatrix.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace {
	// プレイヤーからのカメラ相対位置(斜め上・手前)
	const Vector3 kCameraOffset = { 0.0f, 30.0f, -20.0f };

	// XZ平面での距離の2乗
	float DistanceSq(const Vector3& a, const Vector3& b) {
		const float dx = a.x - b.x;
		const float dz = a.z - b.z;
		return dx * dx + dz * dz;
	}

	// 円と円の当たり判定(XZ平面)
	bool IsHitCircle(const Vector3& a, float radiusA, const Vector3& b, float radiusB) {
		const float r = radiusA + radiusB;
		return DistanceSq(a, b) <= r * r;
	}
} // namespace

GameScene::~GameScene() {
	for (Bubble* b : bubbles_) {
		delete b;
	}
	for (EnemyBubble* eb : enemyBubbles_) {
		delete eb;
	}
	for (Enemy* e : enemies_) {
		delete e;
	}
	for (PopBullet* p : popBullets_) {
		delete p;
	}
	delete sphereModel_;
	delete model_;
}

void GameScene::Initialize() {
	model_ = Model::Create();
	sphereModel_ = Model::CreateFromOBJ("sphere", true); // Resources/sphere/sphere.obj

	// プレイヤーを先に初期化(カメラの位置がプレイヤー基準のため)
	player_.Initialize(model_, { 0.0f, 0.0f, 0.0f });

	camera_.Initialize();
	camera_.rotation_ = { ToRadians(56.0f), 0.0f, 0.0f };
	camera_.translation_ = {
		player_.GetPosition().x + kCameraOffset.x,
		player_.GetPosition().y + kCameraOffset.y,
		player_.GetPosition().z + kCameraOffset.z };
	camera_.UpdateMatrix();

	// 床(タイルを敷き詰める。キューブは一辺2なので、scale = タイルの半分の大きさ)
	const float floorHalf = kTileSize * static_cast<float>(kTileCount) * 0.5f;
	for (int i = 0; i < kTileCount; i++) {
		for (int j = 0; j < kTileCount; j++) {
			WorldTransform& tile = floorTiles_[i * kTileCount + j];
			tile.Initialize();
			tile.translation_ = {
				-floorHalf + kTileSize * (static_cast<float>(i) + 0.5f),
				-1.2f,
				-floorHalf + kTileSize * (static_cast<float>(j) + 0.5f) };
			tile.scale_ = { kTileSize * 0.5f, 0.2f, kTileSize * 0.5f };
			WorldMatrix::UpdateWorldMatrix(tile);
		}
	}
	floorColorA_.Initialize();
	floorColorA_.SetColor({ 0.35f, 0.38f, 0.45f, 1.0f });
	floorColorB_.Initialize();
	floorColorB_.SetColor({ 0.28f, 0.30f, 0.37f, 1.0f });

	// 頭上の目印(泡モード=水色の球 / 破裂弾モード=黄色の棒)
	modeMarker_.Initialize();
	markerBubbleColor_.Initialize();
	markerBubbleColor_.SetColor({ 0.5f, 0.8f, 1.0f, 1.0f });
	markerPopColor_.Initialize();
	markerPopColor_.SetColor({ 1.0f, 0.9f, 0.2f, 1.0f });

	// マウスが指している地面の位置(平たい白い目印)
	aimMarker_.Initialize();
	aimMarker_.scale_ = { 0.5f, 0.05f, 0.5f };
	aimColor_.Initialize();
	aimColor_.SetColor({ 1.0f, 1.0f, 1.0f, 0.6f });

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

void GameScene::SpawnScatterBubbles(const Vector3& center, float radius, int count) {
	if (count <= 0) {
		return;
	}
	const float baseAngle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * kPi;

	for (int i = 0; i < count; i++) {
		// 円状に均等な向きへ飛ばす
		const float angle = baseAngle + 2.0f * kPi * static_cast<float>(i) / static_cast<float>(count);
		const Vector3 direction = { std::sin(angle), 0.0f, std::cos(angle) };

		// 割れた泡の中心から、半径の半分だけ外側から出す
		const float offset = radius * 0.5f;
		const Vector3 position = {
			center.x + direction.x * offset,
			center.y,
			center.z + direction.z * offset };

		Bubble* scatterBubble = new Bubble();
		scatterBubble->Initialize(sphereModel_, position, direction);
		scatterBubble->SetSpeed(kScatterSpeed);
		scatterBubble->SetMergeCooldown(kScatterMergeCooldown);
		bubbles_.push_back(scatterBubble);
	}
}

void GameScene::CaptureEnemies(Bubble* bubble) {
	// 1. 泡に重なっている、まだ捕まっていない敵を集める
	std::vector<Enemy*> candidates;
	for (Enemy* e : enemies_) {
		if (e->IsDead() || e->IsCaptured()) {
			continue;
		}
		if (IsHitCircle(bubble->GetPosition(), bubble->GetRadius(), e->GetPosition(), Enemy::kRadius)) {
			candidates.push_back(e);
		}
	}
	if (candidates.empty()) {
		return;
	}

	// 2. 泡の大きさ(capacity)ぶんだけ、泡に近い順に選ぶ
	std::vector<Enemy*> targets;
	const int capacity = bubble->GetCapacity();
	while (!candidates.empty() && static_cast<int>(targets.size()) < capacity) {
		size_t nearest = 0;
		for (size_t i = 1; i < candidates.size(); i++) {
			if (DistanceSq(candidates[i]->GetPosition(), bubble->GetPosition()) < DistanceSq(candidates[nearest]->GetPosition(), bubble->GetPosition())) {
				nearest = i;
			}
		}
		targets.push_back(candidates[nearest]);
		candidates.erase(candidates.begin() + nearest);
	}

	// 3. 選んだ敵の真ん中に、敵入りの泡を作って全員入れる
	Vector3 center = { 0.0f, 0.0f, 0.0f };
	for (Enemy* e : targets) {
		center.x += e->GetPosition().x;
		center.z += e->GetPosition().z;
	}
	center.x /= static_cast<float>(targets.size());
	center.z /= static_cast<float>(targets.size());

	EnemyBubble* enemyBubble = new EnemyBubble();
	// 運動量が保存される形で速度を引き継ぐ(中の敵が重いほど、勢いが落ちる)
	const float bubbleMass = bubble->GetMass();
	const float enemyMass = Enemy::kMass * static_cast<float>(targets.size());
	const float ratio = bubbleMass / (bubbleMass + enemyMass);
	const Vector3 bubbleVelocity = bubble->GetVelocity();
	const Vector3 inheritedVelocity = { bubbleVelocity.x * ratio, 0.0f, bubbleVelocity.z * ratio };
	enemyBubble->Initialize(sphereModel_, center, inheritedVelocity, capacity); // 撃った泡の大きさを引き継ぐ
	for (Enemy* e : targets) {
		enemyBubble->AddEnemy(e);
	}
	enemyBubbles_.push_back(enemyBubble);

	// 撃った泡は1個ぶん消費
	bubble->Kill();
}

void GameScene::Update() {
	const Vector3 aimPos = GetMouseWorldPos();
	player_.Update(aimPos);

	// 狙っている地面の位置に目印を置く
	aimMarker_.translation_ = { aimPos.x, -0.9f, aimPos.z };
	WorldMatrix::UpdateWorldMatrix(aimMarker_);

	// --- SHIFTで弾の種類を切り替え ---
	Input* input = Input::GetInstance();
	if (input->TriggerKey(DIK_LSHIFT) || input->TriggerKey(DIK_RSHIFT)) {
		if (shotMode_ == ShotMode::Bubble) {
			shotMode_ = ShotMode::Pop;
		}
		else {
			shotMode_ = ShotMode::Bubble;
		}
	}

	// 頭上の目印: 泡=大きい四角 / 破裂弾=細長い棒
	modeMarker_.translation_ = { player_.GetPosition().x, 2.4f, player_.GetPosition().z };
	if (shotMode_ == ShotMode::Bubble) {
		modeMarker_.scale_ = { 0.5f, 0.5f, 0.5f };
	}
	else {
		modeMarker_.scale_ = { 0.15f, 0.7f, 0.15f };
	}
	WorldMatrix::UpdateWorldMatrix(modeMarker_);

	// --- 弾を撃つ (SPACE長押しで連射) ---
	if (shotTimer_ > 0) {
		shotTimer_--;
	}
	if (input->PushKey(DIK_SPACE) && shotTimer_ <= 0) {
		const Vector3& dir = player_.GetAimDirection();
		const Vector3 muzzle = { player_.GetPosition().x + dir.x, player_.GetPosition().y, player_.GetPosition().z + dir.z };

		if (shotMode_ == ShotMode::Bubble) {
			shotTimer_ = kShotInterval;
			Bubble* bubble = new Bubble();
			bubble->Initialize(sphereModel_, muzzle, dir);
			bubbles_.push_back(bubble);
		}
		else {
			shotTimer_ = kPopShotInterval;
			PopBullet* popBullet = new PopBullet();
			popBullet->Initialize(model_, muzzle, dir);
			popBullets_.push_back(popBullet);
		}
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
	for (EnemyBubble* eb : enemyBubbles_) {
		eb->Update(kFloorLimit);
	}
	for (PopBullet* p : popBullets_) {
		p->Update();
	}

	// --- 撃った泡同士の合体 ---
	for (size_t i = 0; i < bubbles_.size(); i++) {
		for (size_t j = i + 1; j < bubbles_.size(); j++) {
			if (bubbles_[i]->CanMerge(*bubbles_[j])) {
				bubbles_[i]->Merge(*bubbles_[j]);
			}
		}
	}

	// --- 敵入りの泡同士の合体 ---
	for (size_t i = 0; i < enemyBubbles_.size(); i++) {
		for (size_t j = i + 1; j < enemyBubbles_.size(); j++) {
			if (enemyBubbles_[i]->CanMerge(*enemyBubbles_[j])) {
				enemyBubbles_[i]->Merge(*enemyBubbles_[j]);
			}
		}
	}

	// --- 空きのある敵入りの泡が、触れた敵を吸い込む ---
	for (EnemyBubble* eb : enemyBubbles_) {
		if (eb->IsDead() || !eb->HasRoom()) {
			continue;
		}
		for (Enemy* e : enemies_) {
			if (!eb->HasRoom()) {
				break; // 満員になったら終わり
			}
			if (e->IsDead() || e->IsCaptured()) {
				continue;
			}
			if (IsHitCircle(eb->GetPosition(), eb->GetRadius(), e->GetPosition(), Enemy::kRadius)) {
				eb->AddEnemy(e);
			}
		}
	}

	// --- 撃った泡の当たり判定: 自由な敵を閉じ込める(敵入りの泡は素通り) ---
	for (Bubble* bubble : bubbles_) {
		if (!bubble->IsDead()) {
			CaptureEnemies(bubble);
		}
	}

	// --- 破裂弾の当たり判定 ---
	// 1. 敵入りの泡に当たったら、割って中の敵を全員倒す
	// 2. 大きい泡に当たったら、割って周りに小泡をばらまく
	for (PopBullet* popBullet : popBullets_) {
		if (popBullet->IsDead()) {
			continue;
		}

		// 1. 敵入りの泡
		bool hitEnemyBubble = false;
		for (EnemyBubble* eb : enemyBubbles_) {
			if (eb->IsDead()) {
				continue;
			}
			if (IsHitCircle(popBullet->GetPosition(), PopBullet::kRadius, eb->GetPosition(), eb->GetRadius())) {
				// 割れたら小泡をばらまく: 中にいた敵の数 × kScatterPerEnemy + 空いていた分 × kScatterPerEmptySlot
				const Vector3 popPosition = { eb->GetPosition().x, 0.0f, eb->GetPosition().z };
				const float popRadius = eb->GetRadius();
				const int emptySlots = eb->GetEmptySlots(); // Pop() の前に数えておく
				const int killedCount = eb->Pop();
				SpawnScatterBubbles(popPosition, popRadius, killedCount * kScatterPerEnemy + emptySlots * kScatterPerEmptySlot);
				popBullet->Kill();
				hitEnemyBubble = true;
				break; // 1発で割れる泡は1個まで
			}
		}
		if (hitEnemyBubble) {
			continue;
		}

		// 2. 撃った泡(小泡は割れないので素通り)
		Bubble* hitBubble = nullptr;
		for (Bubble* bubble : bubbles_) {
			if (bubble->IsDead() || bubble->GetCapacity() < kMinPopCapacity) {
				continue;
			}
			if (IsHitCircle(popBullet->GetPosition(), PopBullet::kRadius, bubble->GetPosition(), bubble->GetRadius())) {
				hitBubble = bubble;
				break;
			}
		}
		// bubbles_ を回し終えてから、小泡を追加する(回している途中で増やさないため)
		if (hitBubble != nullptr) {
			SpawnScatterBubbles(hitBubble->GetPosition(), hitBubble->GetRadius(), hitBubble->GetCapacity());
			hitBubble->Kill();
			popBullet->Kill();
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
	for (std::vector<EnemyBubble*>::iterator it = enemyBubbles_.begin(); it != enemyBubbles_.end();) {
		if ((*it)->IsDead()) {
			delete* it;
			it = enemyBubbles_.erase(it);
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

	for (std::vector<PopBullet*>::iterator it = popBullets_.begin(); it != popBullets_.end();) {
		if ((*it)->IsDead()) {
			delete* it;
			it = popBullets_.erase(it);
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

	// --- 不透明なものを先に描く ---
	for (int i = 0; i < kTileCount; i++) {
		for (int j = 0; j < kTileCount; j++) {
			WorldTransform& tile = floorTiles_[i * kTileCount + j];
			if ((i + j) % 2 == 0) {
				model_->Draw(tile, camera_, &floorColorA_);
			}
			else {
				model_->Draw(tile, camera_, &floorColorB_);
			}
		}
	}
	player_.Draw(camera_);
	for (Enemy* e : enemies_) {
		e->Draw(camera_); // 泡に入っている敵も、ここで描く
	}
	for (PopBullet* p : popBullets_) {
		p->Draw(camera_);
	}

	// --- 半透明のものをあとから描く(中の敵が透けて見える) ---
	for (EnemyBubble* eb : enemyBubbles_) {
		eb->Draw(camera_);
	}
	for (Bubble* b : bubbles_) {
		b->Draw(camera_);
	}
	model_->Draw(aimMarker_, camera_, &aimColor_);

	// 頭上の目印
	if (shotMode_ == ShotMode::Bubble) {
		sphereModel_->Draw(modeMarker_, camera_, &markerBubbleColor_);
	}
	else {
		model_->Draw(modeMarker_, camera_, &markerPopColor_);
	}

	Model::PostDraw();
}