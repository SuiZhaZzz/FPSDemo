// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "AidBag.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API AAidBag : public APickup
{
	GENERATED_BODY()
public:
	AAidBag();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float Health;
};
