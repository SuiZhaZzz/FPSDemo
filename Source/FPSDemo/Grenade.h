// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseProjectile.h"
#include "Grenade.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API AGrenade : public ABaseProjectile
{
	GENERATED_BODY()

public:
	AGrenade();

	/** Sphere collision component */
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	class USphereComponent* OverlapCollisionComp;

	/** Trigger time after spawned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	float TriggerTime;
	/** Radial force radius, also the overlap collision component radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	float ForceRadius;
	/** Radial force strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	float ForceStrength;

	FTimerHandle TriggerTimer;

	UFUNCTION(BlueprintCallable)
	void Trigger();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
