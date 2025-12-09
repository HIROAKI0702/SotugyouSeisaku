// Copyright Epic Games, Inc. All Rights Reserved.

#include "SotugyouSeisakuGameMode.h"
#include "SotugyouSeisakuCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

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

/// @brief ビギンプレイ　チュートリアルウィジェットの表示
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

    if (BackgroundMusic)
    {
        BGMComponent = UGameplayStatics::SpawnSound2D(
            this,
            BackgroundMusic,
            1.0f,    
            1.0f,    
            0.0f,    
            nullptr,
            true,    
            false    
        );
    }
}
