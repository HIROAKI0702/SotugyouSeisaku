// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmick_Balance.generated.h"

class UBoxComponent;
class AGimmick_PushBlock;
class ASotugyouSeisakuCharacter;

UCLASS()
class SOTUGYOUSEISAKU_API AGimmick_Balance : public AActor
{
	GENERATED_BODY()

	//ルートコンポーネント
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> mRoot;

	//左の量り
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> mLeftPlateMesh;

	//右の量り
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> mRightPlateMesh;

	//左の量りの判定エリア
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> mLeftTrigger;

	//右の量りの判定エリア
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> mRightTrigger;

public:
	// Sets default values for this actor's properties
	AGimmick_Balance();

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
	FVector mDoorMoveOffset = FVector(0.0f, 0.0f, 300.0f);

	//ドアの移動速度
	UPROPERTY(EditAnywhere, Category = "Balance Settings")
	float mDoorMoveSpeed = 200.0f;

	//釣り合いの許容誤差（この範囲内なら釣り合っているとみなす）
	UPROPERTY(EditAnywhere, Category = "Balance Settings", meta = (ClampMin = "0.0"))
	float mBalanceTolerance = 50.0f;

	//左の皿に乗っているブロック
	UPROPERTY()
	TArray<AGimmick_PushBlock*> mLeftBlocks;

	//右の皿に乗っているブロック
	UPROPERTY()
	TArray<AGimmick_PushBlock*> mRightBlocks;

	//現在の左の皿の重さ
	float mLeftWeight = 0.0f;

	//現在の右の皿の重さ
	float mRightWeight = 0.0f;

	//釣り合っているか
	bool bIsBalanced = false;

	//ドアが開いているか
	bool bDoorOpen = false;

	//ドアの初期位置
	FVector mDoorOriginalPosition;

	//ドアの目標位置
	FVector mDoorTargetPosition;

	//現在の梁の傾き角度
	float mCurrentTiltAngle = 0.0f;

	//オーバーラップイベント
	UFUNCTION()
	void OnLeftPlateBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnLeftPlateEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnRightPlateBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRightPlateEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//重さを計算
	void UpdateWeights();

	//釣り合いをチェック
	void CheckBalance();

	//ドアを動かす
	void MoveDoor(float DeltaTime);
};
