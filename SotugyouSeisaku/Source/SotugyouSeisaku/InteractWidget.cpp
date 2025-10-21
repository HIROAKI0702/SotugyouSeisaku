// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractWidget.h"
#include "Components/TextBlock.h"

void UInteractWidget::SetInteractText(const FText& NewText)
{
    if (mInteractText)
    {
        mInteractText->SetText(NewText);
    }
}

void UInteractWidget::ShowWidget()
{
    SetVisibility(ESlateVisibility::Visible);
}

void UInteractWidget::HideWidget()
{
    SetVisibility(ESlateVisibility::Collapsed);
}
