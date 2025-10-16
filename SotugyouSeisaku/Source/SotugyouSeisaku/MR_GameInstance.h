// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MR_GameInstance.generated.h"

//ステージ情報の構造体
USTRUCT(BlueprintType)
struct FStageInfo
{
	GENERATED_BODY()

	//ステージ名
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString mStageName;

	//ステージのレベル名（マップファイル名）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName mLevelName;

	//ステージのサムネイル画像
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> mThumbnailImage;

	//ステージがクリア済みか
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCleared = false;

	//ステージがアンロックされているか
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsUnlocked = false;
};

/**
 * 
 */
UCLASS()
class SOTUGYOUSEISAKU_API UMR_GameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UMR_GameInstance();

	//ステージ一覧
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stages")
	TArray<FStageInfo> mStages;

	//現在選択されているステージインデックス
	UPROPERTY(BlueprintReadWrite, Category = "Stages")
	int32 mCurrentStageIndex = 0;

	//ステージをアンロック
	UFUNCTION(BlueprintCallable, Category = "Stages")
	void UnlockStage(int32 StageIndex);

	//ステージをクリア済みにする
	UFUNCTION(BlueprintCallable, Category = "Stages")
	void SetStageCleared(int32 StageIndex);

	//ステージをロード
	UFUNCTION(BlueprintCallable, Category = "Stages")
	void LoadStage(int32 StageIndex);

	//タイトル画面に戻る
	UFUNCTION(BlueprintCallable, Category = "Stages")
	void ReturnToTitle();

	//ステージ選択画面に戻る
	UFUNCTION(BlueprintCallable, Category = "Stages")
	void ReturnToStageSelect();
};
