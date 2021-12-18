
// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "FPSDemoCharacter.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "BaseWeapon.h"

// Sets default values
ABaseProjectile::ABaseProjectile()
{
	bReplicates = true;
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	//CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GetRootComponent());

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	LifeSpan = 3.0f;
	InitialLifeSpan = LifeSpan;

	DamageType = UDamageType::StaticClass();
	Damage = 10.0f;
}

// Called when the game starts or when spawned
void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
	AActor* Actor = GetOwner();
	if (Actor)
	{
		AFPSDemoCharacter* Char = Cast<AFPSDemoCharacter>(Actor);
		if (Char && Char->EquippedWeapon)
		{
			FVector Location = Char->EquippedWeapon->Mesh->GetSocketLocation("Muzzle");
			if (FireParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FireParticles, Location, FRotator(0.f), true);
			}

			if (FireSound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, Location);
			}
		}
	}
}

// Called every frame
void ABaseProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}



void ABaseProjectile::Destroyed()
{
//	if (HitParticles)
//	{
//		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, GetActorLocation(), FRotator(0.f), true);
//	}
//
//	if (HitSound)
//	{
//		//UGameplayStatics::PlaySound2D(this, HitSound);
//		UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, GetActorLocation());
//	}
}

void ABaseProjectile::PlayParticles(UParticleSystem* Particles)
{
	if (Particles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Particles, GetActorLocation(), FRotator(0.f), true);
	}
}

void ABaseProjectile::PlaySound(USoundCue* Sound)
{
	if (Sound)
	{
		//UGameplayStatics::PlaySound2D(this, HitSound);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, GetActorLocation());
	}
}

void ABaseProjectile::PlayParticlesReliable_Implementation(UParticleSystem* Particles)
{
	PlayParticles(Particles);
}

void ABaseProjectile::PlayParticlesUnreliable_Implementation(UParticleSystem* Particles)
{
	PlayParticles(Particles);
}

void ABaseProjectile::PlaySoundReliable_Implementation(USoundCue* Sound)
{
	PlaySound(Sound);
}

void ABaseProjectile::PlaySoundUnreliable_Implementation(USoundCue* Sound)
{
	PlaySound(Sound);
}