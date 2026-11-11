#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

namespace WorldMatrix {
	// スケール・回転・平行移動から行列を作り、定数バッファへ転送する
	void UpdateWorldMatrix(WorldTransform& worldTransform);
}