// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmick_PushBlock.generated.h"

UCLASS()
class SOTUGYOUSEISAKU_API AGimmick_PushBlock : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGimmick_PushBlock();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* mMesh;

	UPROPERTY()
	ASotugyouSeisakuCharacter* mPushingPlayer;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//押す力
	int mPushPower = 0;

	//押されているかどうか
	bool bIsBeginePushed = false;

	//押す処理
	UFUNCTION()
	void StartPushing(class ASotugyouSeisakuCharacter* PushingPlayer);
	UFUNCTION()
	void StopPushing();

	//プレイヤーに追従させる
	UFUNCTION()
	void MoveWithPlayer(const FVector& DeltaMove);

};
