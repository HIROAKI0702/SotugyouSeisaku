// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "TimerManager.h"
#include "Gimmick_FallFloor.generated.h"

class AGimmick_FallFloorManager;

UCLASS()
class SOTUGYOUSEISAKU_API AGimmick_FallFloor : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> mRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> mMesh;

	//ボタンの判定エリア
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> mTriggerBox;
	
public:	
	// Sets default values for this actor's properties
	AGimmick_FallFloor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//元に戻るまでの時間
	UPROPERTY(EditAnywhere, Category = "Falling Floor")
	float mRespawnDelay = 0.0f;

	//床削除までの時間
	UPROPERTY(EditAnywhere, Category = "Falling Floor")
	float mDeleteDelay = 2.0f;

	//揺れの強さ
	UPROPERTY(EditAnywhere, Category = "Falling Floor")
	float mShakeAmplitude = 0.0f;

	//揺れの速さ
	UPROPERTY(EditAnywhere, Category = "Falling Floor")
	float mShakeFrequency = 0.0f;

	//マネージャーへの参照
	UPROPERTY()
	TObjectPtr<AGimmick_FallFloorManager> mFloorManager;

	//床の元の位置
	FVector mOriginalMeshLocation;

	//揺れているかチェックするフラグ
	bool bIsShaking = false;

	//揺れる時間
	float mShakeTimer = 0.0f;

	//落下を遅延実行するためのタイマー
	FTimerHandle DeleteTimerHandle;
	//床の再生成タイマー
	FTimerHandle RespawnTimerHandle;

	//オーバーラップイベント
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//一定時間経過後に呼ばれ、床を削除する関数
	void DeleteFloor();

	//マネージャーを設定
	void SetManager(class AGimmick_FallFloorManager* Manager) { mFloorManager = Manager; }

};
