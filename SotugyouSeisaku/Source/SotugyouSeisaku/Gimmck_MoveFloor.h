// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmck_MoveFloor.generated.h"

//移動パターンの列挙型
UENUM(BlueprintType)
enum class EFloorMovementPattern : uint8
{
	Horizontal_X UMETA(DisplayName = "横移動 (X軸)"),
	Horizontal_Y UMETA(DisplayName = "横移動 (Y軸)"),
	Vertical_Z UMETA(DisplayName = "縦移動 (Z軸・上下)"),
	Diagonal_XY UMETA(DisplayName = "斜め移動 (XY平面)"),
	Diagonal_XZ UMETA(DisplayName = "斜め移動 (XZ平面)"),
	Diagonal_YZ UMETA(DisplayName = "斜め移動 (YZ平面)"),
	Circle_XY UMETA(DisplayName = "円運動 (XY平面)"),
	Circle_XZ UMETA(DisplayName = "円運動 (XZ平面)"),
	Circle_YZ UMETA(DisplayName = "円運動 (YZ平面)"),
	Custom UMETA(DisplayName = "カスタム (手動設定)")
};

UCLASS()
class SOTUGYOUSEISAKU_API AGimmck_MoveFloor : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> mRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> mMesh;
	
public:	
	// Sets default values for this actor's properties
	AGimmck_MoveFloor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//移動パターンの選択
	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	EFloorMovementPattern mMovementPattern = EFloorMovementPattern::Horizontal_Y;

	//移動距離（パターンによって使い方が異なる）
	UPROPERTY(EditAnywhere, Category = "Movement Settings", meta = (EditCondition = "mMovementPattern != EFloorMovementPattern::Custom"))
	float mMoveDistance = 500.0f;

	//カスタム移動量（Customパターンの場合のみ使用）
	UPROPERTY(EditAnywhere, Category = "Movement Settings", meta = (EditCondition = "mMovementPattern == EFloorMovementPattern::Custom"))
	FVector mCustomMoveOffset = FVector(0.0f, 500.0f, 0.0f);

	//移動速度（cm/秒）
	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float mMoveSpeed = 200.0f;

	//到着時の待機時間（秒）※円運動では無視
	UPROPERTY(EditAnywhere, Category = "Movement Settings", meta = (EditCondition = "mMovementPattern != EFloorMovementPattern::Circle_XY && mMovementPattern != EFloorMovementPattern::Circle_XZ && mMovementPattern != EFloorMovementPattern::Circle_YZ"))
	float mWaitTime = 1.0f;

	//自動で開始するか
	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	bool bAutoStart = true;

	//プレイヤーを一緒に動かすか
	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	bool bMoveActorsOnFloor = true;

	//開始位置（自動で保存）
	FVector mStartPosition;

	//終了位置（計算で保存）
	FVector mEndPosition;

	//円運動用の中心位置
	FVector mCircleCenter;

	//円運動用の現在角度（ラジアン）
	float mCircleAngle = 0.0f;

	//現在の移動方向（1 = 終了位置へ、-1 = 開始位置へ）
	int32 mDirection = 1;

	//現在待機中か
	bool bIsWaiting = false;

	//待機タイマー
	float mWaitTimer = 0.0f;

	//床の上に乗っているアクター
	UPROPERTY()
	TArray<AActor*> mActorsOnFloor;

	//オーバーラップイベント
	UFUNCTION()
	void OnFloorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnFloorEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//移動パターンに応じて終了位置を計算
	void CalculateEndPosition();

	//往復移動の処理
	void UpdateLinearMovement(float DeltaTime);

	//円運動の処理
	void UpdateCircularMovement(float DeltaTime);

};
