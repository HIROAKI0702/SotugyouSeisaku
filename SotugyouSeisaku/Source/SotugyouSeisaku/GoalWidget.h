// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GoalWidget.generated.h"

class UButton;
class UTextBlock;

/**
 * 
 */
UCLASS()
class SOTUGYOUSEISAKU_API UGoalWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

public:
	///ウィジェットを表示する
	UFUNCTION(BlueprintCallable, Category = "Goal Widget")
	void ShowGoalWidget();

	///ウィジェットを非表示にする
	UFUNCTION(BlueprintCallable, Category = "Goal Widget")
	void HideGoalWidget();

protected:
	//タイトルに戻るボタン
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BackToTitleButton;

	//次のステージに進むボタン
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NextStageButton;

	//ゴールテキスト表示
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GoalText;

private:
	//タイトルに戻るボタンが押された時の処理
	UFUNCTION()
	void OnBackToTitleClicked();

	//次のステージボタンが押された時の処理
	UFUNCTION()
	void OnNextStageClicked();

	//次のステージのレベル名
	UPROPERTY(EditAnywhere, Category = "Goal Widget")
	FName NextStageLevelName = TEXT("Stage_2");

	//タイトル画面のレベル名
	UPROPERTY(EditAnywhere, Category = "Goal Widget")
	FName TitleLevelName = TEXT("TitleScreen");
};
