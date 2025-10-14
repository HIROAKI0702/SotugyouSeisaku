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
}

// Called every frame
void AGimmick_LaserEmitter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    //前フレームで生成したビームを全て破棄する
    for (UNiagaraComponent* ActiveBeam : mActiveBeams)
    {
        if (ActiveBeam)
        {
            ActiveBeam->DestroyComponent();
        }
    }
    mActiveBeams.Empty();//配列をクリア

    //光源の開始点と方向を取得
    FVector Start = mStartPoint->GetComponentLocation();
    FVector Dir = mStartPoint->GetForwardVector();

    mBeamPoints.Empty();//前フレームの経路をクリア
    TraceLaser(Start, Dir, 0);//ビームを追跡

    //Niagaraシステムアセットが設定されているか確認
    if (mNiagaraSystemAsset && mBeamPoints.Num() >= 2)
    {
        //軌跡の各セグメントに対してNiagaraシステムをスポーン
        for (int32 i = 0; i < mBeamPoints.Num() - 1; i++)
        {
            // TransientなNiagaraシステムを生成する
            UNiagaraComponent* NewBeam = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                mNiagaraSystemAsset,
                mBeamPoints[i],//スポーン位置
                FRotator::ZeroRotator,
                FVector::OneVector,
                true,//AutoDestroy: ビームが消えたら自動でコンポーネントを破棄する
                true//AutoActivate: 即座に再生を開始する
            );

            if (NewBeam)
            {
                //生成したビームの開始点と終了点をユーザーパラメータに設定
                NewBeam->SetNiagaraVariableVec3(TEXT("Beam Start"), mBeamPoints[i]);
                NewBeam->SetNiagaraVariableVec3(TEXT("Beam End"), mBeamPoints[i + 1]);

                //参照を保持しておく
                mActiveBeams.Add(NewBeam);
            }
        }
    }
}

/// @brief レーザー追跡処理関数
/// @param Start ビームの始点
/// @param Direction ビームの方向ベクトル
/// @param BounceCount ビームの終点
void AGimmick_LaserEmitter::TraceLaser(const FVector& Start, const FVector& Direction, int32 BounceCount)
{
    //最大反射回数を超えると終了
    if (BounceCount > mMaxBounces) return;

    //終点計算
    FVector End = Start + Direction * mMaxDistance;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    //鏡に当たった際に、その鏡を無視しないようにする（反射元の鏡は無視する）
    Params.AddIgnoredActor(Hit.GetActor());

    //線分キャストで衝突判定
    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

    if (bHit)
    {
        End = Hit.ImpactPoint;//衝突位置を終点にする
    }

    //経路にこのビームの始点を追加
    mBeamPoints.Add(Start);

    if (bHit)
    {
        //鏡に当たった場合
        if (AMirrorActor* Mirror = Cast<AMirrorActor>(Hit.GetActor()))
        {
            FVector ReflectedDir = UKismetMathLibrary::GetReflectionVector(Direction, Hit.ImpactNormal);
            FVector NextStart = Hit.ImpactPoint + ReflectedDir * 5.f;//衝突点から少しずらして次回のトレース開始
            TraceLaser(NextStart, ReflectedDir, BounceCount + 1);
            return;//ここで処理を終了させる
        }
        //受光ターゲットに当たった場合
        else if (ALaserGimmickTargetActor* Target = Cast<ALaserGimmickTargetActor>(Hit.GetActor()))
        {
            Target->OnLightHit();
            mBeamPoints.Add(End);
            return;//ここで処理を終了させる
        }
    }

    //衝突しない場合、または上記以外のものに当たった場合は終点を追加
    mBeamPoints.Add(End);
}

