// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet.h"
#include "Components/SphereComponent.h"
#include "BaseTarget.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "FPSDemoCharacter.h"

ABullet::ABullet()
{
	// Server-specific
	if (GetLocalRole() == ROLE_Authority)
	{
		// set up a notification for when this component hits something blocking
		CollisionComp->OnComponentHit.AddDynamic(this, &ABullet::OnHit);
	}

	ImpulseScale = 100.f;
	Damage = 10.f;
}

void ABullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if (OtherActor != nullptr && OtherActor != this && OtherComp != nullptr)
	{
		PlayParticlesReliable(HitParticles);
		PlaySoundReliable(HitSound);

		ABaseTarget* Target = Cast<ABaseTarget>(OtherActor);
		// Target-specific
		if (Target && !Target->bDied)
		{ 

			OtherComp->SetSimulatePhysics(true);
			// Add an impulse
			OtherComp->AddImpulseAtLocation(GetVelocity() * ImpulseScale, GetActorLocation());

			AActor* OwingActor = GetOwner();
			Target->DestroyTarget(OwingActor);

			Destroy();
		}
		
		// Character-specific
		AFPSDemoCharacter* OtherChar = Cast<AFPSDemoCharacter>(OtherActor);
		if (OtherChar)
		{
			UGameplayStatics::ApplyPointDamage(OtherChar, Damage, NormalImpulse, Hit, GetInstigator()->Controller, this, DamageType);
			Destroy();
		}

		Destroy();
	}
}
