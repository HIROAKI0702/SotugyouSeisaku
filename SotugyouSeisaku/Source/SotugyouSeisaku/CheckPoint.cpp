// Fill out your copyright notice in the Description page of Project Settings.

#include "CheckPoint.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "MR_GameInstance.h"

ACheckPoint::ACheckPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	// ルートコンポーネントとしてボックスコンポーネントを設定
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;

	// ボックスのサイズを設定
	CollisionBox->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetGenerateOverlapEvents(true);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// メッシュコンポーネントを作成
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(CollisionBox);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// デフォルト値
	CheckPointIndex = 0;
	bIsActivated = false;
}

void ACheckPoint::BeginPlay()
{
	Super::BeginPlay();

	// オーバーラップイベントをバインド
	if (CollisionBox)
	{
		CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ACheckPoint::OnBoxBeginOverlap);
	}

	UE_LOG(LogTemp, Log, TEXT("CheckPoint %d created"), CheckPointIndex);
}

void ACheckPoint::ActivateCheckPoint()
{
	if (bIsActivated)
	{
		return; // 既に有効化されている場合は処理しない
	}

	bIsActivated = true;

	UE_LOG(LogTemp, Log, TEXT("CheckPoint %d activated!"), CheckPointIndex);

	// パーティクルエフェクトを再生
	if (ActivationParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ActivationParticle, GetActorLocation());
	}

	// SE を再生
	if (ActivationSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ActivationSound, GetActorLocation());
	}

	// GameInstance にチェックポイントを記録
	UMR_GameInstance* GameInstance = Cast<UMR_GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (GameInstance)
	{
		GameInstance->SetCurrentCheckPoint(CheckPointIndex);
		UE_LOG(LogTemp, Log, TEXT("GameInstance: CheckPoint %d saved"), CheckPointIndex);
	}
}

void ACheckPoint::OnBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	// プレイヤーキャラクターがオーバーラップしたか確認
	if (OtherActor && OtherActor->IsA<APawn>())
	{
		ActivateCheckPoint();
	}
}