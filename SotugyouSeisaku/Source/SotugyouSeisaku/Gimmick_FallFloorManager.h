// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmick_FallFloorManager.generated.h"

class UBoxComponent;

//落ちる床を管理するデータ構造
USTRUCT(BlueprintType)
struct FFallingFloorData
{
    GENERATED_BODY()

    //床の初期位置
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    //落下までの遅延時間
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DeleteDelay = 2.0f;

    //再生成までの遅延時間
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RespawnDelay = 5.0f;

    //揺れの強さ
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ShakeAmplitude = 5.0f;

    //揺れの速さ
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ShakeFrequency = 20.0f;

    //内部用：現在の揺れ状態
    float ShakeTimer = 0.0f;
    bool bIsShaking = false;

    //床コンポーネント参照
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> Mesh = nullptr;
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UBoxComponent> TriggerBox = nullptr;

    FTimerHandle DeleteTimerHandle;
    FTimerHandle RespawnTimerHandle;
};

UCLASS()
class SOTUGYOUSEISAKU_API AGimmick_FallFloorManager : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AGimmick_FallFloorManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    //オーバーラップ時の処理
    UFUNCTION()
    void OnFloorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


    UPROPERTY(EditAnywhere)
    TObjectPtr<UStaticMesh> mFloorMesh;

    //床のサイズ
    UPROPERTY(EditAnywhere)
    FVector mFloorScale = FVector(1.0f, 1.0f, 0.2f);

    //全床データ
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FFallingFloorData> mFloors;

    //床の落下
    void DeleteFloor(int32 Index);

    //床の再生成
    void RespawnFloor(int32 Index);
};
