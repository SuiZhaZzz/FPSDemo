// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnVolume.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

// Sets default values
ASpawnVolume::ASpawnVolume()
{
	bReplicates = true;

	SpawningBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawningBox"));
}

// Called when the game starts or when spawned
void ASpawnVolume::BeginPlay()
{
	Super::BeginPlay();
	
	for (auto Actor : SpawnArray)
	{
		HandleSpawns(Actor.Key, Actor.Value);
	}
}

FVector ASpawnVolume::GetSpawnPoint()
{
	FVector Extent = SpawningBox->GetScaledBoxExtent();
	FVector Origin = SpawningBox->GetComponentLocation();

	FVector Point = UKismetMathLibrary::RandomPointInBoundingBox(Origin, Extent);
	
	return Point;
}

void ASpawnVolume::HandleSpawns_Implementation(UClass* ToSpawn, int32 Num)
{
	if (ToSpawn)
	{
		// Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.Instigator = GetInstigator();
		ActorSpawnParams.Owner = this;

		for (int i = 0; i < Num; ++i)
		{
			GetWorld()->SpawnActor<AActor>(ToSpawn, GetSpawnPoint(), FRotator(0.f), ActorSpawnParams);
		}
	}
}