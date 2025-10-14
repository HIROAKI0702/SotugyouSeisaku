// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmick_LaserEmitter.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class AMirrorActor;
class ALaserGimmickTargetActor;

UCLASS()
class SOTUGYOUSEISAKU_API AGimmick_LaserEmitter : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGimmick_LaserEmitter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//このNiagaraシステムアセットをBPで設定できるようにする
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Laser")
	class UNiagaraSystem* mNiagaraSystemAsset;

	//光線の始点
	UPROPERTY(EditAnywhere, Category = "Laser Setting")
	TObjectPtr<USceneComponent> mStartPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Laser Setting")
	TObjectPtr<UNiagaraComponent> mLaserBeam;

	//最大射程距離
	UPROPERTY(EditAnywhere, Category = "Laser Setting")
	float mMaxDistance = 1000.0f;

	//反射の最大回数
	UPROPERTY(EditAnywhere, Category = "Laser Setting")
	int32 mMaxBounces = 3;

	//現在のレーザー経路を保持
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FVector> mBeamPoints;

	//Tickごとに生成されるNiagaraコンポーネントを管理するための配列
	TArray<class UNiagaraComponent*> mActiveBeams;

	//光線を追跡する関数
	void TraceLaser(const FVector& Start, const FVector& Direction, int32 BounceCount = 0);
};
