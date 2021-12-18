// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseTarget.h"
#include "Components/StaticMeshComponent.h"
#include "FPSDemoCharacter.h"

// Sets default values
ABaseTarget::ABaseTarget()
{
	bReplicates = true;
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
	bIsRotating = true;
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
	if (bIsRotating)
	{
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

}

void ABaseTarget::DestroyTarget(AActor* Causer)
{
	if (Causer)
	{
		AFPSDemoCharacter* Char = Cast<AFPSDemoCharacter>(Causer);
		if (Char)
		{
			// Increment the causer's score
			Char->IncrementScore(Score);
		}
	}

	if (bIsRotating)
	{
		bDied = true;
		// Call destroy after death time
		GetWorldTimerManager().SetTimer(DeathTimer, this, &ABaseTarget::Disappear, DeathTime);
	}

}

void ABaseTarget::Disappear()
{
	// Implemented in blueprint
	EndGame();
	Destroy();
}