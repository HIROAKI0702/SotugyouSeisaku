// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractWidget.generated.h"

/**
 * 
 */
UCLASS()
class SOTUGYOUSEISAKU_API UInteractWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    //インタラクトテキストを設定
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SetInteractText(const FText& NewText);

    //ウィジェットを表示
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void ShowWidget();

    //ウィジェットを非表示
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void HideWidget();

protected:
    //テキストブロック（Blueprintでバインド）
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* mInteractText;

    //キーアイコン用の画像
    UPROPERTY(meta = (BindWidget))
    class UImage* mKeyIcon;
};
