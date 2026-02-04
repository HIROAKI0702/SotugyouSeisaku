// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GMB_StageSelectScreen.generated.h"
class UUserWidget;
class USoundBase;
class UAudioComponent;

/**
 *
 */
UCLASS()
class SOTUGYOUSEISAKU_API AGMB_StageSelectScreen : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGMB_StageSelectScreen();
protected:
	virtual void BeginPlay() override;
private:
	//ステージ選択ウィジェットクラス
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> StageSelectWidgetClass;
	//生成されたウィジェットの参照
	UPROPERTY()
	UUserWidget* StageSelectWidgetInstance;

	//UI操作のSE
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	USoundBase* mUIClickSE;

	//タイトルBGM
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	USoundBase* mTitleBGM;

	//BGM用のAudioComponent
	UPROPERTY()
	UAudioComponent* mBGMAudioComponent;

public:
	//BGM再生関数
	UFUNCTION(BlueprintCallable, Category = "Sound")
	void PlayStageSelectBGM();

	//UI SE再生関数
	UFUNCTION(BlueprintCallable, Category = "Sound")
	void PlayUIClickSound();

	//BGM停止関数
	UFUNCTION(BlueprintCallable, Category = "Sound")
	void StopStageSelectBGM();
};