// Fill out your copyright notice in the Description page of Project Settings.

#include "MR_GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "CheckPoint.h"

UMR_GameInstance::UMR_GameInstance()
{
	// デフォルトでステージ1だけアンロック
	mCurrentStageIndex = 0;
	mCurrentCheckPoint = 0;
}

void UMR_GameInstance::UnlockStage(int32 StageIndex)
{
	if (mStages.IsValidIndex(StageIndex))
	{
		mStages[StageIndex].bIsUnlocked = true;
		UE_LOG(LogTemp, Log, TEXT("Stage %d unlocked"), StageIndex);
	}
}

void UMR_GameInstance::SetStageCleared(int32 StageIndex)
{
	if (mStages.IsValidIndex(StageIndex))
	{
		mStages[StageIndex].bIsCleared = true;
		UE_LOG(LogTemp, Log, TEXT("Stage %d cleared"), StageIndex);

		// 次のステージをアンロック
		if (mStages.IsValidIndex(StageIndex + 1))
		{
			UnlockStage(StageIndex + 1);
		}
	}
}

void UMR_GameInstance::LoadStage(int32 StageIndex)
{
	if (mStages.IsValidIndex(StageIndex))
	{
		mCurrentStageIndex = StageIndex;
		mCurrentCheckPoint = 0; // チェックポイントをリセット

		FName LevelName = mStages[StageIndex].mLevelName;
		UGameplayStatics::OpenLevel(this, LevelName);

		UE_LOG(LogTemp, Log, TEXT("Loading stage %d: %s"), StageIndex, *LevelName.ToString());
	}
}

void UMR_GameInstance::ReturnToTitle()
{
	UGameplayStatics::OpenLevel(this, FName("TitleScreen"));
	UE_LOG(LogTemp, Log, TEXT("Returning to Title"));
}

void UMR_GameInstance::ReturnToStageSelect()
{
	UGameplayStatics::OpenLevel(this, FName("StageSelectScreen"));
	UE_LOG(LogTemp, Log, TEXT("Returning to Stage Select"));
}

void UMR_GameInstance::SetCurrentCheckPoint(int32 CheckPointIndex)
{
	mCurrentCheckPoint = CheckPointIndex;
	UE_LOG(LogTemp, Warning, TEXT("Current CheckPoint set to: %d"), CheckPointIndex);
}

void UMR_GameInstance::RespawnAtCheckPoint()
{
	UE_LOG(LogTemp, Warning, TEXT("=== RespawnAtCheckPoint called ==="));
	UE_LOG(LogTemp, Warning, TEXT("Current CheckPoint: %d"), mCurrentCheckPoint);

	// プレイヤーコントローラーを取得
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerController not found!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("PlayerController found"));

	// プレイヤーのポーンを取得
	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerPawn not found!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("PlayerPawn found: %s"), *PlayerPawn->GetName());

	// チェックポイントアクターを見つける
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACheckPoint::StaticClass(), FoundActors);

	UE_LOG(LogTemp, Warning, TEXT("Found %d CheckPoint actors"), FoundActors.Num());

	bool bFoundCheckPoint = false;
	for (AActor* Actor : FoundActors)
	{
		ACheckPoint* CheckPoint = Cast<ACheckPoint>(Actor);
		if (CheckPoint)
		{
			UE_LOG(LogTemp, Warning, TEXT("CheckPoint %d at location: %s"),
				CheckPoint->GetCheckPointIndex(),
				*CheckPoint->GetActorLocation().ToString());

			if (CheckPoint->GetCheckPointIndex() == mCurrentCheckPoint)
			{
				FVector RespawnLocation = CheckPoint->GetActorLocation();
				UE_LOG(LogTemp, Warning, TEXT("Found matching CheckPoint! Respawning at: %s"),
					*RespawnLocation.ToString());

				// プレイヤーをテレポート
				PlayerPawn->SetActorLocation(RespawnLocation);

				UE_LOG(LogTemp, Warning, TEXT("Player respawned at CheckPoint %d"), mCurrentCheckPoint);
				bFoundCheckPoint = true;
				break;
			}
		}
	}

	if (!bFoundCheckPoint)
	{
		UE_LOG(LogTemp, Error, TEXT("CheckPoint %d not found in level!"), mCurrentCheckPoint);
	}
}