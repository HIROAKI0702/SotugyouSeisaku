// Fill out your copyright notice in the Description page of Project Settings.

#include "WireNode.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "WirePuzzleManager.h"
#include "WireConnection.h"
#include "SotugyouSeisakuCharacter.h"

// Sets default values

/// @brief コンストラクタ　ワイヤーノードの各種設定
AWireNode::AWireNode()
{
	//毎フレームTickを呼ぶ
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
	mNodeType = EWireNodeType::Start;
	mPairID = 0;
	bIsConnected = false;
	mConnectedNode = nullptr;
	bPlayerInRange = false;
	mCurrentPlayer = nullptr;
	bHasWire = true;//スタートノードは最初からワイヤーを持っている
	mInteractDistance = 200.0f;

	//拡張機能のデフォルト値
	mMaxInputs = 2;   //Mergeノードは2入力
	mMaxOutputs = 2;  //Splitノードは2出力
	mMergedColor = EWireColor::Red;
}

/// @brief ゲーム開始時に呼ばれる初期化処理
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
			//マテリアルの"Color"パラメータに色を設定
			DynMat->SetVectorParameterValue(FName("Color"), GetWireColorValue());
		}
	}
}

/// @brief 毎フレーム呼ばれる更新処理
/// @param DeltaTime 前フレームからの経過時間
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

	//すでに接続されている場合
	if (bIsConnected)
	{
		return true;
	}

	//プレイヤーがワイヤーを持っていない場合
	if (!PlayerCharacter->IsCarryingWire())
	{
		//スタートノードで、ワイヤーがあるならインタラクト可能
		bool CanPickup = (mNodeType == EWireNodeType::Start && bHasWire);
		return CanPickup;
	}
	else
	{
		//プレイヤーがワイヤーを持っている場合
		//エンドノードで未接続ならどの色でも接続可能
		bool CanConnect = (mNodeType == EWireNodeType::End && !bIsConnected);
		return CanConnect;
	}
}

/// @brief インタラクト時に表示するテキストを取得
/// @return 表示するテキスト
FText AWireNode::GetInteractText_Implementation() const
{
	if (bIsConnected && mNodeType != EWireNodeType::Merge && mNodeType != EWireNodeType::Split)
	{
		return FText::FromString(TEXT("Disconnect Wire"));
	}

	if (mCurrentPlayer && mCurrentPlayer->IsCarryingWire())
	{
		//プレイヤーがワイヤーを持っている場合
		switch (mNodeType)
		{
		case EWireNodeType::End:
		case EWireNodeType::Relay:
			if (!bIsConnected) 
			{
				return FText::FromString(TEXT("Connect Wire"));
			}
			else if (CanProvideOutput()) 
			{
				return FText::FromString(TEXT("Relay Wire"));
			}
			break;

		case EWireNodeType::Merge:
			if (CanAcceptInput())
			{
				return FText::FromString(FString::Printf(TEXT("Connect Wire (%d/%d)"),
					mInputNodes.Num() + 1, mMaxInputs));
			}
			else
			{
				return FText::FromString(TEXT("Merge Full"));
			}
			break;

		case EWireNodeType::Split:
			if (CanAcceptInput()) 
			{
				return FText::FromString(TEXT("Connect Input Wire"));
			}
			else if (CanProvideOutput()) 
			{
				return FText::FromString(FString::Printf(TEXT("Split Wire (%d/%d)"), mOutputNodes.Num(), mMaxOutputs));
			}
			break;
		}
	}
	else
	{
		//プレイヤーがワイヤーを持っていない場合
		if (CanProvideOutput())
		{
			switch (mNodeType)
			{
			case EWireNodeType::Start:
				return FText::FromString(TEXT("Pick Up Wire"));

			case EWireNodeType::Relay:
				return FText::FromString(TEXT("Relay Wire"));

			case EWireNodeType::Merge:
				return FText::FromString(TEXT("Pick Up Merged Wire"));

			case EWireNodeType::Split:
				return FText::FromString(FString::Printf(TEXT("Split Wire (%d/%d)"),
					mOutputNodes.Num(), mMaxOutputs));
			}
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
	if (!Player)
	{
		return;
	}

	//出力可能かチェック
	if (!CanProvideOutput())
	{
		return;
	}

	AWirePuzzleManager* Manager = AWirePuzzleManager::Get(GetWorld());
	if (!Manager || !Manager->mWireConnectionClass)
	{
		return;
	}

	//ワイヤーの視覚表現を生成
	AWireConnection* Connection = GetWorld()->SpawnActor<AWireConnection>(Manager->mWireConnectionClass);
	if (Connection)
	{
		Connection->SetupConnection(this, nullptr, Player);
		Connection->AttachToPlayer(Player);

		Player->SetCarryingWire(this, Connection);

		//ノードタイプに応じた処理
		switch (mNodeType)
		{
		case EWireNodeType::Start:
			bHasWire = false;
			break;

		case EWireNodeType::Relay:
		case EWireNodeType::Merge:
			//中継・合成ノードは入力があれば出力可能
			break;

		case EWireNodeType::Split:
			//分岐ノードは複数出力可能
			break;
		}
	}
}

/// @brief ワイヤーを接続する（エンドノード用）
/// @param Player ワイヤーを接続するプレイヤー
void AWireNode::ConnectWire(ASotugyouSeisakuCharacter* Player)
{
	if (!Player)
	{
		return;
	}

	AWireNode* StartNode = Player->mCarryingWireStartNode;
	AWireConnection* Connection = Player->mCarryingWireConnection;

	if (!StartNode || !Connection)
	{
		return;
	}

	//ノードタイプに応じた接続処理
	switch (mNodeType)
	{
	case EWireNodeType::End:
	case EWireNodeType::Relay:
		//通常の接続
		if (!bIsConnected)
		{
			bIsConnected = true;
			mConnectedNode = StartNode;
			mConnectedWire = Connection;

			StartNode->bIsConnected = true;
			StartNode->mConnectedNode = this;
			StartNode->mConnectedWire = Connection;
			StartNode->AddOutputNode(this);

			AddInputNode(StartNode);

			Connection->SetupConnection(StartNode, this, nullptr);
			Player->ClearCarryingWire();
		}
		break;

	case EWireNodeType::Merge:
		//複数入力を受け付ける
		if (CanAcceptInput())
		{
			AddInputNode(StartNode);
			StartNode->AddOutputNode(this);

			Connection->SetupConnection(StartNode, this, nullptr);
			Player->ClearCarryingWire();

			//全入力が揃ったら合成色を計算
			if (mInputNodes.Num() >= mMaxInputs)
			{
				UpdateMergedColor();
			}
		}
		break;

	case EWireNodeType::Split:
		//入力は1つだけ
		if (mInputNodes.Num() == 0)
		{
			AddInputNode(StartNode);
			StartNode->AddOutputNode(this);

			Connection->SetupConnection(StartNode, this, nullptr);
			Player->ClearCarryingWire();

			//入力が来たら全出力にワイヤーをコピー
			for (AWireNode* OutNode : mOutputNodes)
			{
				if (!OutNode->bIsConnected)
				{
					AWirePuzzleManager* Manager = AWirePuzzleManager::Get(GetWorld());
					if (Manager && Manager->mWireConnectionClass)
					{
						AWireConnection* NewConn = GetWorld()->SpawnActor<AWireConnection>(Manager->mWireConnectionClass);
						if (NewConn)
						{
							NewConn->SetupConnection(this, OutNode, nullptr);
							mConnectedWire = NewConn;
							OutNode->AddInputNode(this);
							OutNode->bIsConnected = true;
						}
					}
				}
			}
			bIsConnected = true;
		}
		break;
	}

	// パズルマネージャーに通知
	AWirePuzzleManager* Manager = AWirePuzzleManager::Get(GetWorld());
	if (Manager)
	{
		Manager->RegisterConnection(StartNode, this, Player);
	}
}

/// @brief ワイヤーの接続を解除する
void AWireNode::Disconnect()
{

	if (!bIsConnected || !mConnectedNode)
	{
		return;
	}

	//接続されているワイヤーのビジュアルを削除
	if (mConnectedWire)
	{
		mConnectedWire->Destroy();
		mConnectedWire = nullptr;
	}

	//接続先ノードの参照を取得
	AWireNode* OtherNode = mConnectedNode;

	//両方のノードの接続状態をクリア
	this->bIsConnected = false;
	this->mConnectedNode = nullptr;

	if (OtherNode)
	{
		OtherNode->bIsConnected = false;
		OtherNode->mConnectedNode = nullptr;
		OtherNode->mConnectedWire = nullptr;

		//スタートノードならワイヤーを再度持つ
		if (OtherNode->mNodeType == EWireNodeType::Start)
		{
			OtherNode->bHasWire = true;
		}
	}

	//このノードがスタートノードならワイヤーを再度持つ
	if (mNodeType == EWireNodeType::Start)
	{
		bHasWire = true;
	}

	//パズルマネージャーに通知（再チェック用）
	AWirePuzzleManager* Manager = AWirePuzzleManager::Get(GetWorld());
	if (Manager)
	{
		Manager->CheckPuzzleCompletion();
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

/// @brief 2つの色を合成して新しい色を計算
/// @param Color1 色1
/// @param Color2 色2
/// @return 合成された色
EWireColor AWireNode::MergeColors(EWireColor Color1, EWireColor Color2)
{
	//赤 + 青 = 紫
	if ((Color1 == EWireColor::Red && Color2 == EWireColor::Blue) ||
		(Color1 == EWireColor::Blue && Color2 == EWireColor::Red))
	{
		return EWireColor::Purple;
	}

	//赤 + 緑 = 黄色
	if ((Color1 == EWireColor::Red && Color2 == EWireColor::Green) ||
		(Color1 == EWireColor::Green && Color2 == EWireColor::Red))
	{
		return EWireColor::Yellow;
	}

	//同じ色同士ならそのまま
	if (Color1 == Color2)
	{
		return Color1;
	}

	//デフォルトは最初の色
	return Color1;
}

/// @brief このノードが入力を受け付けられるか
/// @return 入力可能ならtrue
bool AWireNode::CanAcceptInput() const
{
	switch (mNodeType) 
	{
	case EWireNodeType::End:
		return !bIsConnected;
	case EWireNodeType::Relay:
		return mInputNodes.Num() == 0;//Relayは1入力のみ許可
	case EWireNodeType::Merge:
		return mInputNodes.Num() < mMaxInputs;//最大入力数まで受付
	case EWireNodeType::Split:
		return mInputNodes.Num() == 0;//Splitは入力1本のみ
	default:
		return false;
	}
}

/// @brief このノードが出力できるか
/// @return 出力可能ならtrue
bool AWireNode::CanProvideOutput() const
{
	switch (mNodeType)
	{
	case EWireNodeType::Start:
		return bHasWire;

	case EWireNodeType::Relay:
		return mInputNodes.Num() > 0;//入力が1本以上あれば出力可能

	case EWireNodeType::Merge:
		return mInputNodes.Num() == mMaxInputs;//全入力が揃ったら出力可能

	case EWireNodeType::Split:
		return bIsConnected && mOutputNodes.Num() < mMaxOutputs;//入力があり、出力数未満

	default:
		return false;
	}
}

/// @brief 入力ノードを追加
/// @param InputNode 追加する入力ノード
void AWireNode::AddInputNode(AWireNode* InputNode)
{
	if (!InputNode || mInputNodes.Contains(InputNode))
	{
		return;
	}

	mInputNodes.Add(InputNode);

	switch (mNodeType) 
	{
	case EWireNodeType::Merge:
		UpdateMergedColor();
		if (mInputNodes.Num() >= mMaxInputs) 
		{
			bIsConnected = true;
			bHasWire = true;//入力が揃ったら出力可能にする
		}
		break;
	case EWireNodeType::Relay:
	case EWireNodeType::End:
		bIsConnected = true;
		mConnectedNode = InputNode;
		break;
	case EWireNodeType::Split:
		bIsConnected = true;//入力が1本あれば接続済み
		break;
	}
}

/// @brief 出力ノードを追加
/// @param OutputNode 追加する出力ノード
void AWireNode::AddOutputNode(AWireNode* OutputNode)
{
	if (!OutputNode || mOutputNodes.Contains(OutputNode))
	{
		return;
	}

	mOutputNodes.Add(OutputNode);
}

/// @brief 合成色を計算して更新
void AWireNode::UpdateMergedColor()
{
	if (mNodeType != EWireNodeType::Merge || mInputNodes.Num() < 2)
	{
		return;
	}

	//最初の2つの入力ノードの色を合成
	EWireColor Color1 = mInputNodes[0]->mWireColor;
	EWireColor Color2 = mInputNodes[1]->mWireColor;

	mMergedColor = MergeColors(Color1, Color2);
	mWireColor = mMergedColor;//出力色を更新

	//マテリアルを更新
	if (mMesh && mMesh->GetMaterial(0))
	{
		UMaterialInstanceDynamic* DynMat = mMesh->CreateDynamicMaterialInstance(0);
		if (DynMat)
		{
			DynMat->SetVectorParameterValue(FName("Color"), GetWireColorValue());
		}
	}
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
