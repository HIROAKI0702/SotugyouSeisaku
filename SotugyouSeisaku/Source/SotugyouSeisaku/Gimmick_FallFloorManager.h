// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmick_FallFloorManager.generated.h"

class UBoxComponent;

//落ちる床を管理するデータ構造
USTRUCT(BlueprintType)
struct FFallFloorData
{
    GENERATED_BODY()

    UPROPERTY()
    TSubclassOf<class AGimmick_FallFloor> FloorClass;

    UPROPERTY()
    FVector OriginalLocation;

    UPROPERTY()
    FRotator OriginalRotation;

    UPROPERTY()
    bool bIsDestroyed;

    FFallFloorData()
        : FloorClass(nullptr)
        , OriginalLocation(FVector::ZeroVector)
        , OriginalRotation(FRotator::ZeroRotator)
        , bIsDestroyed(false)
    {}
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

    //床を登録
    UFUNCTION(BlueprintCallable, Category = "Floor Manager")
    void RegisterFloor(class AGimmick_FallFloor* Floor);

    //床が破壊されたことを記録
    UFUNCTION(BlueprintCallable, Category = "Floor Manager")
    void OnFloorDestroyed(class AGimmick_FallFloor* Floor);

    //すべての床を再生成
    UFUNCTION(BlueprintCallable, Category = "Floor Manager")
    void RespawnAllFloors();

    //特定の床を再生成
    UFUNCTION(BlueprintCallable, Category = "Floor Manager")
    void RespawnFloor(int32 Index);

protected:
    //登録されている床のデータ
    UPROPERTY()
    TArray<FFallFloorData> mFloorDataList;

    //現在存在している床の参照
    UPROPERTY()
    TArray<class AGimmick_FallFloor*> mActiveFloors;
};
