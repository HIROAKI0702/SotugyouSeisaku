// Fill out your copyright notice in the Description page of Project Settings.

#include "Gimmck_MoveFloor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"

/// @brief コンストラクタ　動く床の各種設定
AGimmck_MoveFloor::AGimmck_MoveFloor()
{
	PrimaryActorTick.bCanEverTick = true;

	//ルートコンポーネント作成
	mRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = mRoot;

	//床のメッシュ
	mMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
	mMesh->SetupAttachment(RootComponent);

	//Movableに設定
	mMesh->SetMobility(EComponentMobility::Movable);

	//コリジョン設定
	mMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	mMesh->SetCollisionObjectType(ECC_WorldStatic);
	mMesh->SetCollisionResponseToAllChannels(ECR_Block);
	mMesh->SetGenerateOverlapEvents(true);
	mMesh->SetSimulatePhysics(false);

	//デフォルト設定
	mMovementPattern = EFloorMovementPattern::Horizontal_Y;
	mMoveDistance = 500.0f;
	mCustomMoveOffset = FVector(0.0f, 500.0f, 0.0f);
	mMoveSpeed = 200.0f;
	mWaitTime = 1.0f;
	bAutoStart = true;
	bMoveActorsOnFloor = true;
}

void AGimmck_MoveFloor::BeginPlay()
{
	Super::BeginPlay();

	//開始位置を保存
	mStartPosition = GetActorLocation();

	//パターンに応じて終了位置を計算
	CalculateEndPosition();

	// オーバーラップイベントをバインド
	if (mMesh)
	{
		mMesh->OnComponentBeginOverlap.AddDynamic(this, &AGimmck_MoveFloor::OnFloorBeginOverlap);
		mMesh->OnComponentEndOverlap.AddDynamic(this, &AGimmck_MoveFloor::OnFloorEndOverlap);
	}

	// 自動開始しない場合は待機状態
	if (!bAutoStart)
	{
		bIsWaiting = true;
	}
}

/// @brief 移動パターンのよって終了位置を計算する関数
void AGimmck_MoveFloor::CalculateEndPosition()
{
	switch (mMovementPattern)
	{
	case EFloorMovementPattern::Horizontal_X://X軸方向への水平移動
		mEndPosition = mStartPosition + FVector(mMoveDistance, 0.0f, 0.0f);
		break;

	case EFloorMovementPattern::Horizontal_Y://Y軸方向への水平移動
		mEndPosition = mStartPosition + FVector(0.0f, mMoveDistance, 0.0f);
		break;

	case EFloorMovementPattern::Vertical_Z://Z軸方向への垂直移動
		mEndPosition = mStartPosition + FVector(0.0f, 0.0f, mMoveDistance);
		break;

	case EFloorMovementPattern::Diagonal_XY://XY平面上の斜め移動
	{
		float HalfDist = mMoveDistance / FMath::Sqrt(2.0f);
		mEndPosition = mStartPosition + FVector(HalfDist, HalfDist, 0.0f);
	}
	break;

	case EFloorMovementPattern::Diagonal_XZ://XZ平面上の斜め移動
	{
		float HalfDist = mMoveDistance / FMath::Sqrt(2.0f);
		mEndPosition = mStartPosition + FVector(HalfDist, 0.0f, HalfDist);
	}
	break;

	case EFloorMovementPattern::Diagonal_YZ://YZ平面上の斜め移動
	{
		float HalfDist = mMoveDistance / FMath::Sqrt(2.0f);
		mEndPosition = mStartPosition + FVector(0.0f, HalfDist, HalfDist);
	}
	break;
	//円運動
	case EFloorMovementPattern::Circle_XY:
	case EFloorMovementPattern::Circle_XZ:
	case EFloorMovementPattern::Circle_YZ:
		//円運動の場合、中心位置は開始位置
		mCircleCenter = mStartPosition;
		mCircleAngle = 0.0f;
		break;

	case EFloorMovementPattern::Custom:
		mEndPosition = mStartPosition + mCustomMoveOffset;
		break;
	}
}

void AGimmck_MoveFloor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//円運動かどうかで処理を分岐
	bool bIsCircular = (mMovementPattern == EFloorMovementPattern::Circle_XY ||
		mMovementPattern == EFloorMovementPattern::Circle_XZ ||
		mMovementPattern == EFloorMovementPattern::Circle_YZ);

	if (bIsCircular)
	{
		UpdateCircularMovement(DeltaTime);
	}
	else
	{
		UpdateLinearMovement(DeltaTime);
	}
}

/// @brief 往復移動の処理関数
/// @param DeltaTime フレーム間の経過時間
void AGimmck_MoveFloor::UpdateLinearMovement(float DeltaTime)
{
	//待機中の処理
	if (bIsWaiting)
	{
		mWaitTimer += DeltaTime;

		if (mWaitTimer >= mWaitTime)
		{
			bIsWaiting = false;
			mWaitTimer = 0.0f;
			mDirection *= -1;
		}

		return;
	}

	//現在位置と目標位置の取得
	FVector CurrentPosition = GetActorLocation();
	FVector TargetPosition = (mDirection == 1) ? mEndPosition : mStartPosition;

	//滑らかに移動
	FVector NewPosition = FMath::VInterpConstantTo(
		CurrentPosition,
		TargetPosition,
		DeltaTime,
		mMoveSpeed
	);

	//移動量を計算
	FVector DeltaMove = NewPosition - CurrentPosition;

	//位置を更新
	SetActorLocation(NewPosition);

	//床の上のアクターも一緒に移動
	if (bMoveActorsOnFloor && !DeltaMove.IsNearlyZero())
	{
		for (AActor* Actor : mActorsOnFloor)
		{
			if (Actor)
			{
				Actor->AddActorWorldOffset(DeltaMove, false);
			}
		}
	}

	//目標位置に到達したかチェック
	float DistanceToTarget = FVector::Dist(NewPosition, TargetPosition);

	//目的地の到着したかチェック
	if (DistanceToTarget < 1.0f)
	{
		SetActorLocation(TargetPosition);
		bIsWaiting = true;
		mWaitTimer = 0.0f;
	}
}

/// @brief 円運動の処理関数
/// @param DeltaTime フレーム間の経過時間
void AGimmck_MoveFloor::UpdateCircularMovement(float DeltaTime)
{
	FVector CurrentPosition = GetActorLocation();

	//角速度を計算（速度 / 半径）
	float AngularSpeed = mMoveSpeed / mMoveDistance;
	mCircleAngle += AngularSpeed * DeltaTime;

	//角度を0〜2πの範囲に保つ
	if (mCircleAngle >= 2.0f * PI)
	{
		mCircleAngle -= 2.0f * PI;
	}

	//パターンに応じて新しい位置を計算
	FVector NewPosition = mCircleCenter;

	switch (mMovementPattern)
	{
	case EFloorMovementPattern::Circle_XY:
		NewPosition.X += FMath::Cos(mCircleAngle) * mMoveDistance;
		NewPosition.Y += FMath::Sin(mCircleAngle) * mMoveDistance;
		break;

	case EFloorMovementPattern::Circle_XZ:
		NewPosition.X += FMath::Cos(mCircleAngle) * mMoveDistance;
		NewPosition.Z += FMath::Sin(mCircleAngle) * mMoveDistance;
		break;

	case EFloorMovementPattern::Circle_YZ:
		NewPosition.Y += FMath::Cos(mCircleAngle) * mMoveDistance;
		NewPosition.Z += FMath::Sin(mCircleAngle) * mMoveDistance;
		break;
	}

	//移動量を計算
	FVector DeltaMove = NewPosition - CurrentPosition;

	//位置を更新
	SetActorLocation(NewPosition);

	//床の上のアクターも一緒に移動
	if (bMoveActorsOnFloor && !DeltaMove.IsNearlyZero())
	{
		for (AActor* Actor : mActorsOnFloor)
		{
			if (Actor)
			{
				Actor->AddActorWorldOffset(DeltaMove, false);
			}
		}
	}
}
/// @brief 床にアクターが乗った時に呼ばれるオーバーラップイベント
/// @param OverlappedComponent イベントが発生したコンポーネント
/// @param OtherActor 重なったアクター
/// @param OtherComp 重なったアクターのコンポーネント
/// @param OtherBodyIndex オーバーラップボディのインデックス
/// @param bFromSweep スウィープ結果によるものか
/// @param SweepResult 衝突時のヒット情報
void AGimmck_MoveFloor::OnFloorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//自分自身や無効なアクタは無視
	if (OtherActor == this || !OtherActor)
	{
		return;
	}

	if (!mActorsOnFloor.Contains(OtherActor))
	{
		mActorsOnFloor.Add(OtherActor);
	}
}

/// @brief 床からアクターが降りた時に呼ばれるオーバーラップイベント
/// @param OverlappedComponent イベントが発生したコンポーネント
/// @param OtherActor 離れたアクター
/// @param OtherComp 離れたアクターのコンポーネント
/// @param OtherBodyIndex オーバーラップボディのインデックス
void AGimmck_MoveFloor::OnFloorEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//自分自身や無効なアクタは無視
	if (OtherActor == this || !OtherActor)
	{
		return;
	}

	if (mActorsOnFloor.Contains(OtherActor))
	{
		mActorsOnFloor.Remove(OtherActor);
	}
}