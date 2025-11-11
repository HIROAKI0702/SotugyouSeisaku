// Fill out your copyright notice in the Description page of Project Settings.


#include "GoalWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

/// @brief ウィジェットが構築された時に呼ばれる
void UGoalWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//ボタンのクリックイベントをバインド
	if (BackToTitleButton)
	{
		BackToTitleButton->OnClicked.AddDynamic(this, &UGoalWidget::OnBackToTitleClicked);
	}

	if (NextStageButton)
	{
		NextStageButton->OnClicked.AddDynamic(this, &UGoalWidget::OnNextStageClicked);
	}

	//初期状態では非表示
	SetVisibility(ESlateVisibility::Hidden);
}

/// @brief ゴールウィジェットを表示する
void UGoalWidget::ShowGoalWidget()
{
	SetVisibility(ESlateVisibility::Visible);

	//マウスカーソルを表示し、UIにフォーカス
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
}

/// @brief ゴールウィジェットを非表示にする
void UGoalWidget::HideGoalWidget()
{
	SetVisibility(ESlateVisibility::Hidden);

	//入力モードをゲームに戻す
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
}

/// @brief タイトルに戻るボタンが押された時の処理
void UGoalWidget::OnBackToTitleClicked()
{
	//タイトル画面に遷移
	UGameplayStatics::OpenLevel(GetWorld(), TitleLevelName);
}

/// @brief 次のステージボタンが押された時の処理
void UGoalWidget::OnNextStageClicked()
{
	//次のステージに遷移
	UGameplayStatics::OpenLevel(GetWorld(), NextStageLevelName);
}

