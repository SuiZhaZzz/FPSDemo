// Fill out your copyright notice in the Description page of Project Settings.


#include "Grenade.h"
#include "BaseTarget.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

AGrenade::AGrenade()
{
	// Use a sphere as a simple collision representation
	OverlapCollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphereComp"));
	OverlapCollisionComp->InitSphereRadius(500.0f);

	// Players can't walk on it
	OverlapCollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	OverlapCollisionComp->CanCharacterStepUpOn = ECB_No;

	OverlapCollisionComp->SetupAttachment(GetRootComponent());

	TriggerTime = 3.f;
	ForceRadius = 500.f;
	ForceStrength = 2000.f;

	OverlapCollisionComp->InitSphereRadius(ForceRadius);

}

// Called when the game starts or when spawned
void AGrenade::BeginPlay()
{
	Super::BeginPlay();
	// Triggered after trigger time
	GetWorldTimerManager().SetTimer(TriggerTimer, this, &AGrenade::Trigger, TriggerTime);
}

void AGrenade::Trigger()
{
	if (HitParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, GetActorLocation(), FRotator(0.f), true);
	}

	if (HitSound)
	{
		UGameplayStatics::PlaySound2D(this, HitSound);
	}

	// Get overlapping actors by collision component
	TArray<AActor*> OverlapActors;
	// Filter target
	OverlapCollisionComp->GetOverlappingActors(OverlapActors, ABaseTarget::StaticClass());

	//UE_LOG(LogTemp, Warning, TEXT("OverlapActors: %d"), OverlapActors.Num());

	// Add radial force for each overlapping target
	for (AActor* Actor : OverlapActors)
	{
		ABaseTarget* Target = Cast<ABaseTarget>(Actor);
		if (Target && !Target->bDied)
		{
			UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(Target->GetRootComponent());

			if (MeshComp)
			{
				MeshComp->SetSimulatePhysics(true);
				// Use  ERadialImpulseFalloff::RIF_Linear for the impulse to get linearly weaker as it gets further from origin.
				// Set the float radius and the float strength.
				MeshComp->AddRadialImpulse(GetActorLocation(), ForceRadius, ForceStrength, ERadialImpulseFalloff::RIF_Constant, false);
			}

			Target->DestroyTarget(Character);
		}
	}
	Destroy();
}
