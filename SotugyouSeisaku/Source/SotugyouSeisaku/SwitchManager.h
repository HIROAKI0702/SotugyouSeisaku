// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SwitchManager.generated.h"

/**
 * 
 */
UCLASS()
class SOTUGYOUSEISAKU_API USwitchManager : public UObject
{
	GENERATED_BODY()
	
public:
    static USwitchManager* Get(UWorld* World);

    void RegisterSwitchUsage();

    bool IsRecentlyUsed() const;

private:
    float LastUsedTime = 0.0f;
    float Cooldown = 1.0f; //全スイッチ共通で1秒間無効
};
