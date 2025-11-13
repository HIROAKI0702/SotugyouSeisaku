// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WireConnection.generated.h"

class USplineComponent;
class USplineMeshComponent;

UCLASS()
class SOTUGYOUSEISAKU_API AWireConnection : public AActor
{
	GENERATED_BODY()

	//コンポーネント
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> mRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> mSpline;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineMeshComponent> mSplineMesh;
	
public:	
	// Sets default values for this actor's properties
	AWireConnection();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//ワイヤーに使用するメッシュ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire Settings")
	TObjectPtr<UStaticMesh> mWireMesh;

	//ワイヤーに使用するマテリアル
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire Settings")
	TObjectPtr<UMaterialInterface> mWireMaterial;

	TArray<USplineMeshComponent*>mSplineMeshes;

	//ワイヤーの太さ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire Settings")
	float mWireThickness;

	//ワイヤーの接続を設定
	UFUNCTION(BlueprintCallable, Category = "Wire")
	void SetupConnection(class AWireNode* StartNode, class AWireNode* EndNode, class ASotugyouSeisakuCharacter* Player);

	//ワイヤーの視覚的な表現を更新
	UFUNCTION(BlueprintCallable, Category = "Wire")
	void UpdateWireVisuals();

	//ワイヤーの色を設定
	UFUNCTION(BlueprintCallable, Category = "Wire")
	void SetWireColor(FLinearColor Color);

	//ワイヤーをプレイヤーにアタッチ
	UFUNCTION(BlueprintCallable, Category = "Wire")
	void AttachToPlayer(ASotugyouSeisakuCharacter* Player);

private:
	//ワイヤーの開始ノード
	UPROPERTY()
	TObjectPtr<AWireNode> mStartNode;

	//ワイヤーの終了ノード
	UPROPERTY()
	TObjectPtr<AWireNode> mEndNode;

	//ワイヤーを運んでいるプレイヤー
	UPROPERTY()
	TObjectPtr<ASotugyouSeisakuCharacter> mCarryingPlayer;

	//動的に生成されたマテリアルインスタンス
	TObjectPtr<UMaterialInstanceDynamic> mDynamicMaterial;

	//プレイヤーにアタッチされているか
	bool bAttachedToPlayer;
};
