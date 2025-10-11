// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick_Button.h"
#include "Components/StaticMeshComponent.h"
#include "Gimmick_ButtonManager.h"
#include "Gimmick_PushBlock.h"

// Sets default values

/// @brief コンストラクタ　ボタンの各種設定
AGimmick_Button::AGimmick_Button()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//ルートコンポーネント作成
	mRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = mRoot;

	//ボタンのメッシュ
	mMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ButtonMesh"));
	mMesh->SetupAttachment(RootComponent);
	mMesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ButtonBaseMesh"));
	mMesh2->SetupAttachment(RootComponent);

	//トリガーボックス（ボタンの判定エリア）
	mTriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	mTriggerBox->SetupAttachment(RootComponent);
	mTriggerBox->SetBoxExtent(FVector(60.0f, 60.0f, 25.0f));//サイズ調整可能
	mTriggerBox->SetCollisionResponseToAllChannels(ECR_Overlap);

	//デフォルト設定
	mMoveDir = FVector(400.0f, 0.0f, 0.0f);
	mMoveSpeed = 300.0f;
	bReturnToOriginal = true;
}

// Called when the game starts or when spawned
void AGimmick_Button::BeginPlay()
{
	Super::BeginPlay();

	//オーバーラップイベントをバインド
	mTriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AGimmick_Button::OnTriggerBeginOverlap);
	mTriggerBox->OnComponentEndOverlap.AddDynamic(this, &AGimmick_Button::OnTriggerEndOverlap);

	//ターゲットブロックの初期位置を保存
	if (mTargetDoor)
	{
		mBlockOriginalPosition = mTargetDoor->GetActorLocation();
		mBlockTargetPosition = mBlockOriginalPosition + mMoveDir;	
	}
}

// Called every frame
void AGimmick_Button::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//ブロックを目標位置に向かって移動
	if (mTargetDoor)
	{
		MoveBlock(DeltaTime);
	}
}

/// @brief プレイヤーがボタンを踏んだかをチェックする
/// @param OverlappedComponent イベントを発生させた自身のコリジョン
/// @param OtherActor トリガー範囲に入ったアクタ
/// @param OtherComp 相手アクタのどのコンポーネントに当たったか
/// @param OtherBodyIndex 複数ボディを持つ物理コンポーネント向けのインデックス番号
/// @param bFromSweep 移動による衝突によって発生したか
/// @param SweepResult 衝突の詳細情報
void AGimmick_Button::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//自分自身や無効なアクタは無視
	if (OtherActor && OtherActor != this)
	{
		mOverlappingActorCount++;

		if (!bIsPressed)
		{
			bIsPressed = true;

			//ボタンのメッシュを少し下げる
			if (mMesh)
			{
				FVector NewLocation = mMesh->GetRelativeLocation();
				NewLocation.Z -= 10.0f;//5cm下げる
				mMesh->SetRelativeLocation(NewLocation);
			}

			//ボタンマネージャーに通知（複数ボタンシステム）
			if (mButtonManager)
			{
				mButtonManager->OnButtonPressed(this);
			}
		}
	}
}

/// @brief プレイヤーがボタンから離れたときの終了処理
/// @param OverlappedComponent イベントを発生させた自身のコリジョン
/// @param OtherActor トリガー範囲から出たアクタ
/// @param OtherComp 相手アクタのどのコンポーネントに当たったか
/// @param OtherBodyIndex 複数ボディを持つ物理コンポーネント向けのインデックス番号
void AGimmick_Button::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//自分自身や無効なアクタは無視
	if (OtherActor && OtherActor != this)
	{
		mOverlappingActorCount--;

		//誰も乗っていない場合
		if (mOverlappingActorCount <= 0)
		{
			mOverlappingActorCount = 0;
			bIsPressed = false;

			//ボタンのメッシュを元に戻す
			if (mMesh)
			{
				FVector NewLocation = mMesh->GetRelativeLocation();
				NewLocation.Z += 10.0f; //元に戻す
				mMesh->SetRelativeLocation(NewLocation);
			}

			//ボタンマネージャーに通知
			if (mButtonManager)
			{
				mButtonManager->OnButtonReleased(this);
			}
		}
	}
}

/// @brief ブロックを動かす
/// @param DeltaTime 移動する時間
void AGimmick_Button::MoveBlock(float DeltaTime)
{
	if (!mTargetDoor)
		return;

	FVector CurrentPosition = mTargetDoor->GetActorLocation();
	FVector TargetPosition;

	//ボタンが押されているかどうかで目標位置を決定
	if (bIsPressed)
	{
		//押されている → 移動先へ
		TargetPosition = mBlockTargetPosition;
	}
	else
	{
		//押されていない → 元の位置へ（設定による）
		if (bReturnToOriginal)
		{
			TargetPosition = mBlockOriginalPosition;
		}
		else
		{
			//戻らない設定なら現在位置を維持
			return;
		}
	}

	//滑らかに移動
	FVector NewPosition = FMath::VInterpConstantTo(
		CurrentPosition,
		TargetPosition,
		DeltaTime,
		mMoveSpeed
	);

	mTargetDoor->SetActorLocation(NewPosition);

	//デバッグ：目標位置に到達したらログ出力
	if (FVector::Dist(NewPosition, TargetPosition) < 1.0f)
	{
		static bool bReachedTarget = false;
		if (!bReachedTarget)
		{
			UE_LOG(LogTemp, Log, TEXT("Block reached target position"));
			bReachedTarget = true;
		}
	}
	else
	{
		static bool bReachedTarget = true;
		bReachedTarget = false;
	}
}
