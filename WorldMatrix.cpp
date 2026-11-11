#include "WorldMatrix.h"
#include "MathUtility.h"

namespace WorldMatrix {

	void UpdateWorldMatrix(WorldTransform& worldTransform) {
		// スケール・回転・平行移動からワールド行列を作る
		worldTransform.matWorld_ = MakeAffineMatrix(worldTransform.scale_, worldTransform.rotation_, worldTransform.translation_);
		// 定数バッファへ転送
		worldTransform.TransferMatrix();
	}

}