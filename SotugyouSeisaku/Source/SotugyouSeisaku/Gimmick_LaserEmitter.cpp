// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick_LaserEmitter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraComponent.h"
#include "DrawDebugHelpers.h"
#include "MirrorActor.h"
#include "LaserGimmickTargetActor.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values

/// @brief コンストラクタ　レーザーの各種設定
AGimmick_LaserEmitter::AGimmick_LaserEmitter()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//光の始点
	mStartPoint = CreateDefaultSubobject<USceneComponent>(TEXT("StartPoint"));
	RootComponent = mStartPoint;
}

// Called when the game starts or when spawned
void AGimmick_LaserEmitter::BeginPlay()
{
	Super::BeginPlay();

    //旧実装のビームコンポーネントを無効化
    if (mLaserBeam)
    {
        mLaserBeam->Deactivate();
    }

    if (mNiagaraSystemAsset)
    {
        mLaserBeam = UNiagaraFunctionLibrary::SpawnSystemAttached(
            mNiagaraSystemAsset,
            mStartPoint,
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            false
        );
    }
}

// Called every frame
void AGimmick_LaserEmitter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    //毎フレームレーザー発射（反射なしなのでBounceCount=0）
    FVector Start = mStartPoint->GetComponentLocation();
    FVector Dir = mStartPoint->GetForwardVector();
    TraceLaser(Start, Dir, 0);
}

/// @brief レーザー追跡処理関数
/// @param Start ビームの始点
/// @param Direction ビームの方向ベクトル
/// @param BounceCount ビームの終点
void AGimmick_LaserEmitter::TraceLaser(const FVector& Start, const FVector& Direction, int32 BounceCount)
{
    //経路初期化
    mBeamPoints.Empty();

    FVector End = Start + Direction * mMaxDistance;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

    mBeamPoints.Add(Start);

    if (bHit)
    {
        End = Hit.ImpactPoint;

        //ドア（ターゲット）に当たったら開く
        if (ALaserGimmickTargetActor* Target = Cast<ALaserGimmickTargetActor>(Hit.GetActor()))
        {
            Target->OnLightHit();
        }
    }

    mBeamPoints.Add(End);

    //Niagaraレーザーの始点・終点をパラメータで渡す
    if (mLaserBeam)
    {
        mLaserBeam->SetWorldLocation(Start);

        //Niagara SystemでUser Parameter("LaserEnd")を作成しておく
        mLaserBeam->SetVectorParameter(TEXT("LaserEnd"), End);
    }
}

