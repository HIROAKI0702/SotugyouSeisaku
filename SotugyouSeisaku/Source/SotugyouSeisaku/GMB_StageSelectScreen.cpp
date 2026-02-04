// Fill out your copyright notice in the Description page of Project Settings.
#include "GMB_StageSelectScreen.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"

/// @brief コンストラクタ　ステージ選択画面のウィジェットを表示するゲームモードベース
AGMB_StageSelectScreen::AGMB_StageSelectScreen()
{
	//プレイヤーコントローラーを設定（UI専用）
	PlayerControllerClass = APlayerController::StaticClass();
	//デフォルトポーンは不要
	DefaultPawnClass = nullptr;
}

void AGMB_StageSelectScreen::BeginPlay()
{
	Super::BeginPlay();

	//BGM を再生
	PlayStageSelectBGM();

	//ウィジェットを作成して表示
	if (StageSelectWidgetClass)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PC)
		{
			StageSelectWidgetInstance = CreateWidget<UUserWidget>(PC, StageSelectWidgetClass);
			if (StageSelectWidgetInstance)
			{
				StageSelectWidgetInstance->AddToViewport();
				//マウスカーソルを表示
				PC->bShowMouseCursor = true;
				PC->bEnableClickEvents = true;
				PC->bEnableMouseOverEvents = true;
				//入力モードをUIのみに設定
				FInputModeUIOnly InputMode;
				InputMode.SetWidgetToFocus(StageSelectWidgetInstance->TakeWidget());
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				PC->SetInputMode(InputMode);
			}
		}
	}
}

/// @brief UI操作のSEを再生する関数
void AGMB_StageSelectScreen::PlayUIClickSound()
{
	if (mUIClickSE)
	{
		// PlayerController を取得
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PC)
		{
			// PlaySound2D は PlayerController を第2引数に指定
			UGameplayStatics::PlaySound2D(PC, mUIClickSE, 1.0f);
		}
	}
}

/// @brief タイトルBGMを再生する関数
void AGMB_StageSelectScreen::PlayStageSelectBGM()
{
	if (mTitleBGM)
	{
		//AudioComponentがない場合は作成
		if (!mBGMAudioComponent)
		{
			mBGMAudioComponent = NewObject<UAudioComponent>(this);
			if (mBGMAudioComponent)
			{
				mBGMAudioComponent->RegisterComponent();
			}
		}

		//BGMをループ再生
		if (mBGMAudioComponent)
		{
			mBGMAudioComponent->SetSound(mTitleBGM);
			mBGMAudioComponent->bAutoActivate = true;
			mBGMAudioComponent->SetVolumeMultiplier(0.6f);
			mBGMAudioComponent->bIsUISound = false;
			mBGMAudioComponent->Play(0.0f);

			UE_LOG(LogTemp, Log, TEXT("Playing Title BGM"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Title BGM is not set!"));
	}
}

/// @brief タイトルBGMを停止する関数
void AGMB_StageSelectScreen::StopStageSelectBGM()
{
	if (mBGMAudioComponent && mBGMAudioComponent->IsPlaying())
	{
		mBGMAudioComponent->Stop();
		UE_LOG(LogTemp, Log, TEXT("Stopping Title BGM"));
	}
}