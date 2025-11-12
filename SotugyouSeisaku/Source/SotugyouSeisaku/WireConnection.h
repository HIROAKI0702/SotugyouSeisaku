// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WireConnection.generated.h"

UCLASS()
class SOTUGYOUSEISAKU_API AWireConnection : public AActor
{
	GENERATED_BODY()

	//コンポーネント
	UPROPERTY(VisibleAnywhere)
	USceneComponent* mRoot;

	UPROPERTY(VisibleAnywhere)
	class USplineComponent* mSpline;

	UPROPERTY(VisibleAnywhere)
	class USplineMeshComponent* mSplineMesh;
	
public:	
	// Sets default values for this actor's properties
	AWireConnection();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//設定
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire Settings")
	UStaticMesh* mWireMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire Settings")
	UMaterialInterface* mWireMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire Settings")
	float mWireThickness;

	//関数
	UFUNCTION(BlueprintCallable, Category = "Wire")
	void SetupConnection(class AWireNode* StartNode, class AWireNode* EndNode, class ASotugyouSeisakuCharacter* Player);

	UFUNCTION(BlueprintCallable, Category = "Wire")
	void UpdateWireVisuals();

	UFUNCTION(BlueprintCallable, Category = "Wire")
	void SetWireColor(FLinearColor Color);

	UFUNCTION(BlueprintCallable, Category = "Wire")
	void AttachToPlayer(ASotugyouSeisakuCharacter* Player);

private:
	UPROPERTY()
	class AWireNode* mStartNode;

	UPROPERTY()
	class AWireNode* mEndNode;

	UPROPERTY()
	ASotugyouSeisakuCharacter* mCarryingPlayer;

	UMaterialInstanceDynamic* mDynamicMaterial;

	bool bAttachedToPlayer;
};
