// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmick_PushBlock.h"
#include "Components/BoxComponent.h"
#include "Gimmick_Button.generated.h"

class AGimmick_ButtonManager;

UCLASS()
class SOTUGYOUSEISAKU_API AGimmick_Button : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> mRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> mMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> mMesh2;

	//ボタンの判定エリア
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> mTriggerBox;

	//動かすドア（エディタで指定）
	UPROPERTY(EditAnywhere, Category = "Button Settings")
	AActor* mTargetDoor;
	

public:	
	// Sets default values for this actor's properties
	AGimmick_Button();

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

	//マネージャーを設定（マネージャーから呼ばれる）
	void SetButtonManager(AGimmick_ButtonManager* Manager) { mButtonManager = Manager; }

	//ボタンが押されているか取得
	bool IsPressed() const { return bIsPressed; }

	//ブロックを移動させる
	void MoveBlock(float DeltaTime);

	//ブロックの移動方向（ローカル座標）
	UPROPERTY(EditAnywhere, Category = "Button Settings")
	FVector mMoveDir = FVector(0.0f, 200.0f, 0.0f); // デフォルトはY軸方向に200cm

	//移動速度（cm/秒）
	UPROPERTY(EditAnywhere, Category = "Button Settings")
	float mMoveSpeed = 200.0f;

	//ボタンを離したら元に戻すか
	UPROPERTY(EditAnywhere, Category = "Button Settings")
	bool bReturnToOriginal = true;

	//ボタンが押されているか
	UPROPERTY(VisibleAnywhere, Category = "Button State")
	bool bIsPressed = false;

	//ブロックの初期位置
	FVector mBlockOriginalPosition;

	//ブロックの目標位置
	FVector mBlockTargetPosition;

	//現在ボタンに乗っているアクターの数
	int32 mOverlappingActorCount = 0;

	//ボタンマネージャー（複数ボタンシステム用）
	UPROPERTY()
	AGimmick_ButtonManager* mButtonManager = nullptr;
};
