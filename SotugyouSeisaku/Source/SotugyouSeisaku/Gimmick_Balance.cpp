// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick_Balance.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Gimmick_PushBlock.h"
#include "DrawDebugHelpers.h"
#include "SotugyouSeisakuCharacter.h"

// Sets default values

/// @brief コンストラクタ　各種設定
AGimmick_Balance::AGimmick_Balance()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//ルート
	mRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = mRoot;

	//左の量り
	mLeftPlateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftScale"));
	mLeftPlateMesh->SetupAttachment(RootComponent);
	mLeftPlateMesh->SetRelativeLocation(FVector(0.0f, -300.0f, 0.0f)); // 左側

	//右の量り
	mRightPlateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightScale"));
	mRightPlateMesh->SetupAttachment(RootComponent);
	mRightPlateMesh->SetRelativeLocation(FVector(0.0f, 300.0f, 0.0f)); // 右側

	//左のトリガー
	mLeftTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftTrigger"));
	mLeftTrigger->SetupAttachment(mLeftPlateMesh);
	mLeftTrigger->SetBoxExtent(FVector(50.0f, 50.0f, 200.0f));
	mLeftTrigger->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f)); // 量りの少し上
	mLeftTrigger->SetGenerateOverlapEvents(true);

	//右のトリガー
	mRightTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("RightTrigger"));
	mRightTrigger->SetupAttachment(mRightPlateMesh);
	mRightTrigger->SetBoxExtent(FVector(50.0f, 50.0f, 200.0f));
	mRightTrigger->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f)); // 量りの少し上
	mRightTrigger->SetGenerateOverlapEvents(true);

	// デフォルト設定
	mDoorMoveOffset = FVector(0.0f, 0.0f, 300.0f);
	mDoorMoveSpeed = 200.0f;
	mBalanceTolerance = 50.0f;
}

// Called when the game starts or when spawned
void AGimmick_Balance::BeginPlay()
{
	Super::BeginPlay();

	//ドアの初期位置を保存
	if (mTargetDoor)
	{
		mDoorOriginalPosition = mTargetDoor->GetActorLocation();
		mDoorTargetPosition = mDoorOriginalPosition + mDoorMoveOffset;
	}

	//オーバーラップイベントをバインド
	mLeftTrigger->OnComponentBeginOverlap.AddDynamic(this, &AGimmick_Balance::OnLeftPlateBeginOverlap);
	mLeftTrigger->OnComponentEndOverlap.AddDynamic(this, &AGimmick_Balance::OnLeftPlateEndOverlap);

	mRightTrigger->OnComponentBeginOverlap.AddDynamic(this, &AGimmick_Balance::OnRightPlateBeginOverlap);
	mRightTrigger->OnComponentEndOverlap.AddDynamic(this, &AGimmick_Balance::OnRightPlateEndOverlap);
}

// Called every frame
void AGimmick_Balance::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//重さを更新
	UpdateWeights();

	//釣り合いをチェック
	CheckBalance();

	//ドアを動かす
	if (mTargetDoor)
	{
		MoveDoor(DeltaTime);
	}
}

/// @brief 左の量りにブロックを乗せた瞬間に呼ばれるオーバーラップイベント
///        一定時間後に床を落下させる。
/// @param OverlappedComponent イベントを発生させた自身のコリジョン
/// @param OtherActor トリガー範囲に入ったアクタ
/// @param OtherComp 相手アクタのどのコンポーネントに当たったか
/// @param OtherBodyIndex 複数ボディを持つ場合のインデックス
/// @param bFromSweep 移動による衝突かどうか
/// @param SweepResult 衝突の詳細情報
void AGimmick_Balance::OnLeftPlateBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AGimmick_PushBlock* Block = Cast<AGimmick_PushBlock>(OtherActor))
	{
		if (!mLeftBlocks.Contains(Block))
		{
			mLeftBlocks.Add(Block);
		}
	}
}

/// @brief 左の量りからブロックを降ろした瞬間に呼ばれるオーバーラップイベント
///        一定時間後に床を落下させる。
/// @param OverlappedComponent イベントを発生させた自身のコリジョン
/// @param OtherActor トリガー範囲に入ったアクタ
/// @param OtherComp 相手アクタのどのコンポーネントに当たったか
/// @param OtherBodyIndex 複数ボディを持つ場合のインデックス
/// @param bFromSweep 移動による衝突かどうか
/// @param SweepResult 衝突の詳細情報
void AGimmick_Balance::OnLeftPlateEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AGimmick_PushBlock* Block = Cast<AGimmick_PushBlock>(OtherActor))
	{
		if (mLeftBlocks.Contains(Block))
		{
			mLeftBlocks.Remove(Block);
		}
	}
}

/// @brief 右の量りにブロックを乗せた瞬間に呼ばれるオーバーラップイベント
///        一定時間後に床を落下させる。
/// @param OverlappedComponent イベントを発生させた自身のコリジョン
/// @param OtherActor トリガー範囲に入ったアクタ
/// @param OtherComp 相手アクタのどのコンポーネントに当たったか
/// @param OtherBodyIndex 複数ボディを持つ場合のインデックス
/// @param bFromSweep 移動による衝突かどうか
/// @param SweepResult 衝突の詳細情報
void AGimmick_Balance::OnRightPlateBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AGimmick_PushBlock* Block = Cast<AGimmick_PushBlock>(OtherActor))
	{
		if (!mRightBlocks.Contains(Block))
		{
			mRightBlocks.Add(Block);
		}
	}
}

/// @brief 右の量りからブロックを降ろした瞬間に呼ばれるオーバーラップイベント
///        一定時間後に床を落下させる。
/// @param OverlappedComponent イベントを発生させた自身のコリジョン
/// @param OtherActor トリガー範囲に入ったアクタ
/// @param OtherComp 相手アクタのどのコンポーネントに当たったか
/// @param OtherBodyIndex 複数ボディを持つ場合のインデックス
/// @param bFromSweep 移動による衝突かどうか
/// @param SweepResult 衝突の詳細情報
void AGimmick_Balance::OnRightPlateEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AGimmick_PushBlock* Block = Cast<AGimmick_PushBlock>(OtherActor))
	{
		if (mRightBlocks.Contains(Block))
		{
			mRightBlocks.Remove(Block);
		}
	}
}

/// @brief 量りに乗ったオブジェクトの重さを更新する関数
void AGimmick_Balance::UpdateWeights()
{
	//左の量りの重さを計算
	mLeftWeight = 0.0f;
	for (AGimmick_PushBlock* Block : mLeftBlocks)
	{
		if (Block)
		{
			mLeftWeight += Block->GetBlockWeight();
		}
	}

	//右の量りの重さを計算
	mRightWeight = 0.0f;
	for (AGimmick_PushBlock* Block : mRightBlocks)
	{
		if (Block)
		{
			mRightWeight += Block->GetBlockWeight();
		}
	}
}

/// @brief 重さが釣り合っているか判別する関数
void AGimmick_Balance::CheckBalance()
{
	//重さの差
	float WeightDifference = FMath::Abs(mLeftWeight - mRightWeight);

	//前回の状態を保存
	bool bWasBalanced = bIsBalanced;

	//ブロックが乗っているかチェック
	bool bHasLeft = (mLeftWeight > 0.0f);
	bool bHasRight = (mRightWeight > 0.0f);

	//両方にブロックがある場合のみ釣り合い判定を行う
	if (bHasLeft && bHasRight)
	{
		bIsBalanced = (WeightDifference <= mBalanceTolerance);
	}
	else
	{
		//どちらかが空なら釣り合っていない
		bIsBalanced = false;
	}

	//状態が変化したときのみドア操作
	if (bIsBalanced && !bWasBalanced)
	{
		bDoorOpen = true; //釣り合ったら開く
	}
	else if (!bIsBalanced && bWasBalanced)
	{
		bDoorOpen = false;//崩れたら閉じる
	}
}

/// @brief ドアを動かす関数
/// @param DeltaTime フレーム間の経過時間
void AGimmick_Balance::MoveDoor(float DeltaTime)
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
