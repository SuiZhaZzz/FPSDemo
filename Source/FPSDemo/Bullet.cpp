// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet.h"
#include "Components/SphereComponent.h"
#include "BaseTarget.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"


ABullet::ABullet()
{
	// set up a notification for when this component hits something blocking
	CollisionComp->OnComponentHit.AddDynamic(this, &ABullet::OnHit);

	ImpulseScale = 100.f;
}

void ABullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if (OtherActor != nullptr && OtherActor != this && OtherComp != nullptr)
	{
		ABaseTarget* Target = Cast<ABaseTarget>(OtherActor);
		if (Target && !Target->bDied)
		{
			if (HitParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, GetActorLocation(), FRotator(0.f), true);
			}

			if (HitSound)
			{
				UGameplayStatics::PlaySound2D(this, HitSound);
			}

			OtherComp->SetSimulatePhysics(true);
			// Add an impulse
			OtherComp->AddImpulseAtLocation(GetVelocity() * ImpulseScale, GetActorLocation());

			Target->DestroyTarget(Character);
			Destroy();
		}
	}
}
