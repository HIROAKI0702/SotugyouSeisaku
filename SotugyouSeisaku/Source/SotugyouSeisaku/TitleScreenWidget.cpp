// Fill out your copyright notice in the Description page of Project Settings.
#include "TitleScreenWidget.h"
#include "Components/Button.h"
#include "MR_GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GMB_TitleScreen.h"

/// @brief コンストラクタ　タイトルウィジェットの初期設定
void UTitleScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();
	//GameInstance取得
	mGameInstance = Cast<UMR_GameInstance>(GetGameInstance());
	//ボタンのイベントをバインド
	if (mStartButton)
	{
		mStartButton->OnClicked.AddDynamic(this, &UTitleScreenWidget::OnStartButtonClicked);
	}
	if (mQuitButton)
	{
		mQuitButton->OnClicked.AddDynamic(this, &UTitleScreenWidget::OnQuitButtonClicked);
	}
	//マウスカーソルを表示
	mPlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (mPlayerController)
	{
		mPlayerController->bShowMouseCursor = true;
		mPlayerController->SetInputMode(FInputModeUIOnly());
	}
}

/// @brief GameModeのSE再生関数を呼び出す
void UTitleScreenWidget::PlayButtonClickSound()
{
	AGMB_TitleScreen* GameMode = Cast<AGMB_TitleScreen>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->PlayUIClickSound();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get GameMode!"));
	}
}

/// @brief ゲーム開始ボタンをクリックしたときに呼ばれる関数
void UTitleScreenWidget::OnStartButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Start button clicked"));

	// SE を再生
	PlayButtonClickSound();

	// SE の再生時間分、遅延してからレベル遷移
	GetWorld()->GetTimerManager().SetTimer(
		StageLoadTimerHandle,
		[this]()
		{
			if (mGameInstance)
			{
				mGameInstance->ReturnToStageSelect();
			}
		},
		0.5f,  // 0.5秒待機（SE の長さに合わせて調整）
		false
	);
}

/// @brief ゲーム終了ボタンを押したときに呼ばれる関数
void UTitleScreenWidget::OnQuitButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Quit button clicked"));

	// SE を再生
	PlayButtonClickSound();

	// SE の再生時間分、遅延してからゲーム終了
	GetWorld()->GetTimerManager().SetTimer(
		QuitTimerHandle,
		[this]()
		{
			//ゲーム終了
			APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
			UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
		},
		0.5f,  // 0.5秒待機（SE の長さに合わせて調整）
		false
	);
}