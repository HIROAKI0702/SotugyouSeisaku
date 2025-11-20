// Fill out your copyright notice in the Description page of Project Settings.


#include "WirePuzzleManager.h"
#include "WireConnection.h"
#include "Kismet/GameplayStatics.h"


// Sets default values

/// @brief コンストラクタ　パズルマネージャーの各種設定
AWirePuzzleManager::AWirePuzzleManager()
{
	//毎フレームTickを呼ぶ
	PrimaryActorTick.bCanEverTick = true;

	//デフォルト値を設定
	mTargetDoor = nullptr;
	bResetOnWrongConnection = true;
	bPuzzleCompleted = false;
	bDoorOpen = false;

	//ドアの移動設定
	mDoorMoveOffset = FVector(0.0f, 0.0f, 300.0f);
	mDoorMoveSpeed = 200.0f;

	//初期化
	mAllNodes.Empty();
	mActiveConnections.Empty();
}

/// @brief ゲーム開始時に呼ばれる初期化処理
void AWirePuzzleManager::BeginPlay()
{
	Super::BeginPlay();

	//ドアの初期位置を保存
	if (mTargetDoor)
	{
		//Blueprint の StaticMesh1 / StaticMesh2 を名前で取得
		UStaticMeshComponent* DoorParts1 =
			Cast<UStaticMeshComponent>(mTargetDoor->GetDefaultSubobjectByName(TEXT("StaticMesh1")));

		UStaticMeshComponent* DoorParts2 =
			Cast<UStaticMeshComponent>(mTargetDoor->GetDefaultSubobjectByName(TEXT("StaticMesh2")));

		if (DoorParts1) mLowerOriginalPos = DoorParts1->GetRelativeLocation();
		if (DoorParts2) mUpperOriginalPos = DoorParts2->GetRelativeLocation();
	}

	//レベル上のノードを初期化
	InitializeNodes();
}

/// @brief 毎フレーム呼ばれる更新処理
/// @param DeltaTime 前フレームからの経過時間
void AWirePuzzleManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//ドアが開く状態ならドアを動かす
	if (mTargetDoor && bDoorOpen)
	{
		MoveDoor(DeltaTime);
	}
}

/// @brief ノードを初期化
void AWirePuzzleManager::InitializeNodes()
{
	mAllNodes.Empty();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWireNode::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (AWireNode* Node = Cast<AWireNode>(Actor))
		{
			mAllNodes.Add(Node);

			//既に接続済みのノードがある場合、その視覚ワイヤーをアクティブリストに追加して管理できるようにする
			if (Node->mConnectedWire)
			{
				mActiveConnections.Add(Node->mConnectedWire);
			}
		}
	}
}

/// @brief ワールド内のマネージャーインスタンスを取得
/// @param World 検索するワールド
/// @return マネージャーのインスタンス
AWirePuzzleManager* AWirePuzzleManager::Get(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	//ワールド内のマネージャーを検索
	AActor* FoundActor = UGameplayStatics::GetActorOfClass(World, AWirePuzzleManager::StaticClass());
	return Cast<AWirePuzzleManager>(FoundActor);
}

/// @brief ワイヤーの接続を登録
/// @param StartNode 開始ノード
/// @param EndNode 終了ノード
/// @param Player 接続したプレイヤー
void AWirePuzzleManager::RegisterConnection(AWireNode* StartNode, AWireNode* EndNode, ASotugyouSeisakuCharacter* Player)
{
	if (!StartNode || !EndNode)
		return;

	//StartNode が持っている視覚用ワイヤーがあれば登録
	if (StartNode->mConnectedWire && !mActiveConnections.Contains(StartNode->mConnectedWire))
	{
		mActiveConnections.Add(StartNode->mConnectedWire);
	}
	//EndNodeのワイヤー
	if (EndNode->mConnectedWire && !mActiveConnections.Contains(EndNode->mConnectedWire))
	{
		mActiveConnections.Add(EndNode->mConnectedWire);
	}

	//相互の参照関係を作る
	if (!StartNode->mOutputNodes.Contains(EndNode))
	{
		StartNode->mOutputNodes.Add(EndNode);
	}

	if (!EndNode->mInputNodes.Contains(StartNode))
	{
		EndNode->mInputNodes.Add(StartNode);
	}

	//bIsConnected フラグはノード側のロジックで既にセットされているはずだが、
	if (StartNode->mOutputNodes.Num() > 0)
	{
		StartNode->bIsConnected = true;
	}
	if (EndNode->mInputNodes.Num() > 0)
	{
		EndNode->bIsConnected = true;
	}

	//Merge ノードは入力が揃ったら自動出力（ノードが持つ関数を呼ぶ）
	if (EndNode->mNodeType == EWireNodeType::Merge)
	{
		if (EndNode->CanProvideOutput())
		{
			//ノード内で出力ワイヤーを生成する実装があれば呼ぶ
			if (EndNode->mConnectedWire && !mActiveConnections.Contains(EndNode->mConnectedWire))
			{
				mActiveConnections.Add(EndNode->mConnectedWire);
			}
		}
	}

	//パズル状態をチェック
	CheckPuzzleCompletion();
}

/// @brief パズルの完了状態をチェック
void AWirePuzzleManager::CheckPuzzleCompletion()
{
	bool bAllConnected = true;
	bool bAllCorrect = true;

	if (mWirePairs.Num() == 0)
	{
		return;
	}

	//各ペアを検査
	for (FWirePair& Pair : mWirePairs)
	{
		//ペアが設定されていなければスキップ（ただしパズル全体が未完了となる）
		if (!Pair.StartNode || !Pair.EndNode)
		{
			bAllConnected = false;
			continue;
		}

		//Start が接続済か、または最終的な経路があるかを確認する
		if (!Pair.StartNode->bIsConnected)
		{
			bAllConnected = false;
			continue;
		}
		if (!Pair.EndNode->bIsConnected)
		{
			bAllConnected = false;
			continue;
		}

		//StartNode から EndNode へ到達できるか（Relay/Merge/Split を辿る）
		if (!IsPathConnected(Pair.StartNode, Pair.EndNode))
		{
			bAllCorrect = false;

			//設定によっては失敗として即リセット
			if (bResetOnWrongConnection)
			{
				OnPuzzleFailure();
				return;
			}
		}
		else
		{
			Pair.bIsConnected = true;
		}
	}

	//全て接続済かつ正解なら成功
	if (bAllConnected && bAllCorrect)
	{
		OnPuzzleSuccess();
	}
}

/// @brief スタートノードからエンドノードまで経路がつながっているか確認（Relay/Merge/Split対応）
/// @param StartNode スタートノード
/// @param EndNode エンドノード
/// @return 経路がつながっていればtrue
bool AWirePuzzleManager::IsPathConnected(AWireNode* StartNode, AWireNode* EndNode)
{
	if (!StartNode || !EndNode)
	{
		return false;
	}

	//DFS（深さ優先探索）で経路を探索
	TSet<AWireNode*> Visited;
	bool bConnected = TracePathDFS(StartNode, EndNode, Visited);

	return bConnected;
}

/// @brief 深さ優先探索で経路を探索
/// @param CurrentNode 現在のノード
/// @param TargetNode 目標ノード
/// @param Visited 訪問済みのノード
/// @return 経路が存在すればtrue
bool AWirePuzzleManager::TracePathDFS(AWireNode* CurrentNode, AWireNode* TargetNode, TSet<AWireNode*>& Visited)
{
	if (!CurrentNode)
	{
		return false;
	}

	//目標に到達
	if (CurrentNode == TargetNode)
	{
		return true;
	}

	//すでに訪問済み
	if (Visited.Contains(CurrentNode))
	{
		return false;
	}

	Visited.Add(CurrentNode);

	int32 NodeType = (int32)CurrentNode->mNodeType;

	//出力ノードを探索
	for (AWireNode* OutputNode : CurrentNode->mOutputNodes)
	{
		if (OutputNode)
		{
			int32 OutputType = (int32)OutputNode->mNodeType;
			
			if (OutputNode->bIsConnected)
			{
				if (TracePathDFS(OutputNode, TargetNode, Visited))
				{
					return true;
				}
			}
		}
	}

	return false;
}

/// @brief パズルをリセット
void AWirePuzzleManager::ResetPuzzle()
{
	//レベル上の全ノードを取得
	TArray<AActor*> FoundNodes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWireNode::StaticClass(), FoundNodes);

	//すべてのノードの接続を解除
	for (AActor* Actor : FoundNodes)
	{
		if (AWireNode* Node = Cast<AWireNode>(Actor))
		{
			Node->Disconnect();
		}
	}

	//すべてのワイヤー接続を削除
	for (AWireConnection* Connection : mActiveConnections)
	{
		if (Connection)
		{
			Connection->Destroy();
		}
	}
	mActiveConnections.Empty();

	//ペアのステータスをリセット
	for (FWirePair& Pair : mWirePairs)
	{
		Pair.bIsConnected = false;
	}

	bPuzzleCompleted = false;
}

/// @brief パズル成功時に呼ばれる
void AWirePuzzleManager::OnPuzzleSuccess()
{
	//すでに完了している場合は何もしない
	if (bPuzzleCompleted)
	{
		return;
	}

	bPuzzleCompleted = true;
	bDoorOpen = true;
}

/// @brief パズル失敗時に呼ばれる
void AWirePuzzleManager::OnPuzzleFailure()
{
	//TODO: 失敗時のエフェクトやサウンドを再生

	//1秒後にリセット
	FTimerHandle ResetHandle;
	GetWorld()->GetTimerManager().SetTimer(ResetHandle, this,
		&AWirePuzzleManager::ResetPuzzle, 1.0f, false);
}

/// @brief ドアを動かす
/// @param DeltaTime フレーム間の経過時間
void AWirePuzzleManager::MoveDoor(float DeltaTime)
{
	if (!mTargetDoor) return;

	//Blueprint内のStaticMesh1（下）とStaticMesh2（上）を取得
	UStaticMeshComponent* DoorParts1 =
		Cast<UStaticMeshComponent>(mTargetDoor->GetDefaultSubobjectByName(TEXT("StaticMesh1")));

	UStaticMeshComponent* DoorParts2 =
		Cast<UStaticMeshComponent>(mTargetDoor->GetDefaultSubobjectByName(TEXT("StaticMesh2")));

	if (!DoorParts1 || !DoorParts2) return;

	//目標オフセット
	float SlideAmount = 220.f;//上下の移動量

	FVector LowerClosedPos = mLowerOriginalPos;
	FVector UpperClosedPos = mUpperOriginalPos;

	FVector LowerOpenPos = LowerClosedPos - FVector(0, 0, SlideAmount);//下へスライド
	FVector UpperOpenPos = UpperClosedPos + FVector(0, 0, SlideAmount);//上へスライド

	FVector LowerTarget = bDoorOpen ? LowerOpenPos : LowerClosedPos;
	FVector UpperTarget = bDoorOpen ? UpperOpenPos : UpperClosedPos;

	//補間移動
	FVector NewLowerPos = FMath::VInterpConstantTo(
		DoorParts1->GetRelativeLocation(), LowerTarget, DeltaTime, mDoorMoveSpeed
	);

	FVector NewUpperPos = FMath::VInterpConstantTo(
		DoorParts2->GetRelativeLocation(), UpperTarget, DeltaTime, mDoorMoveSpeed
	);

	//パーツの位置を更新
	DoorParts1->SetRelativeLocation(NewLowerPos);
	DoorParts2->SetRelativeLocation(NewUpperPos);
}
