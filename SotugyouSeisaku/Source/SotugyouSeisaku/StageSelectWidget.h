// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StageSelectWidget.generated.h"
class UButton;
class UMR_GameInstance;
/**
 *
 */
UCLASS()
class SOTUGYOUSEISAKU_API UStageSelectWidget : public UUserWidget
{
	GENERATED_BODY()
	//タイトルに戻るボタン
	UPROPERTY(meta = (BindWidget))
	UButton* mBackButton;
protected:
	virtual void NativeConstruct() override;
private:
	//ボタンクリックイベント
	UFUNCTION()
	void OnBackButtonClicked();
	//StageSelectWidget.h
	UFUNCTION()
	void OnStage0ButtonClicked();
	//GameInstance参照
	UMR_GameInstance* mGameInstance;
	//ステージボタンの配列（動的に取得）
	TArray<UButton*> mStageButtons;
	//ステージボタンを自動検出
	void FindStageButtons();
	//ステージボタンの状態を更新
	void UpdateStageButtons();
	//ボタンクリック時のSE再生
	void PlayButtonClickSound();

	FTimerHandle StageLoadTimerHandle;
};