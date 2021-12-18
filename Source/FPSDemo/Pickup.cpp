// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "FPSDemoCharacter.h"

// Sets default values
APickup::APickup()
{
	// Set replicated
	bReplicates = true;

	CollisionVolume = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionVolume"));
	CollisionVolume->CanCharacterStepUpOn = ECB_No;
	RootComponent = CollisionVolume;

	OverlapVolume = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapVolume"));
	OverlapVolume->SetupAttachment(GetRootComponent());
	OverlapVolume->CanCharacterStepUpOn = ECB_No;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GetRootComponent());
	Mesh->CanCharacterStepUpOn = ECB_No;

}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();
	
	// Server-specific
	if (GetLocalRole() == ROLE_Authority)
	{
		OverlapVolume->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnOverlapBegin);
		OverlapVolume->OnComponentEndOverlap.AddDynamic(this, &APickup::OnOverlapEnd);
	}
}

void APickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AFPSDemoCharacter* Char = Cast<AFPSDemoCharacter>(OtherActor);
		if(Char)
		{
			Char->SetActiveOverlappingPickup(this);
		}
	}
}

void APickup::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AFPSDemoCharacter* Char = Cast<AFPSDemoCharacter>(OtherActor);
		if (Char)
		{
			Char->SetActiveOverlappingPickup(nullptr);
		}
	}
}



