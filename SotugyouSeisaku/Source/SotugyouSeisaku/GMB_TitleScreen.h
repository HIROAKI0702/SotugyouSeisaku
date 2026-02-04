// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GMB_TitleScreen.generated.h"

class UUserWidget;
class USoundBase;
class UAudioComponent;

/**
 *
 */
UCLASS()
class SOTUGYOUSEISAKU_API AGMB_TitleScreen : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGMB_TitleScreen();

protected:
	virtual void BeginPlay() override;

private:
	//タイトルウィジェットクラス
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> TitleWidgetClass;

	//生成されたウィジェットの参照
	UPROPERTY()
	UUserWidget* TitleWidgetInstance;

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
	//UI SE再生関数
	UFUNCTION(BlueprintCallable, Category = "Sound")
	void PlayUIClickSound();

	//BGM再生関数
	UFUNCTION(BlueprintCallable, Category = "Sound")
	void PlayTitleBGM();

	//BGM停止関数
	UFUNCTION(BlueprintCallable, Category = "Sound")
	void StopTitleBGM();
};