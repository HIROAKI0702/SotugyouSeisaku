// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmick_JumpPad.generated.h"

UCLASS()
class SOTUGYOUSEISAKU_API AGimmick_JumpPad : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> mRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> mMesh;
	
public:	
	// Sets default values for this actor's properties
	AGimmick_JumpPad();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//ジャンプの強さ
	UPROPERTY(EditAnywhere, Category = "JumpPad")
	float JumpPower = 2000.f;

	//コールバック関数
	UFUNCTION()
	void OnPadOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult);
};
