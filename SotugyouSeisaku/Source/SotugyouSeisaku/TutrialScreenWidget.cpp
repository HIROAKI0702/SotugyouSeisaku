// Fill out your copyright notice in the Description page of Project Settings.


#include "TutrialScreenWidget.h"
#include "Components/Button.h"

/// @brief ネイティブコンストラクタ　ボタンのバインド
void UTutrialScreenWidget::NativeConstruct() 
{
    Super::NativeConstruct();

    //CloseButton が WBP 上で設定されていればバインド
    if (mCloseButton)
    {
        mCloseButton->OnClicked.AddDynamic(this, &UTutrialScreenWidget::OnCloseButtonClicked);
    }
}

/// @brief クローズボタンをクリックしたときの処理
void UTutrialScreenWidget::OnCloseButtonClicked()
{
    //ウィジェットを画面から削除
    RemoveFromParent();

    //プレイヤーコントローラーの入力モードをゲーム用に戻す
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }
}
