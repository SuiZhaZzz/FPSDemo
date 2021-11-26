// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseTarget.generated.h"

UCLASS()
class FPSDEMO_API ABaseTarget : public AActor
{
	GENERATED_BODY()
	
public:	
	ABaseTarget();

	/** Base mesh component */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh)
	class UStaticMeshComponent* Mesh;

	/** Different score for different target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Target)
	int32 Score;

	FTimerHandle DeathTimer;
	
	/** Die after death time once damaged */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Target)
	float DeathTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh)
	bool bDied;

	float RunningTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Target)
	float A;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Target)
	float B;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Target)
	float C;

	// If causer's score >= this score, spawn a boss
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Target)
	int32 BossScore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Target)
	TSubclassOf<class ABaseTarget> BossClass;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void DestroyTarget(class AFPSDemoCharacter* Causer);

	// Wrap destroy method
	void Disappear();

	UFUNCTION(BlueprintImplementableEvent)
	void EndGame();
};
