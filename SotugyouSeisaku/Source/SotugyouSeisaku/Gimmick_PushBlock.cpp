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

	mPushPower = 100.f;
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

void AGimmick_PushBlock::StartPushing(ASotugyouSeisakuCharacter* PushingPlayer)
{
	bIsBeginePushed = true;
	mPushingPlayer = PushingPlayer;
}

void AGimmick_PushBlock::StopPushing()
{
	bIsBeginePushed = false;
	mPushingPlayer = nullptr;
}

void AGimmick_PushBlock::MoveWithPlayer(const FVector& DeltaMove)
{
	UE_LOG(LogTemp, Warning, TEXT("Block Moving: %s"), *DeltaMove.ToString());

	FHitResult Hit;
	AddActorWorldOffset(DeltaMove, false, &Hit);

	if (Hit.bBlockingHit)
	{
		UE_LOG(LogTemp, Error, TEXT("Block hit something: %s"), *GetNameSafe(Hit.GetActor()));

		//衝突するまでの距離だけ移動（スライディング移動）
		FVector SafeMove = DeltaMove * Hit.Time;
		SetActorLocation(GetActorLocation() + SafeMove);
	}
}

