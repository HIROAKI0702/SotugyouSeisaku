// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick_FallFloor.h"
#include "GameFramework/Character.h"
#include "Gimmick_FallFloorManager.h"

// Sets default values

/// @brief コンストラクタ　落ちる床の各種設定
AGimmick_FallFloor::AGimmick_FallFloor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//ルートコンポーネント作成
	mRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = mRoot;

	//床のメッシュ
	mMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
	mMesh->SetupAttachment(RootComponent);

	//トリガーボックスの設定
	mTriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	mTriggerBox->SetupAttachment(RootComponent);
	mTriggerBox->SetBoxExtent(FVector(50.0f, 50.0f, 20.0f));
	mTriggerBox->SetCollisionResponseToAllChannels(ECR_Overlap);

	// デフォルト値を設定
	mShakeAmplitude = 5.0f;  //揺れの強さ
	mShakeFrequency = 20.0f; //揺れの速さ
	mDeleteDelay = 1.0f;
	mShakeTimer = 0.0f;
	bIsShaking = false;
}

// Called when the game starts or when spawned
void AGimmick_FallFloor::BeginPlay()
{
	Super::BeginPlay();

	//オーバーラップイベントをバインド
	mTriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AGimmick_FallFloor::OnTriggerBeginOverlap);

	//床のメッシュの元の位置を保存
	mOriginalMeshLocation = mMesh->GetRelativeLocation();
}

// Called every frame
void AGimmick_FallFloor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//揺れている間（警告アニメーション）
	if (bIsShaking)
	{
		mShakeTimer += DeltaTime;
		FVector ShakeOffset;
		ShakeOffset.X = FMath::Sin(mShakeTimer * mShakeFrequency) * mShakeAmplitude;
		ShakeOffset.Y = FMath::Cos(mShakeTimer * mShakeFrequency) * mShakeAmplitude;

		//床を揺らす
		mMesh->SetRelativeLocation(mOriginalMeshLocation + ShakeOffset);
	}
}

/// @brief プレイヤーなどが床に乗った瞬間に呼ばれるイベント。
///        一定時間後に床を削除する。
/// @param OverlappedComponent イベントを発生させた自身のコリジョン
/// @param OtherActor トリガー範囲に入ったアクタ
/// @param OtherComp 相手アクタのどのコンポーネントに当たったか
/// @param OtherBodyIndex 複数ボディを持つ場合のインデックス
/// @param bFromSweep 移動による衝突かどうか
/// @param SweepResult 衝突の詳細情報
void AGimmick_FallFloor::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//自分自身や無効なアクタは無視
	if (!OtherActor || OtherActor == this)
		return;

	//すでに動作中なら無視
	if (bIsShaking)
		return;

	bIsShaking = true;
	mShakeTimer = 0.0f;

	//一定時間後に床を削除
	GetWorldTimerManager().SetTimer(DeleteTimerHandle, this, &AGimmick_FallFloor::DeleteFloor, mDeleteDelay, false);
}

/// @brief 床の削除を開始する。Tickで位置を更新するようになる。
void AGimmick_FallFloor::DeleteFloor()
{
	//マネージャーに破壊を通知
	if (mFloorManager)
	{
		mFloorManager->OnFloorDestroyed(this);
	}

	Destroy();//床を削除 → プレイヤーは落下
}

