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
}

/// @brief ゲーム開始時に呼ばれる初期化処理
void AWirePuzzleManager::BeginPlay()
{
	Super::BeginPlay();

	//ドアの初期位置を保存
	if (mTargetDoor)
	{
		mDoorOriginalPosition = mTargetDoor->GetActorLocation();
		mDoorTargetPosition = mDoorOriginalPosition + mDoorMoveOffset;
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
	//レベル上の全WireNodeを検索
	TArray<AActor*> FoundNodes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWireNode::StaticClass(), FoundNodes);
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
	{
		return;
	}

	//パズルの完了状態をチェック
	CheckPuzzleCompletion();
}

/// @brief パズルの完了状態をチェック
void AWirePuzzleManager::CheckPuzzleCompletion()
{
	bool bAllConnected = true;//すべて接続されているか
	bool bAllCorrect = true;  //すべて正しく接続されているか

	//登録されたペアをチェック
	for (FWirePair& Pair : mWirePairs)
	{
		if (!Pair.StartNode || !Pair.EndNode)
		{
			continue;
		}

		//接続されているかチェック
		if (!Pair.StartNode->bIsConnected || !Pair.EndNode->bIsConnected)
		{
			bAllConnected = false;
			continue;
		}

		//正しく接続されているかチェック（スタートとエンドが互いに接続されているか）
		if (Pair.StartNode->mConnectedNode != Pair.EndNode)
		{
			bAllCorrect = false;

			//間違った接続でリセットする設定の場合
			if (bResetOnWrongConnection)
			{
				OnPuzzleFailure();
				return;
			}
		}

		//このペアは正しく接続されている
		Pair.bIsConnected = true;
	}

	//すべて正しく接続されている場合
	if (bAllConnected && bAllCorrect)
	{
		OnPuzzleSuccess();
	}
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
	if (!mTargetDoor)
	{
		return;
	}

	//現在位置を取得
	FVector CurrentPosition = mTargetDoor->GetActorLocation();

	//目標位置に向かって滑らかに移動
	FVector NewPosition = FMath::VInterpConstantTo(
		CurrentPosition,
		mDoorTargetPosition,
		DeltaTime,
		mDoorMoveSpeed
	);

	//新しい位置を設定
	mTargetDoor->SetActorLocation(NewPosition);
}
