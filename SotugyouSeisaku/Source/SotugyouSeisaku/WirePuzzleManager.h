// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WireNode.h"
#include "WirePuzzleManager.generated.h"

USTRUCT(BlueprintType)
struct FWirePair
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class AWireNode* StartNode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class AWireNode* EndNode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWireColor RequiredColor;

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

	//ê›íË
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
	TArray<FWirePair> mWirePairs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
	AActor* mTargetDoor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
	bool bResetOnWrongConnection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
	TSubclassOf<class AWireConnection> mWireConnectionClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Settings")
	FVector mDoorMoveOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Settings")
	float mDoorMoveSpeed;

	//èÛë‘
	UPROPERTY(BlueprintReadOnly, Category = "Puzzle State")
	bool bPuzzleCompleted;

	//ä÷êî
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void RegisterConnection(AWireNode* StartNode, AWireNode* EndNode, ASotugyouSeisakuCharacter* Player);

	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void CheckPuzzleCompletion();

	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void ResetPuzzle();

	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void OnPuzzleSuccess();

	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void OnPuzzleFailure();

	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	static AWirePuzzleManager* Get(UWorld* World);

private:
	void InitializeNodes();
	void MoveDoor(float DeltaTime);

	FVector mDoorOriginalPosition;
	FVector mDoorTargetPosition;
	bool bDoorOpen;

	UPROPERTY()
	TArray<class AWireConnection*> mActiveConnections;
};
