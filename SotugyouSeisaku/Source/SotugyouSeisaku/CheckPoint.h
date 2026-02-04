// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CheckPoint.generated.h"

class UBoxComponent;
class UParticleSystem;
class USoundBase;

UCLASS()
class SOTUGYOUSEISAKU_API ACheckPoint : public AActor
{
	GENERATED_BODY()

public:
	ACheckPoint();

	virtual void BeginPlay() override;

protected:
	// チェックポイント用のボックスコンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	UBoxComponent* CollisionBox;

	// チェックポイントのメッシュ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	class UStaticMeshComponent* MeshComponent;

	// チェックポイント通過時のパーティクル
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	UParticleSystem* ActivationParticle;

	// チェックポイント通過時のSE
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* ActivationSound;

	// チェックポイントのインデックス
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CheckPoint")
	int32 CheckPointIndex;

	// チェックポイントが有効化されているか
	UPROPERTY(BlueprintReadOnly, Category = "CheckPoint")
	bool bIsActivated;

public:
	// チェックポイント有効化関数
	UFUNCTION(BlueprintCallable, Category = "CheckPoint")
	void ActivateCheckPoint();

	// チェックポイント取得関数
	UFUNCTION(BlueprintCallable, Category = "CheckPoint")
	int32 GetCheckPointIndex() const { return CheckPointIndex; }

	// 有効化状態取得関数
	UFUNCTION(BlueprintCallable, Category = "CheckPoint")
	bool IsActivated() const { return bIsActivated; }

protected:
	// オーバーラップ開始時のコールバック
	UFUNCTION()
	void OnBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};