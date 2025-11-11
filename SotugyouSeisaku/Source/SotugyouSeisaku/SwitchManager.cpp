// Fill out your copyright notice in the Description page of Project Settings.


#include "SwitchManager.h"
#include "Engine/World.h"

USwitchManager* USwitchManager::Get(UWorld* World)
{
    static USwitchManager* Instance = nullptr;
    if (!Instance)
    {
        Instance = NewObject<USwitchManager>();
        Instance->AddToRoot();//GCされないようにする
    }
    return Instance;
}

void USwitchManager::RegisterSwitchUsage()
{
    if (UWorld* World = GetWorld())
    {
        LastUsedTime = World->GetTimeSeconds();
    }
}

bool USwitchManager::IsRecentlyUsed() const
{
    if (const UWorld* World = GetWorld())
    {
        return (World->GetTimeSeconds() - LastUsedTime) < Cooldown;
    }
    return false;
}
