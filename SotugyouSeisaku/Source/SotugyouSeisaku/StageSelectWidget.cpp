// Fill out your copyright notice in the Description page of Project Settings.


#include "StageSelectWidget.h"
#include "Components/Button.h"
#include "MR_GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetTree.h"
#include "GMB_StageSelectScreen.h"

/// @brief コンストラクタ　初期設定と各ボタンのバインド
void UStageSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//GameInstance取得
	mGameInstance = Cast<UMR_GameInstance>(GetGameInstance());
	if (!mGameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get GameInstance!"));
		return;
	}

	//ステージボタンを自動検出
	FindStageButtons();

	//戻るボタンのイベントをバインド
	if (mBackButton)
	{
		mBackButton->OnClicked.AddDynamic(this, &UStageSelectWidget::OnBackButtonClicked);
	}

	//各ステージボタンのイベントをバインド
	if (mStageButtons.IsValidIndex(0) && mStageButtons[0])
	{
		mStageButtons[0]->OnClicked.AddDynamic(this, &UStageSelectWidget::OnStage0ButtonClicked);
	}

	//ステージボタンの状態を更新
	UpdateStageButtons();

	//マウスカーソルを表示
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		PC->bShowMouseCursor = true;
		PC->SetInputMode(FInputModeUIOnly());
	}

	UE_LOG(LogTemp, Log, TEXT("StageSelectWidget constructed successfully"));
}

/// @brief ウィジェットからボタンを検出する関数
void UStageSelectWidget::FindStageButtons()
{
	mStageButtons.Empty();

	//WidgetTree から全てのウィジェットを取得
	TArray<UWidget*> AllWidgets;
	WidgetTree->GetAllWidgets(AllWidgets);

	//"StageButton_" で始まるボタンを検索
	TMap<int32, UButton*> ButtonMap; //インデックス順にソートするためのマップ

	for (UWidget* Widget : AllWidgets)
	{
		if (UButton* Button = Cast<UButton>(Widget))
		{
			FString ButtonName = Button->GetName();

			//"StageButton_0", "StageButton_1" などのパターンをチェック
			if (ButtonName.StartsWith(TEXT("StageButton_")))
			{
				//"_" の後の数字を取得
				FString IndexString = ButtonName.RightChop(12);//"StageButton_" は12文字
				if (IndexString.IsNumeric())
				{
					int32 Index = FCString::Atoi(*IndexString);
					ButtonMap.Add(Index, Button);

					UE_LOG(LogTemp, Log, TEXT("Found stage button: %s (Index: %d)"), *ButtonName, Index);
				}
			}
		}
	}

	//インデックス順にソートして配列に追加
	ButtonMap.KeySort([](int32 A, int32 B) { return A < B; });

	for (const auto& Pair : ButtonMap)
	{
		mStageButtons.Add(Pair.Value);
	}

	UE_LOG(LogTemp, Log, TEXT("Total stage buttons found: %d"), mStageButtons.Num());
}

/// @brief GameModeのSE再生関数を呼び出す
void UStageSelectWidget::PlayButtonClickSound()
{
	AGMB_StageSelectScreen* GameMode = Cast<AGMB_StageSelectScreen>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->PlayUIClickSound();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get GameMode!"));
	}
}

/// @brief 戻るボタンの関数
void UStageSelectWidget::OnBackButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Back button clicked"));

	PlayButtonClickSound();

	// SE の再生時間分、遅延してからレベル遷移
	GetWorld()->GetTimerManager().SetTimer(
		StageLoadTimerHandle,
		[this]()
		{
			if (mGameInstance)mGameInstance->ReturnToTitle();

		},
		0.5f,  // 0.5秒待機（SE の長さに合わせて調整）
		false
	);
}

/// @brief ステージ１に遷移する関数
void UStageSelectWidget::OnStage0ButtonClicked()
{
	PlayButtonClickSound();

	// SE の再生時間分、遅延してからレベル遷移
	GetWorld()->GetTimerManager().SetTimer(
		StageLoadTimerHandle,
		[this]()
		{
			if (mGameInstance) mGameInstance->LoadStage(0);
		},
		0.5f,  // 0.5秒待機（SE の長さに合わせて調整）
		false
	);

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
}

/// @brief ボタンのアップデート関数
void UStageSelectWidget::UpdateStageButtons()
{
	if (!mGameInstance)
	{
		return;
	}

	const TArray<FStageInfo>& Stages = mGameInstance->mStages;

	UE_LOG(LogTemp, Log, TEXT("Updating stage buttons. Total stages: %d"), Stages.Num());

	//各ステージボタンの状態を更新
	for (int32 i = 0; i < mStageButtons.Num(); i++)
	{
		if (mStageButtons[i] && Stages.IsValidIndex(i))
		{
			//アンロック状態に応じてボタンを有効/無効化
			mStageButtons[i]->SetIsEnabled(Stages[i].bIsUnlocked);

			//ロックされている場合は暗くする
			if (!Stages[i].bIsUnlocked)
			{
				mStageButtons[i]->SetColorAndOpacity(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f));
			}
		}
	}
}