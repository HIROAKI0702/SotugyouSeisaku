// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick_FallFloorManager.h"
#include "Gimmick_FallFloor.h"
#include <Kismet/GameplayStatics.h>

// Sets default values

/// @brief コンストラクタ　Managerの初期化
AGimmick_FallFloorManager::AGimmick_FallFloorManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AGimmick_FallFloorManager::BeginPlay()
{
    Super::BeginPlay();

    //レベル内のすべてのAGimmick_FallFloor を検索して登録
    TArray<AActor*> FoundFloors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGimmick_FallFloor::StaticClass(), FoundFloors);

    //検査結果を登録
    for (AActor* Actor : FoundFloors)
    {
        if (AGimmick_FallFloor* Floor = Cast<AGimmick_FallFloor>(Actor))
        {
            RegisterFloor(Floor);
        }
    }
}

/// @brief FallFloorの情報をマネージャーに登録
/// @param Floor 登録するFallFloorアクタ
void AGimmick_FallFloorManager::RegisterFloor(AGimmick_FallFloor* Floor)
{
    if (!Floor)
    {
        return;
    }

    //床の初期情報を保存
    FFallFloorData NewData;
    NewData.FloorClass = Floor->GetClass();
    NewData.OriginalLocation = Floor->GetActorLocation();
    NewData.OriginalRotation = Floor->GetActorRotation();
    NewData.bIsDestroyed = false;

    mFloorDataList.Add(NewData);
    mActiveFloors.Add(Floor);

    //床にマネージャーを設定
    Floor->SetManager(this);
}

/// @brief 破壊されたFallFloorを特定
/// @param Floor FallFloorアクタ
void AGimmick_FallFloorManager::OnFloorDestroyed(AGimmick_FallFloor* Floor)
{
    if (!Floor)
    {
        return;
    }

    //どの床が破壊されたか特定
    for (int32 i = 0; i < mActiveFloors.Num(); i++)
    {
        if (mActiveFloors[i] == Floor)
        {
            if (mFloorDataList.IsValidIndex(i))
            {
                mFloorDataList[i].bIsDestroyed = true;
                mActiveFloors[i] = nullptr;
            }
            break;
        }
    }
}

/// @brief 全ての床を再生成する関数
void AGimmick_FallFloorManager::RespawnAllFloors()
{
    if (!GetWorld())
    {
        return;
    }

    //登録した床を調べる
    for (int32 i = 0; i < mFloorDataList.Num(); i++)
    {
        if (mFloorDataList[i].bIsDestroyed)
        {
            //床の再生成
            RespawnFloor(i);
        }
    }
}

/// @brief 指定インデックスの床を再生成する
/// @param Index 再生成対象の床データインデックス
void AGimmick_FallFloorManager::RespawnFloor(int32 Index)
{
    if (!GetWorld() || !mFloorDataList.IsValidIndex(Index))
    {
        return;
    }

    FFallFloorData& FloorData = mFloorDataList[Index];

    if (!FloorData.bIsDestroyed)
    {
        return; //まだ破壊されていない
    }

    //床を再生成
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AGimmick_FallFloor* NewFloor = GetWorld()->SpawnActor<AGimmick_FallFloor>(
        FloorData.FloorClass,
        FloorData.OriginalLocation,
        FloorData.OriginalRotation,
        SpawnParams
    );

    //再生成が成功したときにリストを更新
    if (NewFloor)
    {
        FloorData.bIsDestroyed = false;
        mActiveFloors[Index] = NewFloor;
        NewFloor->SetManager(this);
    }
}


