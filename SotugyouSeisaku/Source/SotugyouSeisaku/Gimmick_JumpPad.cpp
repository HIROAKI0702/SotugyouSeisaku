// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick_JumpPad.h"
#include "SotugyouSeisakuCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values

/// @brief コンストラクタ　ジャンプパッドの各種設定
AGimmick_JumpPad::AGimmick_JumpPad()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//ルートコンポーネント作成
	mRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = mRoot;

	//ジャンプパッドのメッシュ
	mMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("JumpPadMesh"));
	mMesh->SetupAttachment(RootComponent);
	mMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

}

// Called when the game starts or when spawned
void AGimmick_JumpPad::BeginPlay()
{
	Super::BeginPlay();
	
	if (mMesh)
	{
		mMesh->OnComponentBeginOverlap.AddDynamic(this, &AGimmick_JumpPad::OnPadOverlap);
	}
}

// Called every frame
void AGimmick_JumpPad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

/// @brief プレイヤーがジャンプパッドの範囲内に入ってきた瞬間呼ばれるおーばラップイベント
/// @param OverlappedComponent イベントを発生させた自身のコリジョン
/// @param OtherActor トリガー範囲に入ったアクタ
/// @param OtherComp 相手アクタのどのコンポーネントに当たったか
/// @param OtherBodyIndex 複数ボディを持つ場合のインデックス
/// @param bFromSweep 移動による衝突かどうか
/// @param SweepResult 衝突の詳細情報
void AGimmick_JumpPad::OnPadOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult)
{
	ASotugyouSeisakuCharacter* Character = Cast<ASotugyouSeisakuCharacter>(OtherActor);
	if (Character && Character->GetCharacterMovement())
	{
		//上方向に強い速度を加える
		FVector LaunchVelocity = FVector(0, 0, JumpPower);
		Character->LaunchCharacter(LaunchVelocity, true, true);
	}
}
