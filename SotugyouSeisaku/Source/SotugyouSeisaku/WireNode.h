// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "WireNode.generated.h"

class UBoxComponent;

//導線の色を定義する列挙型
UENUM(BlueprintType)
enum class EWireColor : uint8
{
	Red UMETA(DisplayName = "Red"),
	Blue UMETA(DisplayName = "Blue"),
	Green UMETA(DisplayName = "Green"),
	Yellow UMETA(DisplayName = "Yellow"),
	Purple UMETA(DisplayName = "Purple")
};

//ノードのタイプを定義する列挙型
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

	//ルートコンポーネント
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> mRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> mMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> mInteractTrigger;
	
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

	//ワイヤーが接続されているか
	UPROPERTY(BlueprintReadOnly, Category = "Wire State")
	bool bIsConnected;

	UPROPERTY(BlueprintReadOnly, Category = "Wire State")
	AWireNode* mConnectedNode;

	//このノードがワイヤーを持っているか（スタートノードのみ）
	UPROPERTY(BlueprintReadOnly, Category = "Wire State")
	bool bHasWire;

	//ワイヤーを拾う（スタートノード用）
	UFUNCTION(BlueprintCallable, Category = "Wire")
	void PickupWire(ASotugyouSeisakuCharacter* Player);

	//ワイヤーを接続する（エンドノード用）
	UFUNCTION(BlueprintCallable, Category = "Wire")
	void ConnectWire(ASotugyouSeisakuCharacter* Player);

	//ワイヤーの接続を解除する
	UFUNCTION(BlueprintCallable, Category = "Wire")
	void Disconnect();

	//指定したノードと接続可能かチェック
	UFUNCTION(BlueprintCallable, Category = "Wire")
	bool CanConnectTo(AWireNode* TargetNode) const;

	//ワイヤーの色を取得（FLinearColor形式）
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
	//プレイヤーが範囲内にいるか
	bool bPlayerInRange;
	//範囲内にいる現在のプレイヤー
	class ASotugyouSeisakuCharacter* mCurrentPlayer;
};
