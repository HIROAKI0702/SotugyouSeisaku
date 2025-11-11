// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GoalActor.generated.h"

class UBoxComponent;
class UGoalWidget;

UCLASS()
class SOTUGYOUSEISAKU_API AGoalActor : public AActor
{
	GENERATED_BODY()
	
	//ルートコンポーネント
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> mRoot;

	//ゴールのメッシュ
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> mMesh;

	//トリガーボックス（ゴール判定エリア）
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> mTriggerBox;

public:	
	// Sets default values for this actor's properties
	AGoalActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	//ゴールウィジェットのクラス
	UPROPERTY(EditAnywhere, Category = "Goal Settings")
	TSubclassOf<UGoalWidget> mGoalWidgetClass;

	//ゴールウィジェットのインスタンス
	UPROPERTY()
	TObjectPtr<UGoalWidget> mGoalWidget;

	//ゴールしたかどうか
	bool bGoalReached = false;

	//オーバーラップイベント
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
