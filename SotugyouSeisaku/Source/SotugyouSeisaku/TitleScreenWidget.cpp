// Fill out your copyright notice in the Description page of Project Settings.


#include "TitleScreenWidget.h"
#include "Components/Button.h"
#include "MR_GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

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

	if (mOptionsButton)
	{
		mOptionsButton->OnClicked.AddDynamic(this, &UTitleScreenWidget::OnOptionsButtonClicked);
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

/// @brief ゲーム開始ボタンをクリックしたときに呼ばれる関数
void UTitleScreenWidget::OnStartButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Start button clicked"));

	//ステージ選択画面へ
	if (mGameInstance)
	{
		mGameInstance->ReturnToStageSelect();
	}
}

/// @brief 設定ボタンを押したときに呼ばれる関数
void UTitleScreenWidget::OnOptionsButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Options button clicked"));
	//オプション画面の処理（今回は省略）
}

/// @brief ゲーム終了ボタンを押したときに呼ばれる関数
void UTitleScreenWidget::OnQuitButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Quit button clicked"));

	//ゲーム終了
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
}
