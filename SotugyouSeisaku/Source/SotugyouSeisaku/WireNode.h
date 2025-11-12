// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "WireNode.generated.h"

//導線の色を定義
UENUM(BlueprintType)
enum class EWireColor : uint8
{
	Red UMETA(DisplayName = "Red"),
	Blue UMETA(DisplayName = "Blue"),
	Green UMETA(DisplayName = "Green"),
	Yellow UMETA(DisplayName = "Yellow"),
	Purple UMETA(DisplayName = "Purple")
};

//ノードのタイプ
UENUM(BlueprintType)
enum class EWireNodeType : uint8
{
	Start UMETA(DisplayName = "Start Point"),
	End UMETA(DisplayName = "End Point")
};

UCLASS()
class SOTUGYOUSEISAKU_API AWireNode : public AActor,public IInteractable
{
	GENERATED_BODY()

	//コンポーネント
	UPROPERTY(VisibleAnywhere)
	USceneComponent* mRoot;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* mMesh;

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* mInteractTrigger;
	
public:	
	// Sets default values for this actor's properties
	AWireNode();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//インタラクトインターフェース実装
	virtual void Interact_Implementation(ASotugyouSeisakuCharacter* PlayerCharacter) override;
	virtual bool CanInteract_Implementation(ASotugyouSeisakuCharacter* PlayerCharacter) const override;
	virtual FText GetInteractText_Implementation() const override;

	//設定
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire Settings")
	EWireColor mWireColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire Settings")
	EWireNodeType mNodeType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire Settings")
	int32 mPairID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire Settings")
	float mInteractDistance;

	//状態
	UPROPERTY(BlueprintReadOnly, Category = "Wire State")
	bool bIsConnected;

	UPROPERTY(BlueprintReadOnly, Category = "Wire State")
	AWireNode* mConnectedNode;

	//ワイヤーを持っているか
	UPROPERTY(BlueprintReadOnly, Category = "Wire State")
	bool bHasWire;

	//関数
	UFUNCTION(BlueprintCallable, Category = "Wire")
	void PickupWire(ASotugyouSeisakuCharacter* Player);

	UFUNCTION(BlueprintCallable, Category = "Wire")
	void ConnectWire(ASotugyouSeisakuCharacter* Player);

	UFUNCTION(BlueprintCallable, Category = "Wire")
	void Disconnect();

	UFUNCTION(BlueprintCallable, Category = "Wire")
	bool CanConnectTo(AWireNode* TargetNode) const;

	UFUNCTION(BlueprintCallable, Category = "Wire")
	FLinearColor GetWireColorValue() const;

	//オーバーラップイベント
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	bool bPlayerInRange;
	class ASotugyouSeisakuCharacter* mCurrentPlayer;
};
