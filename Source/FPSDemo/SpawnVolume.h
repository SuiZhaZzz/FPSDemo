// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnVolume.generated.h"

UCLASS()
class FPSDEMO_API ASpawnVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawnVolume();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	class UBoxComponent* SpawningBox;

	/** Actors and Nums to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	TMap<TSubclassOf<AActor>, int32> SpawnArray;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	/** Get random spawn point in spawn box. */
	UFUNCTION(BlueprintPure, Category = "Spawning")
	FVector GetSpawnPoint();

	/** Run on server to spawn actors. */
	UFUNCTION(Server, Reliable, Category = "Spawning")
	void HandleSpawns(UClass* ToSpawn, int32 Num);
};
