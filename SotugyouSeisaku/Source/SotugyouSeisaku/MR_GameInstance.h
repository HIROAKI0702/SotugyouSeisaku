// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MR_GameInstance.generated.h"

// ステージ情報を格納する構造体
USTRUCT(BlueprintType)
struct FStageInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString mStageName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName mLevelName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UTexture2D* mThumbnailImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCleared = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsUnlocked = false;
};

UCLASS()
class SOTUGYOUSEISAKU_API UMR_GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UMR_GameInstance();

	// ステージ情報の配列
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
	TArray<FStageInfo> mStages;

	// 現在のステージインデックス
	UPROPERTY(BlueprintReadWrite, Category = "Stage")
	int32 mCurrentStageIndex;

	// 現在のチェックポイント
	UPROPERTY(BlueprintReadWrite, Category = "CheckPoint")
	int32 mCurrentCheckPoint;

	// ステージをアンロック
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void UnlockStage(int32 StageIndex);

	// ステージをクリア状態にする
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void SetStageCleared(int32 StageIndex);

	// 指定したステージをロード
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void LoadStage(int32 StageIndex);

	// タイトル画面に戻る
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void ReturnToTitle();

	// ステージ選択画面に戻る
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void ReturnToStageSelect();

	// チェックポイントを設定
	UFUNCTION(BlueprintCallable, Category = "CheckPoint")
	void SetCurrentCheckPoint(int32 CheckPointIndex);

	// チェックポイントを取得
	UFUNCTION(BlueprintCallable, Category = "CheckPoint")
	int32 GetCurrentCheckPoint() const { return mCurrentCheckPoint; }

	// チェックポイントからリスポーン
	UFUNCTION(BlueprintCallable, Category = "CheckPoint")
	void RespawnAtCheckPoint();
};