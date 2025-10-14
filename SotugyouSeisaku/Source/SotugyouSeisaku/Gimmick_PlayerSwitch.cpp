// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick_PlayerSwitch.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "SotugyouSeisakuCharacter.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AGimmick_PlayerSwitch::AGimmick_PlayerSwitch()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//ルートコンポーネント作成
	mRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = mRoot;

	//床のメッシュ
	mMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
	mMesh->SetupAttachment(RootComponent);

	//トリガーボックス
	mTriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	mTriggerBox->SetupAttachment(RootComponent);
	mTriggerBox->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
	mTriggerBox->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void AGimmick_PlayerSwitch::BeginPlay()
{
	Super::BeginPlay();
	
	//オーバーラップイベントをバインド
	mTriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AGimmick_PlayerSwitch::OnTriggerBeginOverlap);
	mTriggerBox->OnComponentEndOverlap.AddDynamic(this, &AGimmick_PlayerSwitch::OnTriggerEndOverlap);
}

// Called every frame
void AGimmick_PlayerSwitch::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/// @brief プレイヤーがレバーのインタラクション範囲に入った瞬間に呼ばれるオーバーラップイベント
/// @param OverlappedComponent イベントを発生させた自身のコリジョン
/// @param OtherActor トリガー範囲に入ったアクタ
/// @param OtherComp 相手アクタのどのコンポーネントに当たったか
/// @param OtherBodyIndex 複数ボディを持つ場合のインデックス
/// @param bFromSweep 移動による衝突かどうか
/// @param SweepResult 衝突の詳細情報
void AGimmick_PlayerSwitch::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ASotugyouSeisakuCharacter* Player = Cast<ASotugyouSeisakuCharacter>(OtherActor))
	{
		bPlayerInRange = true;
		mCurrentPlayer = Player;

		// プレイヤーにこの切り替えポイントを登録
		Player->SetNearbySwitch(this);
	}
}

/// @brief プレイヤーがレバーのインタラクション範囲から出た瞬間に呼ばれるオーバーラップイベント
/// @param OverlappedComponent イベントを発生させた自身のコリジョン
/// @param OtherActor トリガー範囲に入ったアクタ
/// @param OtherComp 相手アクタのどのコンポーネントに当たったか
/// @param OtherBodyIndex 複数ボディを持つ場合のインデックス
void AGimmick_PlayerSwitch::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ASotugyouSeisakuCharacter* Player = Cast<ASotugyouSeisakuCharacter>(OtherActor))
	{
		bPlayerInRange = false;

		// プレイヤーから切り替えポイントの登録を解除
		Player->SetNearbySwitch(nullptr);

		mCurrentPlayer = nullptr;
	}
}

/// @brief プレイヤーが範囲内でFキーを入力したときに呼ばれる
/// @param InteractingPlayer Fキーを入力したプレイヤークラス
void AGimmick_PlayerSwitch::OnInteract(ASotugyouSeisakuCharacter* InteractingPlayer)
{
	if (!mTargetPlayer)
	{
		return;
	}

	//クールダウンチェック：最後の切り替えから一定時間経っているか
	float CurrentTime = GetWorld()->GetTimeSeconds();
	float TimeSinceLastSwitch = CurrentTime - mLastSwitchTime;

	if (TimeSinceLastSwitch < mSwitchCooldown)
	{
		float RemainingTime = mSwitchCooldown - TimeSinceLastSwitch;

		return;
	}

	//初回使用時：オリジナルプレイヤーを記録
	if (mOriginalPlayer == nullptr)
	{
		mOriginalPlayer = InteractingPlayer;
	}

	//現在操作中のプレイヤーによって切り替え先を決定
	ASotugyouSeisakuCharacter* NextPlayer = nullptr;

	if (InteractingPlayer == mOriginalPlayer)
	{
		//オリジナルプレイヤー → ターゲットプレイヤーへ
		NextPlayer = mTargetPlayer;
	}
	else if (InteractingPlayer == mTargetPlayer)
	{
		//ターゲットプレイヤー → オリジナルプレイヤーへ
		NextPlayer = mOriginalPlayer;
	}
	else
	{
		//想定外のプレイヤー（念のため）
		return;
	}

	//自分自身に切り替えようとしている場合は無視
	if (InteractingPlayer == NextPlayer)
	{
		return;
	}

	SwitchPlayer(NextPlayer, InteractingPlayer);

	//最後の切り替え時刻を更新
	mLastSwitchTime = CurrentTime;
}

/// @brief 操作プレイヤーを入れ替える関数
void AGimmick_PlayerSwitch::SwitchPlayer(ASotugyouSeisakuCharacter* NewPlayer, ASotugyouSeisakuCharacter* OldPlayer)
{
	if (!NewPlayer)
	{
		return;
	}

	//プレイヤーコントローラーを取得
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		return;
	}

	//現在のポーン（プレイヤー）
	APawn* CurrentPawn = PC->GetPawn();

	//修正：切り替え前のプレイヤーの入力をクリア
	if (OldPlayer)
	{
		//CharacterMovementの速度をリセット
		if (UCharacterMovementComponent* Movement = OldPlayer->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}

		//コントローラーを解除
		OldPlayer->GetController()->UnPossess();
	}

	//修正：新しいプレイヤーを操作
	PC->Possess(NewPlayer);
}

