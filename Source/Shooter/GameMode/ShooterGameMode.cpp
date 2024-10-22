// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterGameMode.h"
#include "Shooter/Character/ShooterCharacter.h"
#include "Shooter/PlayerController/ShooterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Shooter/PlayerState/ShooterPlayerState.h"
#include "Shooter/GameState/ShooterGameState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

AShooterGameMode::AShooterGameMode()
{
	bDelayedStart = true;
}

void AShooterGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AShooterGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void AShooterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AShooterPlayerController* ShooterPlayer = Cast<AShooterPlayerController>(*It);
		if (ShooterPlayer)
		{
			ShooterPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void AShooterGameMode::PlayerEliminated(AShooterCharacter* ElimmedCharacter, AShooterPlayerController* VictimController, AShooterPlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;
	AShooterPlayerState* AttackerPlayerState = AttackerController ? Cast<AShooterPlayerState>(AttackerController->PlayerState) : nullptr;
	AShooterPlayerState* VictimPlayerState = VictimController ? Cast<AShooterPlayerState>(VictimController->PlayerState) : nullptr;

	AShooterGameState* ShooterGameState = GetGameState<AShooterGameState>();

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && ShooterGameState)
	{
		TArray<AShooterPlayerState*> PlayersCurrentlyInTheLead;
		for (auto LeadPlayer : ShooterGameState->TopScoringPlayers)
		{
			PlayersCurrentlyInTheLead.Add(LeadPlayer);
		}

		AttackerPlayerState->AddToScore(1.f);
		ShooterGameState->UpdateTopScore(AttackerPlayerState);
		if (ShooterGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			AShooterCharacter* Leader = Cast<AShooterCharacter>(AttackerPlayerState->GetPawn());
			if (Leader)
			{
				Leader->MulticastGainedTheLead();
			}
		}

		for (int32 i = 0; i < PlayersCurrentlyInTheLead.Num(); i++)
		{
			if (!ShooterGameState->TopScoringPlayers.Contains(PlayersCurrentlyInTheLead[i]))
			{
				AShooterCharacter* Loser = Cast<AShooterCharacter>(PlayersCurrentlyInTheLead[i]->GetPawn());
				if (Loser)
				{
					Loser->MulticastLostTheLead();
				}
			}
		}
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDeaths(1);
	}

	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AShooterPlayerController* ShooterPlayer = Cast<AShooterPlayerController>(*It);
		if (ShooterPlayer && AttackerPlayerState && VictimPlayerState)
		{
			ShooterPlayer->BroadcastElim(AttackerPlayerState, VictimPlayerState);
		}
	}
}

void AShooterGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}

void AShooterGameMode::PlayerLeftGame(AShooterPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;
	AShooterGameState* ShooterGameState = GetGameState<AShooterGameState>();
	if (ShooterGameState && ShooterGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		ShooterGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	AShooterCharacter* CharacterLeaving = Cast<AShooterCharacter>(PlayerLeaving->GetPawn());
	if (CharacterLeaving)
	{
		CharacterLeaving->Elim(true);
	}
}
