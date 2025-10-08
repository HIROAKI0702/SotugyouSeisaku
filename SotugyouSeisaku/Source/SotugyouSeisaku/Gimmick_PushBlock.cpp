// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick_PushBlock.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "SotugyouSeisakuCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
AGimmick_PushBlock::AGimmick_PushBlock()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	mMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("mMesh"));
	RootComponent = mMesh;

	mMesh->SetSimulatePhysics(false);//手動で動かす

	//コリジョン設定
	mMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	mMesh->SetCollisionObjectType(ECC_WorldDynamic);
	mMesh->SetCollisionResponseToAllChannels(ECR_Block);

	mMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);

	//プレイヤー（Pawn）とは重なるように設定
	mMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	mPushPower = 100.f;

	//デフォルトは全面(x軸方向)からのみ押せる
	mPushDir = FVector(1.0f, 0.0f, 0.0f);
	mPushAngle = 45.0f;
}

// Called when the game starts or when spawned
void AGimmick_PushBlock::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGimmick_PushBlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/// @brief プレイヤーに押された時に呼ばれる関数
/// @param PushingPlayer 自分を押しているプレイヤーのポインタ
void AGimmick_PushBlock::StartPushing(ASotugyouSeisakuCharacter* PushingPlayer)
{
	//ブロックが押されている状態
	bIsBeginePushed = true;
	mPushingPlayer = PushingPlayer;
}

/// @brief プレイヤーが自分を押すのをやめたときに呼ばれる関数
void AGimmick_PushBlock::StopPushing()
{
	bIsBeginePushed = false;
	mPushingPlayer = nullptr;
}

/// @brief プレイヤーが移動した分だけ自分も同じ方向・量で動かす
/// @param DeltaMove プレイヤーが１フレームで移動した量
void AGimmick_PushBlock::MoveWithPlayer(const FVector& DeltaMove)
{
	//アクターをワールド座標で移動
	FHitResult Hit;
	AddActorWorldOffset(DeltaMove, false, &Hit);

	//if (Hit.bBlockingHit)
	//{
	//	//衝突するまでの距離だけ移動（スライディング移動）
	//	FVector SafeMove = DeltaMove * Hit.Time;
	//	SetActorLocation(GetActorLocation() + SafeMove);
	//}
}

bool AGimmick_PushBlock::CanBePushedByPlayer(const FVector& PlayerLocation)const
{
	//ブロックからプレイヤーへの方向ベクトル
	FVector ToPlayer = PlayerLocation - GetActorLocation();
	ToPlayer.Z = 0.0f;

	if (ToPlayer.IsNearlyZero())
	{
		return false;
	}

	ToPlayer.Normalize();

	//ブロックの「押せる面」の法線方向をワールド座標に変換
	FVector WorldPushDir = GetActorRotation().RotateVector(mPushDir);
	WorldPushDir.Z = 0.0f;
	WorldPushDir.Normalize();

	//プレイヤーが押せる面の「反対側」にいる必要がある
	float DotProduct = FVector::DotProduct(ToPlayer, WorldPushDir);

	//DotProductが負 = プレイヤーは押せる面の反対側にいる
	//角度を計算
	float AngleRadians = FMath::Acos(FMath::Clamp(-DotProduct, -1.0f, 1.0f));
	float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

	bool bCanPush = AngleDegrees <= mPushAngle;

	UE_LOG(LogTemp, Log, TEXT("Push Check: Angle=%.1f°, Tolerance=%.1f°, CanPush=%s"),
		AngleDegrees, mPushAngle, bCanPush ? TEXT("YES") : TEXT("NO"));

	return bCanPush;
}

