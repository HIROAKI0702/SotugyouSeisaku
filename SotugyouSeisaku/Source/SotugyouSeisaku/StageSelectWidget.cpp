// Fill out your copyright notice in the Description page of Project Settings.


#include "StageSelectWidget.h"
#include "Components/Button.h"
#include "MR_GameInstance.h"
#include "Kismet/GameplayStatics.h"

void UStageSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//ステージボタンが存在すれば、すべてにイベント登録
	for (int32 i = 0; i < mStageButtons.Num(); ++i)
	{
		if (mStageButtons[i])
		{
			//ステージ番号をキャプチャしたラムダでクリック時の挙動を定義
			//mStageButtons[i]->OnClicked.AddDynamic(this, &UStageSelectWidget::OnStageButtonClicked);
		}
	}

	//戻るボタンのバインド
	if (mBackButton)
	{
		mBackButton->OnClicked.AddDynamic(this, &UStageSelectWidget::OnBackClicked);
	}
}

void UStageSelectWidget::OnStageButtonClicked(int32 StageIndex)
{
	if (UMR_GameInstance* GameInstance = Cast<UMR_GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GameInstance->LoadStage(StageIndex);
	}
}

void UStageSelectWidget::OnBackClicked()
{
	if (UMR_GameInstance* GameInstance = Cast<UMR_GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GameInstance->ReturnToTitle();
	}
}