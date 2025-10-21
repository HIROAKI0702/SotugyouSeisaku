// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SotugyouSeisakuGameMode.generated.h"

class UUserWidget;

UCLASS(minimalapi)
class ASotugyouSeisakuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASotugyouSeisakuGameMode();

protected:
    virtual void BeginPlay() override;

    //チュートリアルウィジェットのクラス
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> TutorialWidgetClass;

private:
    //作成されたウィジェットの参照
    UPROPERTY()
    UUserWidget* TutorialWidgetInstance;
};



