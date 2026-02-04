// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TitleScreenWidget.generated.h"

class UButton;
class UMR_GameInstance;
class APlayerController;

/**
 *
 */
UCLASS()
class SOTUGYOUSEISAKU_API UTitleScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

private:
	//ゲーム開始ボタン
	UPROPERTY(meta = (BindWidget))
	UButton* mStartButton;

	//ゲーム終了ボタン
	UPROPERTY(meta = (BindWidget))
	UButton* mQuitButton;

	//GameInstance参照
	UPROPERTY()
	UMR_GameInstance* mGameInstance;

	//PlayerController参照
	UPROPERTY()
	APlayerController* mPlayerController;

	//ボタンクリックイベント
	UFUNCTION()
	void OnStartButtonClicked();

	UFUNCTION()
	void OnQuitButtonClicked();

	//SE再生関数
	void PlayButtonClickSound();

	//タイマーハンドル
	FTimerHandle StageLoadTimerHandle;
	FTimerHandle QuitTimerHandle;
};