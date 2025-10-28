// Fill out your copyright notice in the Description page of Project Settings.


#include "RespawnManager.h"
#include "SotugyouSeisakuCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Gimmick_FallFloorManager.h"

// Sets default values
ARespawnManager::ARespawnManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//デフォルトで3つの設定枠を用意
	mCharacterSettings.SetNum(3);
}

// Called when the game starts or when spawned
void ARespawnManager::BeginPlay()
{
	Super::BeginPlay();
	
	//デフォルトのリスポーン地点を検索（設定されていない場合）
	if (!mDefaultRespawnPoint)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);

		if (PlayerStarts.Num() > 0)
		{
			mDefaultRespawnPoint = Cast<APlayerStart>(PlayerStarts[0]);
		}
	}

	// FallFloorManagerを検索
	TArray<AActor*> FoundManagers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGimmick_FallFloorManager::StaticClass(), FoundManagers);

	if (FoundManagers.Num() > 0)
	{
		mFloorManager = Cast<AGimmick_FallFloorManager>(FoundManagers[0]);
	}
}

// Called every frame
void ARespawnManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//各キャラクターのリスポーンチェック
	CheckCharacterRespawns();
}

/// @brief 各キャラクターのリスポーン処理をチェック
void ARespawnManager::CheckCharacterRespawns()
{
	for (int32 i = 0; i < mCharacterSettings.Num(); i++)
	{
		FCharacterRespawnSettings& Settings = mCharacterSettings[i];

		//無効な設定またはキャラクターが設定されていない場合はスキップ
		if (!Settings.bEnabled || !Settings.TargetCharacter)
		{
			continue;
		}

		//キャラクターの現在のZ座標を取得
		float CurrentZ = Settings.TargetCharacter->GetActorLocation().Z;

		//閾値を下回ったらリスポーン
		if (CurrentZ < Settings.RespawnThresholdZ)
		{
			//リスポーン地点を決定（カスタム地点 → デフォルト地点の順）
			APlayerStart* RespawnPoint = Settings.CustomRespawnPoint ?
				Settings.CustomRespawnPoint : mDefaultRespawnPoint;

			if (RespawnPoint)
			{
				RespawnCharacter(Settings.TargetCharacter, RespawnPoint);
			}
		}
	}
}

/// @brief 指定されたキャラクターをリスポーンさせる
/// @param Character リスポーンさせるキャラクター
/// @param RespawnPoint リスポーン地点
void ARespawnManager::RespawnCharacter(ASotugyouSeisakuCharacter* Character, APlayerStart* RespawnPoint)
{
	if (!Character || !RespawnPoint)
	{
		return;
	}

	//位置と向きをリセット
	Character->SetActorLocation(RespawnPoint->GetActorLocation());
	Character->SetActorRotation(RespawnPoint->GetActorRotation());

	//速度をリセット
	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->Velocity = FVector::ZeroVector;
		MoveComp->StopMovementImmediately();
	}

	//落下床を再生成
	if (mFloorManager)
	{
		mFloorManager->RespawnAllFloors();
	}
}

