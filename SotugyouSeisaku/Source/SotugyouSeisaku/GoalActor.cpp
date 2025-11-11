// Fill out your copyright notice in the Description page of Project Settings.


#include "GoalActor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "SotugyouSeisakuCharacter.h"
#include "GoalWidget.h"
#include "Blueprint/UserWidget.h"

// Sets default values
AGoalActor::AGoalActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//ルートコンポーネント作成
	mRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = mRoot;

	//ゴールのメッシュ
	mMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GoalMesh"));
	mMesh->SetupAttachment(RootComponent);

	//トリガーボックス（ゴール判定エリア）
	mTriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	mTriggerBox->SetupAttachment(RootComponent);
	mTriggerBox->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
	mTriggerBox->SetGenerateOverlapEvents(true);

	//コリジョン設定
	mTriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	mTriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	mTriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

// Called when the game starts or when spawned
void AGoalActor::BeginPlay()
{
	Super::BeginPlay();
	
	//オーバーラップイベントをバインド
	mTriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AGoalActor::OnTriggerBeginOverlap);

	//ゴールウィジェットを作成
	if (mGoalWidgetClass)
	{
		mGoalWidget = CreateWidget<UGoalWidget>(GetWorld(), mGoalWidgetClass);
		if (mGoalWidget)
		{
			mGoalWidget->AddToViewport(100);//最前面に表示
			mGoalWidget->HideGoalWidget();
		}
	}
}

// Called every frame
void AGoalActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

/// @brief プレイヤーがゴールエリアに入った時に呼ばれる
/// @param OverlappedComponent イベントを発生させた自身のコリジョン
/// @param OtherActor トリガー範囲に入ったアクタ
/// @param OtherComp 相手アクタのどのコンポーネントに当たったか
/// @param OtherBodyIndex 複数ボディを持つ場合のインデックス
/// @param bFromSweep 移動による衝突かどうか
/// @param SweepResult 衝突の詳細情報
void AGoalActor::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//既にゴールしている場合は無視
	if (bGoalReached)
	{
		return;
	}

	//プレイヤーキャラクターかチェック
	if (ASotugyouSeisakuCharacter* Player = Cast<ASotugyouSeisakuCharacter>(OtherActor))
	{
		bGoalReached = true;

		//プレイヤーの操作を無効化
		APlayerController* PC = Cast<APlayerController>(Player->GetController());
		if (PC)
		{
			//プレイヤーの入力を無効化
			Player->DisableInput(PC);

			//カメラ回転も無効化
			PC->SetIgnoreLookInput(true);
			PC->SetIgnoreMoveInput(true);

			//キャラクターの動きを止める
			if (UCharacterMovementComponent* Movement = Player->GetCharacterMovement())
			{
				Movement->StopMovementImmediately();
				Movement->DisableMovement();
			}
		}

		//ゴールウィジェットを表示
		if (mGoalWidget)
		{
			mGoalWidget->ShowGoalWidget();
		}
	}
}

