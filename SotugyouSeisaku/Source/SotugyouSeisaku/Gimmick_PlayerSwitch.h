// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmick_PlayerSwitch.generated.h"

class UBoxComponent;
class ASotugyouSeisakuCharacter;

UCLASS()
class SOTUGYOUSEISAKU_API AGimmick_PlayerSwitch : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> mRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> mMesh;

	//レバーの判定エリア
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> mTriggerBox;
	
public:	
	// Sets default values for this actor's properties
	AGimmick_PlayerSwitch();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//オーバーラップイベント
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//切り替え先のプレイヤーキャラクター
	UPROPERTY(EditAnywhere, Category = "Switch Settings")
	ASotugyouSeisakuCharacter* mTargetPlayer;

	//最初にこのスイッチを使ったプレイヤー（オリジナル）
	UPROPERTY()
	ASotugyouSeisakuCharacter* mOriginalPlayer = nullptr;

	//クールダウン時間（秒）
	UPROPERTY(EditAnywhere, Category = "Switch Settings", meta = (ClampMin = "0.0"))
	float mSwitchCooldown = 1.0f;

	//現在範囲内にいるプレイヤー
	UPROPERTY()
	ASotugyouSeisakuCharacter* mCurrentPlayer = nullptr;

	//プレイヤーを切り替える
	void SwitchPlayer(ASotugyouSeisakuCharacter* NewPlayer, ASotugyouSeisakuCharacter* OldPlayer);

	//プレイヤーが範囲内でFキーを押したときに呼ばれる
	void OnInteract(ASotugyouSeisakuCharacter* InteractingPlayer);

	//クールダウン用：最後に切り替えた時刻
	float mLastSwitchTime = 0.0f;

	//プレイヤーが範囲内にいるか
	bool bPlayerInRange = false;

};
