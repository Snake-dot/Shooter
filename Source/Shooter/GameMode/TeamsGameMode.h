// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShooterGameMode.h"
#include "TeamsGameMode.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API ATeamsGameMode : public AShooterGameMode
{
	GENERATED_BODY()
	
public:
	ATeamsGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;
	virtual void PlayerEliminated(class AShooterCharacter* ElimmedCharacter, class AShooterPlayerController* VictimController, class AShooterPlayerController* AttackerController) override;

protected:
	virtual void HandleMatchHasStarted() override;
};
