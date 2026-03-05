#include "Variant_Shooter/AI/ShooterAIController.h"
#include "Final_2026.h"
#include "ShooterNPC.h"
#include "Components/StateTreeAIComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "AI/Navigation/PathFollowingAgentInterface.h"
#include "GameFramework/Pawn.h"


AShooterAIController::AShooterAIController()
{
	// create the StateTree component
	StateTreeAI = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));

	// create the AI perception component. It will be configured in BP
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

	// subscribe to the AI perception delegates
	AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AShooterAIController::OnPerceptionUpdated);
	AIPerception->OnTargetPerceptionForgotten.AddDynamic(this, &AShooterAIController::OnPerceptionForgotten);

	UE_LOG(LogFinal_2026, Log, TEXT("ShooterAIController constructed"));
}

void AShooterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	UE_LOG(LogFinal_2026, Log, TEXT("ShooterAIController possessing pawn: %s"), *InPawn->GetName());
	// ensure we're possessing an NPC
	if (AShooterNPC* NPC = Cast<AShooterNPC>(InPawn))
	{
		// add the team tag to the pawn
		NPC->Tags.Add(TeamTag);

		// subscribe to the pawn's OnDeath delegate
		NPC->OnPawnDeath.AddDynamic(this, &AShooterAIController::OnPawnDeath);
	}

	UE_LOG(LogFinal_2026, Log, TEXT("ShooterAIController OnPossess - StateTreeAI valid: %s"), IsValid(StateTreeAI) ? TEXT("true") : TEXT("false"));
	// Ensure StateTree logic is active for dynamically spawned pawns as well.
	if (IsValid(StateTreeAI))
	{
		StateTreeAI->RestartLogic();
	}
}

void AShooterAIController::OnPawnDeath()
{
	// stop movement
	GetPathFollowingComponent()->AbortMove(*this, FPathFollowingResultFlags::UserAbort);

	// stop StateTree logic
	StateTreeAI->StopLogic(FString(""));

	// unpossess the pawn
	UnPossess();

	// destroy this controller
	Destroy();
}

void AShooterAIController::SetCurrentTarget(AActor* Target)
{
	TargetEnemy = Target;
}

void AShooterAIController::ClearCurrentTarget()
{
	TargetEnemy = nullptr;
}

void AShooterAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	UE_LOG(LogFinal_2026, Log,
	       TEXT("AI PerceptionUpdated Controller=%s Actor=%s Sensed=%s Strength=%.2f Tag=%s DelegateBound=%s"),
	       *GetName(),
	       *GetNameSafe(Actor),
	       Stimulus.WasSuccessfullySensed() ? TEXT("true") : TEXT("false"),
	       Stimulus.Strength,
	       *Stimulus.Tag.ToString(),
	       OnShooterPerceptionUpdated.IsBound() ? TEXT("true") : TEXT("false"));

	// pass the data to the StateTree delegate hook
	OnShooterPerceptionUpdated.ExecuteIfBound(Actor, Stimulus);

	// Fallback: directly drive shooting from perception when StateTree target bindings are misconfigured.
	AShooterNPC* ShooterNPC = Cast<AShooterNPC>(GetPawn());
	APawn* SensedPawn = Cast<APawn>(Actor);
	const bool bIsPlayerTarget = IsValid(SensedPawn) ? SensedPawn->IsPlayerControlled() : (IsValid(Actor) && Actor->ActorHasTag(FName("Player")));

	if (!IsValid(ShooterNPC) || !bIsPlayerTarget || !IsValid(Actor))
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		SetCurrentTarget(Actor);
		ShooterNPC->StartShooting(Actor);
		UE_LOG(LogFinal_2026, Log, TEXT("DirectShootFallback start. Controller=%s Shooter=%s Target=%s"),
		       *GetName(), *ShooterNPC->GetName(), *Actor->GetName());
	}
	else if (GetCurrentTarget() == Actor)
	{
		ShooterNPC->StopShooting();
		ClearCurrentTarget();
		UE_LOG(LogFinal_2026, Log, TEXT("DirectShootFallback stop. Controller=%s Shooter=%s Target=%s"),
		       *GetName(), *ShooterNPC->GetName(), *Actor->GetName());
	}
}

void AShooterAIController::OnPerceptionForgotten(AActor* Actor)
{
	UE_LOG(LogFinal_2026, Log, TEXT("AI PerceptionForgotten Controller=%s Actor=%s DelegateBound=%s"),
	       *GetName(),
	       *GetNameSafe(Actor),
	       OnShooterPerceptionForgotten.IsBound() ? TEXT("true") : TEXT("false"));

	// pass the data to the StateTree delegate hook
	OnShooterPerceptionForgotten.ExecuteIfBound(Actor);

	// Fallback: ensure shooting stops if the current target is forgotten.
	if (GetCurrentTarget() == Actor)
	{
		if (AShooterNPC* ShooterNPC = Cast<AShooterNPC>(GetPawn()))
		{
			ShooterNPC->StopShooting();
			UE_LOG(LogFinal_2026, Log, TEXT("DirectShootFallback forgot target. Controller=%s Shooter=%s Target=%s"),
			       *GetName(), *ShooterNPC->GetName(), *GetNameSafe(Actor));
		}

		ClearCurrentTarget();
	}
}
