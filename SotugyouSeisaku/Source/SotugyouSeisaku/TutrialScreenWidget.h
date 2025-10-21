// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TutrialScreenWidget.generated.h"

class UButton;
class UWidgetSwitcher;

/**
 * 
 */
UCLASS()
class SOTUGYOUSEISAKU_API UTutrialScreenWidget : public UUserWidget
{
	GENERATED_BODY()

    //WBPのボタンをバインド
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> mCloseButton;

    //Next/Back ボタンをまとめて管理する配列
    UPROPERTY()
    TArray<UButton*> mNextButtons;

    UPROPERTY()
    TArray<UButton*> mBackButtons;

    //ページ切り替え
    UPROPERTY(meta = (BindWidget))
    class UWidgetSwitcher* mWidgetSwitcher;

protected:
    virtual void NativeConstruct() override;

protected:
    //ボタン押下時に呼ばれるコールバック関数
    UFUNCTION()
    void OnCloseButtonClicked();

    UFUNCTION()
    void OnNextClicked();

    UFUNCTION()
    void OnBackClicked();

    //ウィジェットを名前で探すヘルパー
    void CollectButtons();
};
