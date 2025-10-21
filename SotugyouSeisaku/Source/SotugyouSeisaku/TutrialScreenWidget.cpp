// Fill out your copyright notice in the Description page of Project Settings.


#include "TutrialScreenWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"

/// @brief ネイティブコンストラクタ　ボタンのバインド
void UTutrialScreenWidget::NativeConstruct() 
{
    Super::NativeConstruct();

    //Blueprint内のボタンをまとめて取得
    CollectButtons();

    //CloseButton が WBP 上で設定されていればバインド
    if (mCloseButton)
    {
        mCloseButton->OnClicked.AddDynamic(this, &UTutrialScreenWidget::OnCloseButtonClicked);
    }

    //全てのNextボタンにイベント登録
    for (UButton* Btn : mNextButtons)
    {
        if (Btn)
        {
            Btn->OnClicked.AddDynamic(this, &UTutrialScreenWidget::OnNextClicked);
        }
    }

    //全てのBackボタンにイベント登録
    for (UButton* Btn : mBackButtons)
    {
        if (Btn)
        {
            Btn->OnClicked.AddDynamic(this, &UTutrialScreenWidget::OnBackClicked);
        }
    }
}

/// @brief ボタンのイベントを処理する関数
void UTutrialScreenWidget::CollectButtons()
{
    mNextButtons.Empty();
    mBackButtons.Empty();

    //名前で探す
    int32 Index = 0;
    while (true)
    {
        FString Name = FString::Printf(TEXT("mNextButton%d"), Index);
        UButton* Btn = Cast<UButton>(GetWidgetFromName(FName(*Name)));
        if (!Btn)
            break;
        mNextButtons.Add(Btn);
        Index++;
    }

    Index = 0;
    while (true)
    {
        FString Name = FString::Printf(TEXT("mBackButton%d"), Index);
        UButton* Btn = Cast<UButton>(GetWidgetFromName(FName(*Name)));
        if (!Btn)
            break;
        mBackButtons.Add(Btn);
        Index++;
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

/// @brief 次ページへ移行する関数
void UTutrialScreenWidget::OnNextClicked()
{
    //現在表示されているウィジェットのインデックス番号を取得
    int32 Index = mWidgetSwitcher->GetActiveWidgetIndex();
    mWidgetSwitcher->SetActiveWidgetIndex(FMath::Clamp(Index + 1, 0, mWidgetSwitcher->GetNumWidgets() - 1));
}

/// @brief 前ページへ戻る関数
void UTutrialScreenWidget::OnBackClicked()
{
    int32 Index = mWidgetSwitcher->GetActiveWidgetIndex();
    mWidgetSwitcher->SetActiveWidgetIndex(FMath::Clamp(Index - 1, 0, mWidgetSwitcher->GetNumWidgets() - 1));
}
