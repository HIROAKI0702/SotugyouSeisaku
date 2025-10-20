// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GMB_StageSelectScreen.generated.h"

class UUserWidget;

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
};
