// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SotugyouSeisakuGameMode.generated.h"

class UUserWidget;
class USoundBase;
class UAudioComponent;

UCLASS()
class SOTUGYOUSEISAKU_API ASotugyouSeisakuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASotugyouSeisakuGameMode();

	virtual void BeginPlay() override;

protected:
	// チュートリアル用ウィジェット
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> TutorialWidgetClass;

	// 生成されたウィジェット
	UPROPERTY()
	UUserWidget* TutorialWidgetInstance;

	// BGM
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* BackgroundMusic;

	// BGM用AudioComponent
	UPROPERTY()
	UAudioComponent* BGMComponent;

public:
	// BGM再生関数
	UFUNCTION(BlueprintCallable, Category = "Sound")
	void PlayStageBGM();

	// BGM停止関数
	UFUNCTION(BlueprintCallable, Category = "Sound")
	void StopStageBGM();
};