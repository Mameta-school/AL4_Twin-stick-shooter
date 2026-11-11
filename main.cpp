#include <Windows.h>
#include "GameScene.h"

using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	// エンジンの初期化
	Initialize(L"LE2C_02_アメタ_コウキ");

	srand(static_cast<unsigned int>(time(nullptr)));

	GameScene* gameScene = new GameScene();
	gameScene->Initialize();

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// メインループ
	while (true) {
		// エンジンの更新
		if (Update()) {
			break;
		}
		
		gameScene->Update();

		// 描画開始
		dxCommon->PreDraw();

		gameScene->Draw();

		// 描画終了
		dxCommon->PostDraw();
	}

	delete gameScene;

	// エンジンの終了処理
	Finalize();
	return 0;
}