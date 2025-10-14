// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MirrorActor.generated.h"

UCLASS()
class SOTUGYOUSEISAKU_API AMirrorActor : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> mRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> mMesh;
	
public:	
	// Sets default values for this actor's properties
	AMirrorActor();

	UPROPERTY(EditAnywhere, Category = "Mirror")
	float ReflectDistance = 5000.0f;

	void ReflectLaser(const FHitResult& Hit, const FVector& IncomingDir);
};
