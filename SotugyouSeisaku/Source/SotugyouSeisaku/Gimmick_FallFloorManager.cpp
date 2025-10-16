// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick_FallFloorManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"

// Sets default values

/// @brief コンストラクタ　Managerの初期化
AGimmick_FallFloorManager::AGimmick_FallFloorManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGimmick_FallFloorManager::BeginPlay()
{
	Super::BeginPlay();
	
    for (int32 i = 0; i < mFloors.Num(); i++)
    {
        FFallingFloorData& Data = mFloors[i];

        //StaticMeshComponent 作成
        Data.Mesh = NewObject<UStaticMeshComponent>(this);
        Data.Mesh->RegisterComponent();
        Data.Mesh->SetStaticMesh(mFloorMesh);
        Data.Mesh->SetWorldScale3D(mFloorScale);
        Data.Mesh->SetWorldLocation(Data.Location);
        Data.Mesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

        //BoxComponent 作成
        Data.TriggerBox = NewObject<UBoxComponent>(this);
        Data.TriggerBox->RegisterComponent();
        Data.TriggerBox->SetBoxExtent(FVector(50, 50, 20));
        Data.TriggerBox->SetCollisionResponseToAllChannels(ECR_Overlap);
        Data.TriggerBox->AttachToComponent(Data.Mesh, FAttachmentTransformRules::KeepRelativeTransform);
        Data.TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AGimmick_FallFloorManager::OnFloorBeginOverlap);
        //Index をタグとして格納
        Data.TriggerBox->ComponentTags.Add(FName(*FString::FromInt(i)));
    }
}

// Called every frame
void AGimmick_FallFloorManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    //床データをループ
    for (int32 i = 0; i < mFloors.Num(); i++)
    {
        //揺れ中かつメッシュコンポーネントが有効な床のみ処理
        FFallingFloorData& Data = mFloors[i];
        if (Data.bIsShaking && Data.Mesh)
        {
            //揺れのオフセットを計算
            Data.ShakeTimer += DeltaTime;
            FVector Offset;
            Offset.X = FMath::Sin(Data.ShakeTimer * Data.ShakeFrequency) * Data.ShakeAmplitude;
            Offset.Y = FMath::Cos(Data.ShakeTimer * Data.ShakeFrequency) * Data.ShakeAmplitude;
            Data.Mesh->SetWorldLocation(Data.Location + Offset);
        }
    }
}

/// @brief プレイヤーなどが床に乗った瞬間に呼ばれるイベント。
///        一定時間後に床を削除する。
/// @param OverlappedComponent イベントを発生させた自身のコリジョン
/// @param OtherActor トリガー範囲に入ったアクタ
/// @param OtherComp 相手アクタのどのコンポーネントに当たったか
/// @param OtherBodyIndex 複数ボディを持つ場合のインデックス
/// @param bFromSweep 移動による衝突かどうか
/// @param SweepResult 衝突の詳細情報
void AGimmick_FallFloorManager::OnFloorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    //無効なアクタは無視
    if (!OtherActor || !OtherActor->IsA(ACharacter::StaticClass()))
        return;

    //タグから Index を取得
    if (OverlappedComponent->ComponentTags.Num() == 0) return;
    int32 Index = FCString::Atoi(*OverlappedComponent->ComponentTags[0].ToString());

    //どの床か特定
    if (!mFloors.IsValidIndex(Index)) return;
    FFallingFloorData& Data = mFloors[Index];

    if (Data.bIsShaking) return;
    Data.bIsShaking = true;
    Data.ShakeTimer = 0.0f;

    //一定時間後に DeleteFloor を呼ぶ
    GetWorldTimerManager().SetTimer(Data.DeleteTimerHandle, FTimerDelegate::CreateUObject(this, &AGimmick_FallFloorManager::DeleteFloor, Index), Data.DeleteDelay, false);
}

/// @brief 床を削除する関数
/// @param Index 床の数
void AGimmick_FallFloorManager::DeleteFloor(int32 Index)
{
    //指定されたインデックスが有効化を確認
    if (!mFloors.IsValidIndex(Index)) return;
    FFallingFloorData& Data = mFloors[Index];

    if (Data.Mesh)
        Data.Mesh->SetVisibility(false, true);
    if (Data.TriggerBox)
        Data.TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    //再生成タイマー
    GetWorldTimerManager().SetTimer(Data.RespawnTimerHandle, FTimerDelegate::CreateUObject(this, &AGimmick_FallFloorManager::RespawnFloor, Index), Data.RespawnDelay, false);

    Destroy();//床を削除 → プレイヤーは落下
}

/// @brief 床を再生成する関数
/// @param Index 床の数
void AGimmick_FallFloorManager::RespawnFloor(int32 Index)
{
    //指定されたインデックスが有効化を確認
    if (!mFloors.IsValidIndex(Index)) return;
    FFallingFloorData& Data = mFloors[Index];

    if (Data.Mesh)
    {
        Data.Mesh->SetVisibility(true, true);
        Data.Mesh->SetWorldLocation(Data.Location);
    }
    if (Data.TriggerBox)
        Data.TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    Data.bIsShaking = false;
    Data.ShakeTimer = 0.0f;
}

