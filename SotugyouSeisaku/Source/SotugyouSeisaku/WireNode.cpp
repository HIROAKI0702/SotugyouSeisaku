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

	if (bCanDisconnect)
	{
		Disconnect();
		return;
	}

	//プレイヤーがワイヤーを持っていない場合
	if (!PlayerCharacter->IsCarryingWire())
	{
		//スタートノードからワイヤーを拾う
		if (bHasWire && mNodeType == EWireNodeType::Start && !bIsConnected)
		{
			PickupWire(PlayerCharacter);
		}
		//Relay/Merge/Splitノードから出力ワイヤーを拾う
		else if (CanProvideOutput() && (mNodeType == EWireNodeType::Relay ||
			mNodeType == EWireNodeType::Merge ||
			mNodeType == EWireNodeType::Split))
		{
			PickupWire(PlayerCharacter);
		}
	}
	else
	{
		//プレイヤーがワイヤーを持っている場合
		ConnectWire(PlayerCharacter);
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

	if (bIsConnected || mInputNodes.Num() > 0 || mOutputNodes.Num() > 0)
	{
		return true;//接続がある場合は外せる
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
		//スタートノード：ワイヤーをまだ持っている場合のみ
		if (mNodeType == EWireNodeType::Start)
		{
			return bHasWire;
		}

		//Relayノードはワイヤーを取り出せない
		if (mNodeType == EWireNodeType::Relay)
		{
			return false;
		}

		//Splitノード：入力があって、まだ最大出力数に達していない場合のみ
		if (mNodeType == EWireNodeType::Split)
		{
			return mInputNodes.Num() > 0 && mOutputNodes.Num() < mMaxOutputs;
		}

		return false;
	}
	else
	{
		//プレイヤーがワイヤーを持っている場合
		//全ノードで、まだ入力が可能なら接続可能
		bool CanConnect = ((mNodeType == EWireNodeType::End ||
			mNodeType == EWireNodeType::Relay ||
			mNodeType == EWireNodeType::Merge ||
			mNodeType == EWireNodeType::Split) &&
			CanAcceptInput());
		return CanConnect;
	}
}

/// @brief インタラクト時に表示するテキストを取得
/// @return 表示するテキスト
FText AWireNode::GetInteractText_Implementation() const
{
	if (bIsConnected)
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

		//接続済みなら外せる
		bCanDisconnect = bIsConnected;
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
		bCanDisconnect = false;
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
	AWireConnection* NewConnection = GetWorld()->SpawnActor<AWireConnection>(Manager->mWireConnectionClass);
	if (NewConnection)
	{
		NewConnection->SetupConnection(this, nullptr, Player);
		NewConnection->AttachToPlayer(Player);

		Player->SetCarryingWire(this, NewConnection);

		//Splitノードから出力ワイヤーを取り出す場合
		if (mNodeType == EWireNodeType::Split)
		{
			//出力ノードを追加してカウント
			AWireNode* DummyNode = nullptr;  // Split用のダミーノード
			mOutputNodes.Add(DummyNode);
		}
		//スタート/Relay/Mergeノードからワイヤーを拾う場合
		else if (mNodeType == EWireNodeType::Start || mNodeType == EWireNodeType::Relay || mNodeType == EWireNodeType::Merge)
		{
			bHasWire = false;
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
	{
		//End ノードは1本のワイヤーを受け付ける
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

			// パズルマネージャーに通知
			AWirePuzzleManager* Manager = AWirePuzzleManager::Get(GetWorld());
			if (Manager)
			{
				Manager->RegisterConnection(StartNode, this, Player);
			}
		}
	}
	break;

	case EWireNodeType::Relay:
	{
		//Relay ノードは1本のワイヤーを受け付ける
		if (CanAcceptInput())
		{
			bIsConnected = true;
			mConnectedNode = StartNode;
			mConnectedWire = Connection;

			AddInputNode(StartNode);
			StartNode->AddOutputNode(this);

			Connection->SetupConnection(StartNode, this, nullptr);
			Player->ClearCarryingWire();

			//Relay は出力可能にしてワイヤーを持つ
			bHasWire = true;

			//パズルマネージャーに通知
			AWirePuzzleManager* Manager = AWirePuzzleManager::Get(GetWorld());
			if (Manager)
			{
				Manager->RegisterConnection(StartNode, this, Player);
			}
		}
	}
	break;

	case EWireNodeType::Merge:
	{
		//Merge ノードは複数のワイヤーを受け付ける
		if (CanAcceptInput())
		{
			AddInputNode(StartNode);
			StartNode->AddOutputNode(this);

			Connection->SetupConnection(StartNode, this, nullptr);
			Player->ClearCarryingWire();

			//全入力が揃ったら合成色を計算して出力可能にする
			if (mInputNodes.Num() >= mMaxInputs)
			{
				UpdateMergedColor();
				bIsConnected = true;
				bHasWire = true;
			}

			// パズルマネージャーに通知
			AWirePuzzleManager* Manager = AWirePuzzleManager::Get(GetWorld());
			if (Manager)
			{
				Manager->RegisterConnection(StartNode, this, Player);
			}
		}
	}
	break;

	case EWireNodeType::Split:
	{
		//Splitノードは1本の入力ワイヤーを受け付ける
		if (CanAcceptInput())
		{
			bIsConnected = true;
			mConnectedNode = StartNode;
			mConnectedWire = Connection;

			AddInputNode(StartNode);
			StartNode->AddOutputNode(this);

			Connection->SetupConnection(StartNode, this, nullptr);
			Player->ClearCarryingWire();

			//Split は出力可能にしてワイヤーを持つ
			bHasWire = true;

			// パズルマネージャーに通知
			AWirePuzzleManager* Manager = AWirePuzzleManager::Get(GetWorld());
			if (Manager)
			{
				Manager->RegisterConnection(StartNode, this, Player);
			}
		}
	}
	break;

	default:
		break;
	}
}

/// @brief ワイヤーの接続を解除する
void AWireNode::Disconnect()
{

	if (!bIsConnected && mInputNodes.Num() == 0 && mOutputNodes.Num() == 0)
	{
		return;//何も繋がってない
	}

	//接続中のワイヤーを削除
	if (mConnectedWire)
	{
		mConnectedWire->Destroy();
		mConnectedWire = nullptr;
	}

	//接続先のノードを取得
	AWireNode* OtherNode = mConnectedNode;

	//自ノード側のリセット
	bIsConnected = false;
	bHasWire = (mNodeType == EWireNodeType::Start);//Startは元々ワイヤー持つ
	mConnectedNode = nullptr;

	mInputNodes.Empty();
	mOutputNodes.Empty();

	//相手ノード側もリセット
	if (OtherNode)
	{
		OtherNode->bIsConnected = false;
		OtherNode->bHasWire = (OtherNode->mNodeType == EWireNodeType::Start);
		OtherNode->mConnectedNode = nullptr;

		OtherNode->mInputNodes.Remove(this);
		OtherNode->mOutputNodes.Remove(this);
	}

	//パズルマネージャーへ通知
	if (AWirePuzzleManager* Manager = AWirePuzzleManager::Get(GetWorld()))
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
		//Relay は入力があれば出力可能
		return mInputNodes.Num() > 0 && bHasWire;

	case EWireNodeType::Merge:
		//全入力が揃ったら出力可能
		return mInputNodes.Num() == mMaxInputs && bHasWire;

	case EWireNodeType::Split:
		//Split は入力があれば出力可能
		return mInputNodes.Num() > 0 && bHasWire;

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
		mWireColor = InputNode->mWireColor;

		// マテリアルを更新
		if (mMesh && mMesh->GetMaterial(0))
		{
			UMaterialInstanceDynamic* DynMat = mMesh->CreateDynamicMaterialInstance(0);
			if (DynMat)
			{
				DynMat->SetVectorParameterValue(FName("Color"), GetWireColorValue());
			}
		}
		break;

	case EWireNodeType::Relay:
		bIsConnected = true;
		mConnectedNode = InputNode;
		bHasWire = true;//入力を受けたら出力可能に
		mWireColor = InputNode->mWireColor;

		// マテリアルを更新
		if (mMesh && mMesh->GetMaterial(0))
		{
			UMaterialInstanceDynamic* DynMat = mMesh->CreateDynamicMaterialInstance(0);
			if (DynMat)
			{
				DynMat->SetVectorParameterValue(FName("Color"), GetWireColorValue());
			}
		}

		break;

	case EWireNodeType::End:
		bIsConnected = true;
		mConnectedNode = InputNode;
		break;

	case EWireNodeType::Split:
		bIsConnected = true;
		mConnectedNode = InputNode;
		bHasWire = true;//入力を受けたら出力可能に
		mWireColor = InputNode->mWireColor;

		// マテリアルを更新
		if (mMesh && mMesh->GetMaterial(0))
		{
			UMaterialInstanceDynamic* DynMat = mMesh->CreateDynamicMaterialInstance(0);
			if (DynMat)
			{
				DynMat->SetVectorParameterValue(FName("Color"), GetWireColorValue());
			}
		}

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
