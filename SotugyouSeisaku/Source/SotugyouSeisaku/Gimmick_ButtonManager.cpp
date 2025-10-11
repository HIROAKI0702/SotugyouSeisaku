// Fill out your copyright notice in the Description page of Project Settings.

#include "Gimmick_ButtonManager.h"
#include "Gimmick_Button.h"

/// @brief コンストラクタ　ボタンマネージャーの各種設定
AGimmick_ButtonManager::AGimmick_ButtonManager()
{
	PrimaryActorTick.bCanEverTick = true;

	mDoorMoveOffset = FVector(400.0f, 0.0f, 0.0f);
	mDoorMoveSpeed = 200.0f;
	bResetOnFailure = true;
	bResetAfterSuccess = false;
}

void AGimmick_ButtonManager::BeginPlay()
{
	Super::BeginPlay();

	//ドアの初期位置を保存
	if (mTargetDoor)
	{
		mDoorOriginalPosition = mTargetDoor->GetActorLocation();
		mDoorTargetPosition = mDoorOriginalPosition + mDoorMoveOffset;
	}
	
	//各ボタンにマネージャーを登録
	for (int32 i = 0; i < mButtonSequence.Num(); i++)
	{
		if (mButtonSequence[i])
		{
			mButtonSequence[i]->SetButtonManager(this);
		}
	}
}

void AGimmick_ButtonManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ドアを動かす
	if (mTargetDoor)
	{
		MoveDoor(DeltaTime);
	}
}

/// @brief ボタンが押されたかどうかチェックする関数
/// @param PressedButton //押されたボタンアクタ
void AGimmick_ButtonManager::OnButtonPressed(AGimmick_Button* PressedButton)
{
	//すでにクリア済みの場合は何もしない
	if (bSequenceCompleted && !bResetAfterSuccess)
	{
		return;
	}

	//押されたボタンが次に押すべきボタンか確認
	if (mCurrentStep < mButtonSequence.Num() && mButtonSequence[mCurrentStep] == PressedButton)
	{
		//正解
		mCurrentStep++;

		UE_LOG(LogTemp, Error, TEXT("Success!!!"));

		//すべてのボタンを正しい順番で押した
		if (mCurrentStep >= mButtonSequence.Num())
		{
			OnSequenceSuccess();
		}
	}
	else
	{
		//間違ったボタンが押された
		OnSequenceFailure(PressedButton);
	}
}

/// @brief ボタンが離されたかチェックする関数
/// @param ReleasedButton 離したボタンアクタ
void AGimmick_ButtonManager::OnButtonReleased(AGimmick_Button* ReleasedButton)
{
	//ボタンを離してもシーケンスは継続（必要に応じて変更可能）
	UE_LOG(LogTemp, Log, TEXT("Button released: %s"), *GetNameSafe(ReleasedButton));
}

/// @brief ボタンの順番を正解したことを通知する関数
void AGimmick_ButtonManager::OnSequenceSuccess()
{
	bSequenceCompleted = true;
	bDoorOpen = true;
	// ビジュアルフィードバック（オプション）
	// 例: パーティクルエフェクト、サウンド再生など
}

/// @brief ボタンの押す順番を間違えた事を通知する関数
/// @param WrongButton 間違えたボタンアクタ
void AGimmick_ButtonManager::OnSequenceFailure(AGimmick_Button* WrongButton)
{
	int32 ExpectedIndex = mCurrentStep;
	int32 ActualIndex = mButtonSequence.Find(WrongButton);
}

/// @brief ボタンを離した後リセットする関数
void AGimmick_ButtonManager::ResetSequence()
{
	mCurrentStep = 0;
	bSequenceCompleted = false;
}

/// @brief 正解した後、ドアを開く関数
/// @param DeltaTime //フレーム間の経過時間
void AGimmick_ButtonManager::MoveDoor(float DeltaTime)
{
	FVector CurrentPosition = mTargetDoor->GetActorLocation();
	FVector TargetPosition;

	//ドアの状態に応じて目標位置を決定
	if (bDoorOpen)
	{
		TargetPosition = mDoorTargetPosition;//開く
	}
	else
	{
		TargetPosition = mDoorOriginalPosition;//閉じる
	}

	//滑らかに移動
	FVector NewPosition = FMath::VInterpConstantTo(
		CurrentPosition,
		TargetPosition,
		DeltaTime,
		mDoorMoveSpeed
	);

	mTargetDoor->SetActorLocation(NewPosition);
}