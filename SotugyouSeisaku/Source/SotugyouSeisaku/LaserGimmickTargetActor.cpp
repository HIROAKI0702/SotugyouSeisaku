// Fill out your copyright notice in the Description page of Project Settings.


#include "LaserGimmickTargetActor.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraComponent.h"
#include "MirrorActor.h"
#include "LaserGimmickTargetActor.h"

// Sets default values
ALaserGimmickTargetActor::ALaserGimmickTargetActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//ルートコンポーネント作成
	mRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = mRoot;

	//床のメッシュ
	mMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
	mMesh->SetupAttachment(RootComponent);

	//デフォルト設定
	mDoorMoveOffset = FVector(400.0f, 0.0f, 0.0f);
	mDoorMoveSpeed = 200.0f;
}

// Called when the game starts or when spawned
void ALaserGimmickTargetActor::BeginPlay()
{
	Super::BeginPlay();
	
	//ドアの初期位置を保存
	if (mTargetDoor)
	{
		mDoorOriginalPosition = mTargetDoor->GetActorLocation();
		mDoorTargetPosition = mDoorOriginalPosition + mDoorMoveOffset;
	}
}

// Called every frame
void ALaserGimmickTargetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//光が当たっている間のみドアを動かす
	if (mTargetDoor)
	{
		MoveDoor(DeltaTime);
	}
}

/// @brief 光が当たった時に呼ばれる
void ALaserGimmickTargetActor::OnLightHit()
{
	if (!bIsLit)
	{
		bIsLit = true;
		bDoorOpen = true; // ドアを開く
	}

	// 光が当たり続けている間はタイマーをリセット
	GetWorldTimerManager().ClearTimer(LightResetTimer);
	GetWorldTimerManager().SetTimer(
		LightResetTimer,
		this,
		&ALaserGimmickTargetActor::OnLightLost,
		0.5f, // 0.5秒光が当たらなければ消灯とみなす
		false
	);
}

/// @brief 光が一定時間当たらなくなった時に呼ばれる
void ALaserGimmickTargetActor::OnLightLost()
{
	bIsLit = false;
	bDoorOpen = false; // ドアを閉じる
}

/// @brief ドアを動かす関数
/// @param DeltaTime フレーム間の経過時間
void ALaserGimmickTargetActor::MoveDoor(float DeltaTime)
{
	FVector CurrentPosition = mTargetDoor->GetActorLocation();
	FVector TargetPosition;

	//ドアの状態に応じて目標位置を決定
	if (bDoorOpen)
	{
		TargetPosition = mDoorTargetPosition;//開く
	}
	else
	{
		TargetPosition = mDoorOriginalPosition;//閉じる
	}

	//滑らかに移動
	FVector NewPosition = FMath::VInterpConstantTo(
		CurrentPosition,
		TargetPosition,
		DeltaTime,
		mDoorMoveSpeed
	);

	mTargetDoor->SetActorLocation(NewPosition);
}