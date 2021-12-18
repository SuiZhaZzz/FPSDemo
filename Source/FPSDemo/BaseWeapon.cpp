// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseWeapon.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
ABaseWeapon::ABaseWeapon()
{
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GetRootComponent());

	MaxAmmo = 25;
	Ammo = MaxAmmo;
}

// Called when the game starts or when spawned
void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseWeapon::DecrementAmmo()
{
	if (Ammo > 0) Ammo--;
}

void ABaseWeapon::ReloadAmmo()
{
	Ammo = MaxAmmo;
}