// Fill out your copyright notice in the Description page of Project Settings.


#include "GMB_TitleScreen.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

/// @brief コンストラクタ　タイトル画面のウィジェットを表示するゲームモードベース
AGMB_TitleScreen::AGMB_TitleScreen()
{
	//プレイヤーコントローラーを設定（UI専用）
	PlayerControllerClass = APlayerController::StaticClass();

	//デフォルトポーンは不要
	DefaultPawnClass = nullptr;
}

void AGMB_TitleScreen::BeginPlay()
{
	Super::BeginPlay();

	//ウィジェットを作成して表示
	if (TitleWidgetClass)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PC)
		{
			TitleWidgetInstance = CreateWidget<UUserWidget>(PC, TitleWidgetClass);
			if (TitleWidgetInstance)
			{
				TitleWidgetInstance->AddToViewport();

				//マウスカーソルを表示
				PC->bShowMouseCursor = true;
				PC->bEnableClickEvents = true;
				PC->bEnableMouseOverEvents = true;

				//入力モードをUIのみに設定
				FInputModeUIOnly InputMode;
				InputMode.SetWidgetToFocus(TitleWidgetInstance->TakeWidget());
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				PC->SetInputMode(InputMode);
			}
		}
	}
}

