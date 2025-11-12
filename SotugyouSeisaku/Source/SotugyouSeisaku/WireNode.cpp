// Fill out your copyright notice in the Description page of Project Settings.

#include "WireNode.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "WirePuzzleManager.h"
#include "WireConnection.h"
#include "SotugyouSeisakuCharacter.h"

// Sets default values

/// @brief コンストラクタ　各種設定
AWireNode::AWireNode()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    //ルートコンポーネント作成
    mRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = mRoot;

    //メッシュコンポーネント作成
    mMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    mMesh->SetupAttachment(RootComponent);

    //インタラクトトリガー作成
    mInteractTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractTrigger"));
    mInteractTrigger->SetupAttachment(RootComponent);
    mInteractTrigger->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
    mInteractTrigger->SetGenerateOverlapEvents(true);

    //デフォルト値を設定
    mWireColor = EWireColor::Red;
    mNodeType = EWireNodeType::Start;
    mPairID = 0;
    bIsConnected = false;
    mConnectedNode = nullptr;
    bPlayerInRange = false;
    mCurrentPlayer = nullptr;
    bHasWire = true;//スタートノードは最初からワイヤーを持っている
    mInteractDistance = 200.0f;
}

// Called when the game starts or when spawned
void AWireNode::BeginPlay()
{
    Super::BeginPlay();

    //オーバーラップイベントをバインド
    mInteractTrigger->OnComponentBeginOverlap.AddDynamic(this, &AWireNode::OnTriggerBeginOverlap);
    mInteractTrigger->OnComponentEndOverlap.AddDynamic(this, &AWireNode::OnTriggerEndOverlap);

    //エンドノードは最初ワイヤーを持っていない
    if (mNodeType == EWireNodeType::End)
    {
        bHasWire = false;
    }

    //色に応じてマテリアルを変更
    if (mMesh && mMesh->GetMaterial(0))
    {
        UMaterialInstanceDynamic* DynMat = mMesh->CreateDynamicMaterialInstance(0);
        if (DynMat)
        {
            //マテリアルの "Color" パラメータに色を設定
            DynMat->SetVectorParameterValue(FName("Color"), GetWireColorValue());
        }
    }
}

// Called every frame
void AWireNode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

/// @brief インターフェース実装 - プレイヤーがFキーを押したときに呼ばれる
/// @param PlayerCharacter インタラクトしたプレイヤー
void AWireNode::Interact_Implementation(ASotugyouSeisakuCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || !bPlayerInRange)
    {
        return;
    }

    //プレイヤーがワイヤーを持っていない場合
    if (!PlayerCharacter->IsCarryingWire())
    {
        //このノードがワイヤーを持っていて、スタートノードなら拾える
        if (bHasWire && mNodeType == EWireNodeType::Start && !bIsConnected)
        {
            PickupWire(PlayerCharacter);
        }
    }
    else
    {
        //プレイヤーがワイヤーを持っている場合、エンドノードに接続できる
        if (mNodeType == EWireNodeType::End && !bIsConnected)
        {
            ConnectWire(PlayerCharacter);
        }
    }
}

/// @brief インタラクト可能かチェック
/// @param PlayerCharacter チェックするプレイヤー
/// @return インタラクト可能ならtrue
bool AWireNode::CanInteract_Implementation(ASotugyouSeisakuCharacter* PlayerCharacter) const
{
    if (!PlayerCharacter || !bPlayerInRange)
    {
        return false;
    }

    //距離チェック
    float Distance = FVector::Dist(GetActorLocation(), PlayerCharacter->GetActorLocation());
    if (Distance > mInteractDistance)
    {
        return false;
    }

    //すでに接続されている場合はインタラクト不可
    if (bIsConnected)
    {
        return false;
    }

    //プレイヤーがワイヤーを持っていない場合
    if (!PlayerCharacter->IsCarryingWire())
    {
        return (mNodeType == EWireNodeType::Start && bHasWire);
    }
    else
    {
        //プレイヤーがワイヤーを持っている場合
        AWireNode* StartNode = PlayerCharacter->mCarryingWireStartNode;

        if (mNodeType == EWireNodeType::End && StartNode)
        {
            return (mWireColor == StartNode->mWireColor);
        }
    }

    return false;
}

/// @brief インタラクト時に表示するテキストを取得
/// @return 表示するテキスト
FText AWireNode::GetInteractText_Implementation() const
{
    //すでに接続されている場合
    if (bIsConnected)
    {
        return FText::FromString(TEXT("Connected"));
    }

    //プレイヤーがワイヤーを持っている場合
    if (mCurrentPlayer && mCurrentPlayer->IsCarryingWire())
    {
        if (mNodeType == EWireNodeType::End)
        {
            AWireNode* StartNode = mCurrentPlayer->mCarryingWireStartNode;
            if (StartNode && mWireColor == StartNode->mWireColor)
            {
                //色が一致している
                return FText::FromString(TEXT("Connect Wire"));
            }
            else
            {
                //色が一致していない
                return FText::FromString(TEXT("Wrong Color"));
            }
        }
    }
    else
    {
        //プレイヤーがワイヤーを持っていない場合
        if (mNodeType == EWireNodeType::Start && bHasWire)
        {
            return FText::FromString(TEXT("Pick Up Wire"));
        }
    }

    return FText::FromString(TEXT(""));
}

/// @brief プレイヤーがトリガー範囲に入った時に呼ばれる
/// @param OverlappedComponent 重なったコンポーネント
/// @param OtherActor 重なった相手のアクタ
/// @param OtherComp 相手のコンポーネント
/// @param OtherBodyIndex ボディのインデックス
/// @param bFromSweep スイープによる衝突か
/// @param SweepResult 衝突の詳細情報
void AWireNode::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ASotugyouSeisakuCharacter* Player = Cast<ASotugyouSeisakuCharacter>(OtherActor))
    {
        //プレイヤーが範囲内に入った
        bPlayerInRange = true;
        mCurrentPlayer = Player;
    }
}

/// @brief プレイヤーがトリガー範囲から出た時に呼ばれる
/// @param OverlappedComponent 重なったコンポーネント
/// @param OtherActor 重なった相手のアクタ
/// @param OtherComp 相手のコンポーネント
/// @param OtherBodyIndex ボディのインデックス
void AWireNode::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (ASotugyouSeisakuCharacter* Player = Cast<ASotugyouSeisakuCharacter>(OtherActor))
    {
        //プレイヤーが範囲外に出た
        bPlayerInRange = false;
        mCurrentPlayer = nullptr;
    }
}

/// @brief ワイヤーを拾う（スタートノード用）
/// @param Player ワイヤーを拾うプレイヤー
void AWireNode::PickupWire(ASotugyouSeisakuCharacter* Player)
{
    if (!Player || !bHasWire || mNodeType != EWireNodeType::Start)
    {
        return;
    }

    //パズルマネージャーを取得
    AWirePuzzleManager* Manager = AWirePuzzleManager::Get(GetWorld());
    if (!Manager || !Manager->mWireConnectionClass)
    {
        return;
    }

    //ワイヤーの視覚表現を生成
    AWireConnection* Connection = GetWorld()->SpawnActor<AWireConnection>(Manager->mWireConnectionClass);
    if (Connection)
    {
        //ワイヤーを設定（スタートノードからプレイヤーまで）
        Connection->SetupConnection(this, nullptr, Player);
        Connection->AttachToPlayer(Player);

        //プレイヤーにワイヤーを持たせる
        Player->SetCarryingWire(this, Connection);

        //このノードはもうワイヤーを持っていない
        bHasWire = false;
    }
}

/// @brief ワイヤーを接続する（エンドノード用）
/// @param Player ワイヤーを接続するプレイヤー
void AWireNode::ConnectWire(ASotugyouSeisakuCharacter* Player)
{
    if (!Player || mNodeType != EWireNodeType::End || bIsConnected)
    {
        return;
    }

    //プレイヤーが持っているワイヤー情報を取得
    AWireNode* StartNode = Player->mCarryingWireStartNode;
    AWireConnection* Connection = Player->mCarryingWireConnection;

    if (!StartNode || !Connection)
    {
        return;
    }

    //色が一致しているかチェック
    if (mWireColor != StartNode->mWireColor)
    {
        return;
    }

    //接続を確立
    bIsConnected = true;
    mConnectedNode = StartNode;
    StartNode->bIsConnected = true;
    StartNode->mConnectedNode = this;

    //ワイヤーの終点を設定（プレイヤーからこのノードに固定）
    Connection->SetupConnection(StartNode, this, nullptr);

    //プレイヤーからワイヤーをクリア
    Player->ClearCarryingWire();

    //パズルマネージャーに接続を通知
    AWirePuzzleManager* Manager = AWirePuzzleManager::Get(GetWorld());
    if (Manager)
    {
        Manager->RegisterConnection(StartNode, this, Player);
    }
}

/// @brief ワイヤーの接続を解除する
void AWireNode::Disconnect()
{
    //接続先のノードの接続も解除
    if (mConnectedNode)
    {
        mConnectedNode->bIsConnected = false;
        mConnectedNode->mConnectedNode = nullptr;
    }

    //このノードの接続を解除
    bIsConnected = false;
    mConnectedNode = nullptr;

    //スタートノードはワイヤーを再度持つ
    if (mNodeType == EWireNodeType::Start)
    {
        bHasWire = true;
    }
}

/// @brief 指定したノードと接続可能かチェック
/// @param TargetNode 接続先のノード
/// @return 接続可能ならtrue
bool AWireNode::CanConnectTo(AWireNode* TargetNode) const
{
    if (!TargetNode)
    {
        return false;
    }

    //自分自身には接続できない
    if (TargetNode == this)
    {
        return false;
    }

    //すでに接続されている
    if (bIsConnected || TargetNode->bIsConnected)
    {
        return false;
    }

    //スタート同士、エンド同士は接続できない
    if (mNodeType == TargetNode->mNodeType)
    {
        return false;
    }

    //色が違う
    if (mWireColor != TargetNode->mWireColor)
    {
        return false;
    }

    return true;
}

/// @brief ワイヤーの色を取得（FLinearColor形式）
/// @return ワイヤーの色
FLinearColor AWireNode::GetWireColorValue() const
{
    switch (mWireColor)
    {
    case EWireColor::Red:    return FLinearColor::Red;
    case EWireColor::Blue:   return FLinearColor::Blue;
    case EWireColor::Green:  return FLinearColor::Green;
    case EWireColor::Yellow: return FLinearColor::Yellow;
    case EWireColor::Purple: return FLinearColor(0.5f, 0.0f, 0.5f);
    default: return FLinearColor::White;
    }
}
