// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"
#include "Shooter/PlayerState/ShooterPlayerState.h"
#include "Shooter/GameState/ShooterGameState.h"
#include "Shooter/PlayerController/ShooterPlayerController.h"
#include "Kismet/GameplayStatics.h"

ATeamsGameMode::ATeamsGameMode()
{
	bTeamsMatch = true;
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AShooterGameState* SGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
	if (SGameState)
	{
		AShooterPlayerState* SPState = NewPlayer->GetPlayerState<AShooterPlayerState>();
		if (SPState && SPState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (SGameState->BlueTeam.Num() >= SGameState->RedTeam.Num())
			{
				SGameState->RedTeam.AddUnique(SPState);
				SPState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				SGameState->BlueTeam.AddUnique(SPState);
				SPState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	AShooterGameState* SGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
	AShooterPlayerState* SPState = Exiting->GetPlayerState<AShooterPlayerState>();
	if (SGameState && SPState)
	{
		if (SGameState->RedTeam.Contains(SPState))
		{
			SGameState->RedTeam.Remove(SPState);
		}
		if (SGameState->BlueTeam.Contains(SPState))
		{
			SGameState->BlueTeam.Remove(SPState);
		}
	}
}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	AShooterGameState* SGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
	if (SGameState)
	{
		for (auto PState : SGameState->PlayerArray)
		{
			AShooterPlayerState* SPState = Cast<AShooterPlayerState>(PState.Get());
			if (SPState && SPState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (SGameState->BlueTeam.Num() >= SGameState->RedTeam.Num())
				{
					SGameState->RedTeam.AddUnique(SPState);
					SPState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					SGameState->BlueTeam.AddUnique(SPState);
					SPState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}

float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	AShooterPlayerState* AttackerPState = Attacker->GetPlayerState<AShooterPlayerState>();
	AShooterPlayerState* VictimPState = Victim->GetPlayerState<AShooterPlayerState>();
	if (AttackerPState == nullptr || VictimPState == nullptr) return BaseDamage;
	if (VictimPState == AttackerPState)
	{
		return BaseDamage;
	}
	if (AttackerPState->GetTeam() == VictimPState->GetTeam())
	{
		return 0.f;
	}
	return BaseDamage;
}

void ATeamsGameMode::PlayerEliminated(AShooterCharacter* ElimmedCharacter, AShooterPlayerController* VictimController, AShooterPlayerController* AttackerController)
{
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);

	AShooterGameState* SGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
	AShooterPlayerState* AttackerPlayerState = AttackerController ? Cast<AShooterPlayerState>(AttackerController->PlayerState) : nullptr;
	if (SGameState && AttackerPlayerState)
	{
		if (AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			SGameState->BlueTeamScores();
		}
		if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			SGameState->RedTeamScores();
		}
	}
}
