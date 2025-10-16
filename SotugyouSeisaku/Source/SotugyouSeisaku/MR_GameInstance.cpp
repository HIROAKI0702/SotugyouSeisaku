// Fill out your copyright notice in the Description page of Project Settings.


#include "MR_GameInstance.h"
#include "Kismet/GameplayStatics.h"

UMR_GameInstance::UMR_GameInstance()
{
	//デフォルトでステージ1だけアンロック
	//実際のステージ情報はブループリントで設定
}

/// @brief 指定したステージをアンロック状態にする
/// @param StageIndex アンロックしたいステージのインデックス番号
void UMR_GameInstance::UnlockStage(int32 StageIndex)
{
	//配列範囲チェック
	if (mStages.IsValidIndex(StageIndex))
	{
		//指定ステージのアンロックフラグを有効化
		mStages[StageIndex].bIsUnlocked = true;
		UE_LOG(LogTemp, Log, TEXT("Stage %d unlocked: %s"), StageIndex, *mStages[StageIndex].mStageName);
	}
}

/// @brief ステージをクリア状態にする
/// @param StageIndex クリアしたステージのインデックス番号
void UMR_GameInstance::SetStageCleared(int32 StageIndex)
{
	if (mStages.IsValidIndex(StageIndex))
	{
		//クリアフラグを有効化
		mStages[StageIndex].bIsCleared = true;

		//次のステージをアンロック
		if (mStages.IsValidIndex(StageIndex + 1))
		{
			UnlockStage(StageIndex + 1);
		}

		UE_LOG(LogTemp, Log, TEXT("Stage %d cleared: %s"), StageIndex, *mStages[StageIndex].mStageName);
	}
}

/// @brief 指定したステージをロードする
/// @param StageIndex スロードするステージのインデックス番号
void UMR_GameInstance::LoadStage(int32 StageIndex)
{
	if (mStages.IsValidIndex(StageIndex))
	{
		//現在のステージを記録
		mCurrentStageIndex = StageIndex;
		//対応するレベル名を取得
		FName LevelName = mStages[StageIndex].mLevelName;

		UE_LOG(LogTemp, Warning, TEXT("Loading stage: %s"), *LevelName.ToString());

		//レベルを開く
		UGameplayStatics::OpenLevel(this, LevelName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid stage index: %d"), StageIndex);
	}
}

/// @brief タイトル画面に戻る関数
void UMR_GameInstance::ReturnToTitle()
{
	UE_LOG(LogTemp, Log, TEXT("Returning to title screen"));
	UGameplayStatics::OpenLevel(this, FName("TitleScreen"));
}

/// @brief ステージ選択画面に戻る
void UMR_GameInstance::ReturnToStageSelect()
{
	UE_LOG(LogTemp, Log, TEXT("Returning to stage select"));
	UGameplayStatics::OpenLevel(this, FName("StageSelectScreen"));
}