// Fill out your copyright notice in the Description page of Project Settings.


#include "GMB_StageSelectScreen.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

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