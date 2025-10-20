// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StageSelectWidget.generated.h"

class UButton;
class UMR_GameInstance;
class APlayerController;

/**
 * 
 */
UCLASS()
class SOTUGYOUSEISAKU_API UStageSelectWidget : public UUserWidget
{
	GENERATED_BODY()

	//ステージボタンの配列
	UPROPERTY(EditAnywhere, meta = (BindWidgetOptional))
	TArray<UButton*> mStageButtons;

	//戻るボタン
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* mBackButton;

protected:
	virtual void NativeConstruct() override;

private:
	//ボタン押下コールバック
	UFUNCTION()
	void OnStageButtonClicked(int32 StageIndex);

	UFUNCTION()
	void OnBackClicked();
};
