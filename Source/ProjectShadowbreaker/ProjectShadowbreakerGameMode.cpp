// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectShadowbreakerGameMode.h"
#include "ProjectShadowbreakerCharacter.h"
#include "UObject/ConstructorHelpers.h"

AProjectShadowbreakerGameMode::AProjectShadowbreakerGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
