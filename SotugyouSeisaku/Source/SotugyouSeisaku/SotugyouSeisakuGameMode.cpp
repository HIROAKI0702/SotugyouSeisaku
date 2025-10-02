// Copyright Epic Games, Inc. All Rights Reserved.

#include "SotugyouSeisakuGameMode.h"
#include "SotugyouSeisakuCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASotugyouSeisakuGameMode::ASotugyouSeisakuGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
