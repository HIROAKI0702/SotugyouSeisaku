// Copyright Epic Games, Inc. All Rights Reserved.
#include "SotugyouSeisakuGameMode.h"
#include "SotugyouSeisakuCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"

/// @brief コンストラクタ　デフォルトのポーンクラスを設定
ASotugyouSeisakuGameMode::ASotugyouSeisakuGameMode()
{
    // set default pawn class to our Blueprinted character
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
    if (PlayerPawnBPClass.Class != NULL)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }
}

/// @brief ビギンプレイ　チュートリアルウィジェットの表示とBGM再生
void ASotugyouSeisakuGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (TutorialWidgetClass)
    {
        //プレイヤーコントローラーを取得
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            //ウィジェット作成
            TutorialWidgetInstance = CreateWidget<UUserWidget>(PC, TutorialWidgetClass);
            if (TutorialWidgetInstance)
            {
                //ウィジェットを画面に追加
                TutorialWidgetInstance->AddToViewport();
                //UI専用入力モードに切り替え
                FInputModeUIOnly InputMode;
                InputMode.SetWidgetToFocus(TutorialWidgetInstance->TakeWidget());
                InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                PC->SetInputMode(InputMode);
                PC->bShowMouseCursor = true;
                PC->bEnableClickEvents = true;
                PC->bEnableMouseOverEvents = true;
            }
        }
    }

    // BGM をループ再生
    if (BackgroundMusic)
    {
        PlayStageBGM();
    }
}

/// @brief ステージBGMをループ再生する関数
void ASotugyouSeisakuGameMode::PlayStageBGM()
{
    if (BackgroundMusic)
    {
        // AudioComponentがない場合は作成
        if (!BGMComponent)
        {
            BGMComponent = NewObject<UAudioComponent>(this);
            if (BGMComponent)
            {
                BGMComponent->RegisterComponent();
            }
        }

        // BGMをループ再生
        if (BGMComponent)
        {
            BGMComponent->SetSound(BackgroundMusic);
            BGMComponent->bAutoActivate = true;
            BGMComponent->SetVolumeMultiplier(0.6f);
            BGMComponent->bIsUISound = false;
            BGMComponent->Play(0.0f);

            UE_LOG(LogTemp, Log, TEXT("Playing Stage BGM"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Background Music is not set!"));
    }
}

/// @brief ステージBGMを停止する関数
void ASotugyouSeisakuGameMode::StopStageBGM()
{
    if (BGMComponent && BGMComponent->IsPlaying())
    {
        BGMComponent->Stop();
        UE_LOG(LogTemp, Log, TEXT("Stopping Stage BGM"));
    }
}