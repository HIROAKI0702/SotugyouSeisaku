// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LaserGimmickTargetActor.generated.h"

UCLASS()
class SOTUGYOUSEISAKU_API ALaserGimmickTargetActor : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> mRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> mMesh;

public:	
	// Sets default values for this actor's properties
	ALaserGimmickTargetActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//制御するドア
	UPROPERTY(EditAnywhere, Category = "Balance Settings")
	AActor* mTargetDoor;

	//ドアの移動方向
	UPROPERTY(EditAnywhere, Category = "Balance Settings")
	FVector mDoorMoveOffset = FVector(400.0f, 0.0f, 0.0f);

	//ドアの移動速度
	UPROPERTY(EditAnywhere, Category = "Balance Settings")
	float mDoorMoveSpeed = 200.0f;

	//ドアが開いているか
	bool bDoorOpen = false;

	//光が当たった時に呼ばれる
	void OnLightHit();

	//光が外れた時に呼ばれる（必要なら）
	void OnLightLost();

	//ドアを動かす
	void MoveDoor(float DeltaTime);

	//ドアの初期位置
	FVector mDoorOriginalPosition;

	//ドアの目標位置
	FVector mDoorTargetPosition;

	// 光を受けてから一定時間後にリセットするためのタイマー
	FTimerHandle LightResetTimer;

	// 光が当たったフラグ
	bool bIsLit = false;
};
