// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GMB_TitleScreen.generated.h"

class TitleScreenWidget;

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
	//タイトル画面ウィジェットクラス
	UPROPERTY(EditDefaultsOnly,Category = "UI")
	TSubclassOf<UUserWidget> TitleWidgetClass;

	//生成されたウィジェットの参照
	UPROPERTY()
	UUserWidget* TitleWidgetInstance;
};
