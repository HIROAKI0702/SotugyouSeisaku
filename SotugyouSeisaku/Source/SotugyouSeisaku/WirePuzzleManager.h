// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WireNode.h"
#include "WirePuzzleManager.generated.h"

//ワイヤーのペア情報を保持する構造体
USTRUCT(BlueprintType)
struct FWirePair
{
	GENERATED_BODY()

	//スタートノード
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AWireNode> StartNode;

	//エンドノード
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AWireNode> EndNode;

	//必要な色
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWireColor RequiredColor;

	//ペアID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PairID;

	UPROPERTY(BlueprintReadOnly)
	bool bIsConnected;

	FWirePair()
		: StartNode(nullptr)
		, EndNode(nullptr)
		, RequiredColor(EWireColor::Red)
		, PairID(0)
		, bIsConnected(false)
	{
	}
};

UCLASS()
class SOTUGYOUSEISAKU_API AWirePuzzleManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWirePuzzleManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//正解のワイヤーペアのリスト
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
	TArray<FWirePair> mWirePairs;

	//パズル完了時に開くドア
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
	AActor* mTargetDoor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door Parts", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* DoorFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door Parts", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* DoorLower;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door Parts", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* DoorUpper;

	//間違った接続をした時にリセットするか
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
	bool bResetOnWrongConnection;

	//ワイヤー接続のクラス
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
	TSubclassOf<class AWireConnection> mWireConnectionClass;

	//ドアの移動オフセット
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Settings")
	FVector mDoorMoveOffset;

	//ドアの移動速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Settings")
	float mDoorMoveSpeed;

	//���
	UPROPERTY(BlueprintReadOnly, Category = "Puzzle State")
	bool bPuzzleCompleted;

	//ワイヤーの接続を登録
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void RegisterConnection(AWireNode* StartNode, AWireNode* EndNode, ASotugyouSeisakuCharacter* Player);

	//パズルの完了状態をチェック
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void CheckPuzzleCompletion();

	//パズルをリセット
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void ResetPuzzle();

	//パズル成功時に呼ばれる
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void OnPuzzleSuccess();

	//パズル失敗時に呼ばれる
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void OnPuzzleFailure();

	//ワールド内のマネージャーインスタンスを取得
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	static AWirePuzzleManager* Get(UWorld* World);

private:
	//ノードを初期化
	void InitializeNodes();
	//ドアを動かす
	void MoveDoor(float DeltaTime);

	//ドアの初期位置
	FVector mDoorOriginalPosition;
	//ドアの目標位置
	FVector mDoorTargetPosition;

	FVector mLowerOriginalPos;
	FVector mUpperOriginalPos;

	//ドアが開いているか
	bool bDoorOpen;

	//経路がつながっているか確認（DFS）
	bool IsPathConnected(AWireNode* StartNode, AWireNode* EndNode);

	//深さ優先探索で経路を探索
	bool TracePathDFS(AWireNode* CurrentNode, AWireNode* TargetNode, TSet<AWireNode*>& Visited);

	//現在アクティブなワイヤー接続のリスト
	UPROPERTY()
	TArray<class AWireConnection*> mActiveConnections;

	//レベル上の全ノードをキャッシュする
	UPROPERTY()
	TArray<AWireNode*> mAllNodes;
};
