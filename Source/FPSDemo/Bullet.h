// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseProjectile.h"
#include "Bullet.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API ABullet : public ABaseProjectile
{
	GENERATED_BODY()

public:
	ABullet();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impulse")
	float ImpulseScale;

	/** called when projectile hits something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
