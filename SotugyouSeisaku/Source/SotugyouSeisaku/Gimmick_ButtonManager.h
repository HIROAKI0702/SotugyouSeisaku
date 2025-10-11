// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmick_ButtonManager.generated.h"

class AGimmick_Button;

UCLASS()
class SOTUGYOUSEISAKU_API AGimmick_ButtonManager : public AActor
{
	GENERATED_BODY()

public:
	AGimmick_ButtonManager();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

private:
	//管理するボタンのリスト（順番に押す必要がある）
	UPROPERTY(EditAnywhere, Category = "Button Sequence")
	TArray<AGimmick_Button*> mButtonSequence;

	//制御するドア（開閉対象）
	UPROPERTY(EditAnywhere, Category = "Button Sequence")
	AActor* mTargetDoor;

	//ドアの移動方向
	UPROPERTY(EditAnywhere, Category = "Button Sequence")
	FVector mDoorMoveOffset = FVector(0.0f, 0.0f, 300.0f);

	//ドアの移動速度
	UPROPERTY(EditAnywhere, Category = "Button Sequence")
	float mDoorMoveSpeed = 200.0f;

	//失敗したらリセットするか
	UPROPERTY(EditAnywhere, Category = "Button Sequence")
	bool bResetOnFailure = true;

	//成功後にボタンをリセットするか
	UPROPERTY(EditAnywhere, Category = "Button Sequence")
	bool bResetAfterSuccess = false;	// === 内部状態 ===

	//現在何番目のボタンを待っているか（0から始まる）
	int32 mCurrentStep = 0;

	//すべて正しい順番で押されたか
	bool bSequenceCompleted = false;

	//ドアが開いているか
	bool bDoorOpen = false;

	//ドアの初期位置
	FVector mDoorOriginalPosition;

	//ドアの目標位置
	FVector mDoorTargetPosition;

public:
	//ボタンが押されたときに呼ばれる
	UFUNCTION()
	void OnButtonPressed(AGimmick_Button* PressedButton);

	//ボタンが離されたときに呼ばれる
	UFUNCTION()
	void OnButtonReleased(AGimmick_Button* ReleasedButton);

private:
	//シーケンスをリセット
	void ResetSequence();

	//ドアを動かす
	void MoveDoor(float DeltaTime);

	//正解時の処理
	void OnSequenceSuccess();

	//失敗時の処理
	void OnSequenceFailure(AGimmick_Button* WrongButton);
};