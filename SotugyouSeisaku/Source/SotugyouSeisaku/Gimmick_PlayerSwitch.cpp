// Fill out your copyright notice in the Description page of Project Settings.

#include "Gimmick_PlayerSwitch.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "SotugyouSeisakuCharacter.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "InteractWidget.h"
#include "SwitchManager.h"

// Sets default values

/// @brief コンストラクタ　プレイヤー切り替えギミックの各種設定
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

	//インタラクト可能距離
	mInteractDistance = 200.0f;
}

// Called when the game starts or when spawned
void AGimmick_PlayerSwitch::BeginPlay()
{
	Super::BeginPlay();

	//PlayerAが設定されていない場合、デフォルトポーンを自動取得
	if (!mPlayerA)
	{
		AActor* DefaultPawn = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
		if (DefaultPawn)
		{
			mPlayerA = Cast<ASotugyouSeisakuCharacter>(DefaultPawn);
		}
	}

	//オーバーラップイベントをバインド
	mTriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AGimmick_PlayerSwitch::OnTriggerBeginOverlap);
	mTriggerBox->OnComponentEndOverlap.AddDynamic(this, &AGimmick_PlayerSwitch::OnTriggerEndOverlap);
}

// Called every frame
void AGimmick_PlayerSwitch::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/// @brief インターフェース実装
/// @param PlayerCharacter プレイヤー
void AGimmick_PlayerSwitch::Interact_Implementation(ASotugyouSeisakuCharacter* PlayerCharacter)
{
	if (PlayerCharacter && bPlayerInRange)
	{
		OnInteract(PlayerCharacter);
	}
}

/// @brief インタラクトできるかどうかをチェックする関数
/// @param PlayerCharacter インタラクトするプレイヤー
/// @return プレイヤーかどうかを返す
bool AGimmick_PlayerSwitch::CanInteract_Implementation(ASotugyouSeisakuCharacter* PlayerCharacter) const
{
	if (!PlayerCharacter)
	{
		return false;
	}

	if (!mPlayerA || !mPlayerB)
	{
		return false;
	}

	//距離チェック
	float Distance = FVector::Dist(GetActorLocation(), PlayerCharacter->GetActorLocation());
	if (Distance > mInteractDistance)
	{
		return false;
	}

	// 範囲内にいるかチェック
	return bPlayerInRange;
}

/// @brief インタラクト中にテキストを出す関数
/// @return プレイヤーがインタラクトしているかどうかを返す
FText AGimmick_PlayerSwitch::GetInteractText_Implementation() const
{
	return FText::FromString(TEXT("Switch Player"));
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

		//プレイヤーにこの切り替えポイントを登録
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

		//プレイヤーから切り替えポイントの登録を解除
		Player->SetNearbySwitch(nullptr);

		mCurrentPlayer = nullptr;
	}
}

/// @brief プレイヤーが範囲内でFキーを入力したときに呼ばれる
/// @param InteractingPlayer Fキーを入力したプレイヤークラス
void AGimmick_PlayerSwitch::OnInteract(ASotugyouSeisakuCharacter* InteractingPlayer)
{
	if (!mPlayerA || !mPlayerB)
	{
		return;
	}

	//クールダウンチェック
	float CurrentTime = GetWorld()->GetTimeSeconds();
	float TimeSinceLastSwitch = CurrentTime - mLastSwitchTime;

	if (TimeSinceLastSwitch < mSwitchCooldown)
	{
		float RemainingTime = mSwitchCooldown - TimeSinceLastSwitch;
		return;
	}

	//現在操作中のプレイヤーを判定して、適切な関数を呼ぶ
	if (InteractingPlayer == mPlayerA)
	{
		//PlayerA → PlayerB
		SwitchFromAtoB();
	}
	else if (InteractingPlayer == mPlayerB)
	{
		//PlayerB → PlayerA
		SwitchFromBtoA();
	}
	else
	{
		return;
	}

	//最後の切り替え時刻を更新
	mLastSwitchTime = CurrentTime;
}

/// @brief PlayerAからPlayerBへ切り替える
void AGimmick_PlayerSwitch::SwitchFromAtoB()
{
	if (!mPlayerA || !mPlayerB)
	{
		return;
	}
	PerformSwitch(mPlayerB, mPlayerA);
}

/// @brief PlayerBからPlayerAへ切り替える
void AGimmick_PlayerSwitch::SwitchFromBtoA()
{
	if (!mPlayerA || !mPlayerB)
	{
		return;
	}
	PerformSwitch(mPlayerA, mPlayerB);
}

/// @brief 実際の切り替え処理を行う
/// @param NewPlayer 切り替え先のプレイヤー
/// @param OldPlayer 切り替え前のプレイヤー
void AGimmick_PlayerSwitch::PerformSwitch(ASotugyouSeisakuCharacter* NewPlayer, ASotugyouSeisakuCharacter* OldPlayer)
{
	if (!NewPlayer || !OldPlayer)
	{
		return;
	}

	USwitchManager* Manager = USwitchManager::Get(GetWorld());
	if (Manager->IsRecentlyUsed())
	{
		return;//全スイッチ共通クールタイム中なら何もしない
	}

	Manager->RegisterSwitchUsage();//今使ったことを記録

	//プレイヤーコントローラーを取得
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		return;
	}

	//古いプレイヤーの処理
	if (OldPlayer)
	{
		//ウィジェットを非表示
		if (OldPlayer->GetInteractWidget())
		{
			OldPlayer->GetInteractWidget()->HideWidget();
		}

		//インタラクト可能オブジェクトをクリア
		OldPlayer->mCurrentInteractable.SetObject(nullptr);
		OldPlayer->mCurrentInteractable.SetInterface(nullptr);

		//CharacterMovementの速度をリセット
		if (UCharacterMovementComponent* Movement = OldPlayer->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}

		if (AController* OldController = OldPlayer->GetController())
		{
			//コントローラーを解除
			OldController->UnPossess();
		}
	}

	PC->Possess(NewPlayer);

	//新しいプレイヤーの状態をクリア
	NewPlayer->mCurrentInteractable.SetObject(nullptr);
	NewPlayer->mCurrentInteractable.SetInterface(nullptr);

	//新しいプレイヤーのウィジェットを確実に非表示
	if (NewPlayer->GetInteractWidget())
	{
		NewPlayer->GetInteractWidget()->HideWidget();
	}

	//切り替え直後はインタラクトチェックを少し遅延させる
	FTimerHandle TempHandle;
	GetWorld()->GetTimerManager().SetTimer(TempHandle, [NewPlayer]()
		{
			if (NewPlayer)
			{
				NewPlayer->UpdateInteractUI();
			}
		}, 0.1f, false);
}