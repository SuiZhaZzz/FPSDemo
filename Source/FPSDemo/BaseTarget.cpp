// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseTarget.h"
#include "Components/StaticMeshComponent.h"
#include "FPSDemoCharacter.h"

// Sets default values
ABaseTarget::ABaseTarget()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GetRootComponent());

	Score = 1;
	DeathTime = 1;
	bDied = false;
	RunningTime = 0.f;

	A = 1.f;
	B = 1.f;
	C = 0.f;

	BossScore = 5;
}

// Called when the game starts or when spawned
void ABaseTarget::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ABaseTarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Set rotation
	FRotator Rotation = GetActorRotation();
	Rotation.Yaw += DeltaTime * 45.f;
	SetActorRotation(Rotation);

	// Set location
	FVector NewLoacation = GetActorLocation();
	if (RunningTime >= 2 * PI)
	{
		RunningTime = 0;
	}
	float DeltaHeight = A * FMath::Sin(B * RunningTime + C);
	RunningTime += DeltaTime;

	NewLoacation.Z += DeltaHeight /** 20.f*/;
	SetActorLocation(NewLoacation);
}

void ABaseTarget::DestroyTarget(AFPSDemoCharacter* Causer)
{
	bDied = true;

	// Increment the causer's score
	Causer->IncrementScore(Score);

	// Call destroy after death time
	GetWorldTimerManager().SetTimer(DeathTimer, this, &ABaseTarget::Disappear, DeathTime);
	
	if (Causer->Score >= BossScore)
	{
		// Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;

		// Spawn the boss
		if (BossClass)
		{
			GetWorld()->SpawnActor<ABaseTarget>(BossClass, GetActorLocation() + FVector(10.f, 0.f, 0.f), GetActorRotation(), ActorSpawnParams);
		}
	}

}

void ABaseTarget::Disappear()
{
	// Implemented in blueprint
	EndGame();
	Destroy();
}