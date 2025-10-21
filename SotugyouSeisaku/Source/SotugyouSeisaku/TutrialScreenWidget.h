// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TutrialScreenWidget.generated.h"

class UButton;

/**
 * 
 */
UCLASS()
class SOTUGYOUSEISAKU_API UTutrialScreenWidget : public UUserWidget
{
	GENERATED_BODY()

    //WBPのボタンをバインド
    UPROPERTY(meta = (BindWidget))
    UButton* mCloseButton;
	
protected:
    virtual void NativeConstruct() override;

protected:
    //ボタン押下時に呼ばれるコールバック関数
    UFUNCTION()
    void OnCloseButtonClicked();
};
