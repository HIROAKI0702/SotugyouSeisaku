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

	//スタートボタン
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> mStartButton;

	//オプションボタン
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> mOptionsButton;

	//終了ボタン
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> mQuitButton;
	
protected:
	virtual void NativeConstruct() override;

private:

	//スタートボタンの押下チェック関数
	UFUNCTION()
	void OnStartButtonClicked();

	//設定ボタンの押下チェック関数
	UFUNCTION()
	void OnOptionsButtonClicked();

	//ゲーム終了の押下チェック関数
	UFUNCTION()
	void OnQuitButtonClicked();

	TObjectPtr<UMR_GameInstance> mGameInstance;
	TObjectPtr<APlayerController> mPlayerController;
};
