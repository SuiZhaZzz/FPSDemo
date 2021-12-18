// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSDemoCharacter.h"
//#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "BaseWeapon.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Bullet.h"
#include "Grenade.h"
#include "Animation/AnimInstance.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Pickup.h"
#include "Clip.h"
#include "GrenBag.h"
#include "AidBag.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

//////////////////////////////////////////////////////////////////////////
// AFPSDemoCharacter

AFPSDemoCharacter::AFPSDemoCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	//bUseControllerRotationPitch = false;
	//bUseControllerRotationYaw = false;
	//bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	TPBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("TPBoom"));
	TPBoom->SetupAttachment(RootComponent);
	TPBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	TPBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	TPBoom->bEnableCameraLag = true;  // The camera has a delay

	// Create first person camera boom attached to head
	FPBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("FPBoom"));
	FPBoom->SetupAttachment(GetMesh(), FName("head"));
	FPBoom->TargetArmLength = 0.0f; // The camera follows at this distance behind the character	
	FPBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a third person follow camera
	TPCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TPCamera"));
	TPCamera->SetupAttachment(TPBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	TPCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create a first person follow camera
	FPCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FPCamera"));
	FPCamera->SetupAttachment(FPBoom, USpringArmComponent::SocketName);
	FPCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	ViewMode = EViewMode::EVM_ThirdPerson;
	TargetLagSpeed = 10.f;
	TPSocketOffset = FVector::ZeroVector;

	// Property settings
	Score = 0;
	MaxHealth = 100.f;
	CurrentHealth = MaxHealth;

	// Weapon settings
	MaxGrenade = 3;
	MaxClip = 3;
	MaxAid = 3;
	GrenadeAmount = 0;
	ClipAmount = 0;
	AidAmount = 0;
	HealValue = 50.f;

	// Initialize fire rates
	FireRate = 0.25f;
	ThrowRate = 1.f;
	ReloadRate = 1.f;
	HealRate = 1.f;
	bIsFiringWeapon = false;
	bIsThrowingWeapon = false;
	bIsReloading = false;
	bIsHealing = false;
	bLMBDown = false;

	// Pickup settings
	ActiveOverlappingPickup = nullptr;

	bIsDied = false;
	bIsPlayer = true;
	bIsSliding = false;
}

void AFPSDemoCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;

	// Save third person camera boom socket offset
	TPSocketOffset = TPBoom->SocketOffset;
	// Set Spawn Collision Handling Override
	FActorSpawnParameters ActorSpawnParams;

	// Spawn the weapon
	if (WeaponClass)
	{
		EquippedWeapon = GetWorld()->SpawnActor<ABaseWeapon>(WeaponClass, ActorSpawnParams);
		// Attach to socket
		const USkeletalMeshSocket* RightHandSocket = GetMesh()->GetSocketByName("RightHandSocket");
		if (RightHandSocket)
		{
			RightHandSocket->AttachActor(EquippedWeapon, GetMesh());
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Replicated Properties

void AFPSDemoCharacter::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//Replicate current health.
	DOREPLIFETIME(AFPSDemoCharacter, CurrentHealth);
	DOREPLIFETIME(AFPSDemoCharacter, GrenadeAmount);
	DOREPLIFETIME(AFPSDemoCharacter, ClipAmount);
	DOREPLIFETIME(AFPSDemoCharacter, AidAmount);
}

//////////////////////////////////////////////////////////////////////////
// Health
// TODO
void AFPSDemoCharacter::OnHealthUpdate_Implementation()
{
	// Client-specific functionality
	// Autonomous Proxy
	//if (IsLocallyControlled())
	//{
	//	FString healthMessage = FString::Printf(TEXT("You now have %f health remaining."), CurrentHealth);
	//	FString printMessage1 = FString::Printf(TEXT("Print by %s."), *GetFName().ToString());
	//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
	//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, printMessage1);

	//	if (CurrentHealth <= 0)
	//	{
	//		FString deathMessage = FString::Printf(TEXT("You have been killed."));
	//		FString printMessage2 = FString::Printf(TEXT("Print by %s."), *GetFName().ToString());
	//		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);
	//		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, printMessage2);
	//	}
	//}

	// Server-specific functionality
	// Authority
	if (GetLocalRole() == ROLE_Authority)
	{
		if (!bIsDied && CurrentHealth <= 0)
		{
			// Server-specific
			//LoseGame();
			if (GetLocalRole() == ROLE_Authority)
			{
				PlayMontageCast(DeathMontage);
				bIsDied = true;
			}
			// Client-specific
			LoseGameClient();

			if (!bIsPlayer) return;
			
			AGameStateBase* const GameState = GetWorld() != NULL ? GetWorld()->GetGameState<AGameStateBase>() : NULL;
			if (GameState)
			{
				TArray<APlayerState*> Players = GameState->PlayerArray;
				for (const APlayerState* Player : Players)
				{
					APawn* Pawn = Player->GetPawn();
					AFPSDemoCharacter* Char = Cast<AFPSDemoCharacter>(Pawn);
					if (Char && Char != this && Char->bIsPlayer)
					{
						//Char->WinGame();
						Char->WinGameClient();
					}
				}
			}
		}
	}
}

void AFPSDemoCharacter::OnRep_CurrentHealth()
{
	// Ensure that each client responds the same way to the new CurrentHealth value
	OnHealthUpdate();
}

void AFPSDemoCharacter::SetCurrentHealth(float healthValue)
{
	// Server functionality
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth = FMath::Clamp(healthValue, 0.f, MaxHealth);
		// Ensure server has calls to this function
		OnHealthUpdate();
	}
}

float AFPSDemoCharacter::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float damageApplied = CurrentHealth - DamageTaken;
	PlayParticlesClient(HitParticles);
	SetCurrentHealth(damageApplied);
	return damageApplied;
}

//////////////////////////////////////////////////////////////////////////
// Camera
void AFPSDemoCharacter::ApplyCamera(float LerpAlpha)
{
	// Use lerp to apply camera setting
	TPBoom->TargetArmLength = FMath::Lerp<float, float>(CurCamSetting.TargetArmLength, TarCamSetting.TargetArmLength, LerpAlpha);
	TPBoom->CameraLagSpeed = FMath::Lerp<float, float>(CurCamSetting.TargetLagSpeed, TarCamSetting.TargetLagSpeed, LerpAlpha);
	TPBoom->SocketOffset = FMath::Lerp<FVector, float>(CurCamSetting.TargetSocketOffset, TarCamSetting.TargetSocketOffset, LerpAlpha);
}

//////////////////////////////////////////////////////////////////////////
// Score
void AFPSDemoCharacter::IncrementScore(int32 Amount)
{
	Score += Amount;
}

//////////////////////////////////////////////////////////////////////////
// Fire
void AFPSDemoCharacter::OnFire()
{
	if (EquippedWeapon && EquippedWeapon->Ammo > 0 && BulletClass && !bIsFiringWeapon && !GetEquippedState() && !bIsSliding)
	{
		bLMBDown = true;
		bIsFiringWeapon = true;
		UWorld* World = GetWorld();
		World->GetTimerManager().SetTimer(FiringTimer, this, &AFPSDemoCharacter::StopFire, FireRate, false);

		// Play montage
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && FireMontage)
		{
			AnimInstance->Montage_Play(FireMontage, 1.0f);
		}
		
		// Decrement ammo of equipped weapon
		EquippedWeapon->DecrementAmmo();

		HandleFire();
	}
}

void AFPSDemoCharacter::StopFire()
{
	bIsFiringWeapon = false;
	if (bLMBDown)
	{
		OnFire();
	}
}

void AFPSDemoCharacter::ReleaseFire()
{
	bLMBDown = false;
}
 
void AFPSDemoCharacter::HandleFire_Implementation()
{
	const FRotator SpawnRotation = GetControlRotation();
	FVector SpawnLocation = EquippedWeapon->Mesh->GetSocketLocation("Muzzle") 
		+ (GetControlRotation().Vector() * 100.0f) + (GetActorUpVector() * 50.0f);
	if (!bIsPlayer)
	{
		SpawnLocation = EquippedWeapon->Mesh->GetSocketLocation("Muzzle");
	}
	// Set Spawn Collision Handling Override
	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.Instigator = GetInstigator();
	ActorSpawnParams.Owner = this;

	// Spawn the projectile at the ammo socket
	GetWorld()->SpawnActor<ABullet>(BulletClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
}

void AFPSDemoCharacter::ThrowGrenade()
{

	if (GrenadeClass && GrenadeAmount > 0 && !bIsFiringWeapon && !GetEquippedState() && !bIsSliding)
	{
		bIsThrowingWeapon = true;
		UWorld* World = GetWorld();
		World->GetTimerManager().SetTimer(ThrowingTimer, this, &AFPSDemoCharacter::StopThrow, ThrowRate, false);

		// Play montage
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && FireMontage)
		{
			AnimInstance->Montage_Play(FireMontage, 1.0f);
		}

		HandleThrow();	
	}
}

void AFPSDemoCharacter::StopThrow()
{
	bIsThrowingWeapon = false;
}

void AFPSDemoCharacter::HandleThrow_Implementation()
{
	const FRotator SpawnRotation = GetControlRotation();
	const FVector SpawnLocation = EquippedWeapon->Mesh->GetSocketLocation("Muzzle")
		+ (GetControlRotation().Vector() * 100.0f) + (GetActorUpVector() * 50.0f);

	// Set Spawn Collision Handling Override
	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.Instigator = GetInstigator();
	ActorSpawnParams.Owner = this;

	// Spawn the projectile at the ammo socket
	GetWorld()->SpawnActor<AGrenade>(GrenadeClass, SpawnLocation, SpawnRotation, ActorSpawnParams);

	// Decrement the replicated property GrenadeAmount.
	DecrementGrenade();
}

void AFPSDemoCharacter::ReloadAmmo()
{
	if (ClipAmount > 0 && !GetEquippedState() && !bIsReloading && EquippedWeapon && EquippedWeapon->Ammo < EquippedWeapon->MaxAmmo && !bIsSliding)
	{
		bIsReloading = true;
		PlayMontage(ReloadMontage);
		PlaySound(ReloadSound);

		// Run on server to decrement clip amount.
		HandleReload();

		// Ammo is local property, and is not necessary to be replicated.
		EquippedWeapon->ReloadAmmo();
		
		// Stop in montage notify
		//GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &AFPSDemoCharacter::StopReload, ReloadRate, false);
	}
}

void AFPSDemoCharacter::StopReload()
{
	bIsReloading = false;
}

// Server-RPCs
void AFPSDemoCharacter::HandleReload_Implementation()
{
	// Decrement replicated property `ClipAmount` on the server.
	DecrementClip();
}

void AFPSDemoCharacter::Heal()
{
	if (AidAmount > 0 && !GetEquippedState() && !bIsHealing && CurrentHealth < MaxHealth && !bIsSliding)
	{
		bIsHealing = true;
		PlayMontage(HealMontage);
		PlaySound(HealSound);
		// Run on the server to decrement aid amount and increment health
		HandleHeal();

		// Stop in montage notify
		//GetWorld()->GetTimerManager().SetTimer(HealTimer, this, &AFPSDemoCharacter::StopHeal, HealRate, false);
	}
}

void AFPSDemoCharacter::StopHeal()
{
	bIsHealing = false;
}

void AFPSDemoCharacter::HandleHeal_Implementation()
{
	// Decrement replicated property `AidAmount` on the server.
	DecrementAidBag();
	// Set new health after heal.
	float HealthApplied = CurrentHealth + HealValue;
	SetCurrentHealth(HealthApplied);
}

//////////////////////////////////////////////////////////////////////////
// Pickup
void AFPSDemoCharacter::OnPickup()
{

	HandlePickup();
}

void AFPSDemoCharacter::HandlePickup_Implementation()
//void AFPSDemoCharacter::HandlePickup()
{
	if (ActiveOverlappingPickup)
	{
		AClip* Clip = Cast<AClip>(ActiveOverlappingPickup);
		if (Clip)
		{
			if (ClipAmount < MaxClip)
			{
				PlayParticlesClient(PickParticles);
				PlaySoundClient(PickSound);
				IncrementClip(Clip);
			}
			return;
		}
		AGrenBag* GrenBag = Cast<AGrenBag>(ActiveOverlappingPickup);
		if (GrenBag)
		{
			if (GrenadeAmount < MaxGrenade)
			{
				PlayParticlesClient(PickParticles);
				PlaySoundClient(PickSound);
				IncrementGrenade(GrenBag);
			}
			return;
		}
		AAidBag* AidBag = Cast<AAidBag>(ActiveOverlappingPickup);
		if (AidBag)
		{
			if (AidAmount < MaxAid)
			{
				PlayParticlesClient(PickParticles);
				PlaySoundClient(PickSound);
				IncrementAidBag(AidBag);
			}
			return;
		}
	}
}

//void AFPSDemoCharacter::IncrementClip_Implementation(AClip* Clip)
void AFPSDemoCharacter::IncrementClip(AClip* Clip)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		ClipAmount++;
		Clip->Destroy();
		SetActiveOverlappingPickup(nullptr);
	}

}

//void AFPSDemoCharacter::DecrementClip_Implementation()
void AFPSDemoCharacter::DecrementClip()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (ClipAmount > 0)
			ClipAmount--;
	}
}

//void AFPSDemoCharacter::IncrementGrenade_Implementation(AGrenBag* GrenBag)
void AFPSDemoCharacter::IncrementGrenade(AGrenBag* GrenBag)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		GrenadeAmount++;
		GrenBag->Destroy();
		SetActiveOverlappingPickup(nullptr);
	}
}

//void AFPSDemoCharacter::DecrementGrenade_Implementation()
void AFPSDemoCharacter::DecrementGrenade()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (GrenadeAmount > 0)
			GrenadeAmount--;
	}
}

//void AFPSDemoCharacter::IncrementAidBag_Implementation(AAidBag* AidBag)
void AFPSDemoCharacter::IncrementAidBag(AAidBag* AidBag)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		AidAmount++;
		AidBag->Destroy();
		SetActiveOverlappingPickup(nullptr);
	}
}

//void AFPSDemoCharacter::DecrementAidBag_Implementation()
void AFPSDemoCharacter::DecrementAidBag()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (AidAmount > 0)
			AidAmount--;
	}
}

//////////////////////////////////////////////////////////////////////////
// Utilities
void AFPSDemoCharacter::PlayParticles(UParticleSystem* Particles)
{
	if (Particles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Particles, GetActorLocation(), FRotator(0.f), true);
	}
}

void AFPSDemoCharacter::PlaySound(USoundCue* Sound)
{
	if (Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, GetActorLocation());
	}
}

void AFPSDemoCharacter::PlayMontage(UAnimMontage* Montage)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage, 1.0f);
	}
}

void AFPSDemoCharacter::PlayParticlesClient_Implementation(UParticleSystem* Particles)
{
	PlayParticles(Particles);
}

void AFPSDemoCharacter::PlaySoundClient_Implementation(USoundCue* Sound)
{
	PlaySound(Sound);
}

void AFPSDemoCharacter::PlayMontageClient_Implementation(UAnimMontage* Montage)
{
	PlayMontage(Montage);
}

void AFPSDemoCharacter::PlayMontageCast_Implementation(UAnimMontage* Montage)
{
	PlayMontage(Montage);
}

void AFPSDemoCharacter::WinGame_Implementation()
{

}

void AFPSDemoCharacter::LoseGame_Implementation()
{

}

void AFPSDemoCharacter::WinGameClient_Implementation()
{
	WinGame();
}

void AFPSDemoCharacter::LoseGameClient_Implementation()
{
	LoseGame();
}

//////////////////////////////////////////////////////////////////////////
// Input

void AFPSDemoCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AFPSDemoCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AFPSDemoCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPSDemoCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AFPSDemoCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AFPSDemoCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AFPSDemoCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AFPSDemoCharacter::TouchStopped);

	// Shift view mode
	PlayerInputComponent->BindAction("ShiftView", IE_Pressed, this, &AFPSDemoCharacter::ShiftViewMode);

	PlayerInputComponent->BindAction("Pick", IE_Pressed, this, &AFPSDemoCharacter::OnPickup);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSDemoCharacter::OnFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AFPSDemoCharacter::ReleaseFire);
	PlayerInputComponent->BindAction("Grenade", IE_Pressed, this, &AFPSDemoCharacter::ThrowGrenade);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AFPSDemoCharacter::ReloadAmmo);
	PlayerInputComponent->BindAction("Heal", IE_Pressed, this, &AFPSDemoCharacter::Heal);


}

void AFPSDemoCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	if (bIsDied || bIsSliding) return;
	Jump();
}

void AFPSDemoCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	if (bIsDied || bIsSliding) return;
	StopJumping();
}

void AFPSDemoCharacter::ShiftViewMode()
{
	if (bIsDied || bIsSliding) return;
	// Store current third person camera setting
	CurCamSetting = FTPCamSetting(TPBoom->TargetArmLength, TPBoom->CameraLagSpeed, TPBoom->SocketOffset);

	ViewMode = (ViewMode == EViewMode::EVM_ThirdPerson ? EViewMode::EVM_FirstPerson : EViewMode::EVM_ThirdPerson);
	// Set target third person camera setting
	switch (ViewMode)
	{
		case EViewMode::EVM_ThirdPerson:
			TarCamSetting = FTPCamSetting(TPBoom->TargetArmLength, TargetLagSpeed, TPSocketOffset);
			// This event implemented in blueprint using timeline
			ApplyCameraEvent(false);
			break;
		case EViewMode::EVM_FirstPerson:
			TarCamSetting = FTPCamSetting(TPBoom->TargetArmLength, TargetLagSpeed,
				// Arm length + FP root location - TP root location
				FVector(TPBoom->TargetArmLength - FPBoom->TargetArmLength, 0.f, 0.f)
				+ FPBoom->GetComponentLocation() + FPBoom->GetRelativeLocation() + FPBoom->GetRelativeLocation()
				- (TPBoom->GetComponentLocation() + TPCamera->GetRelativeLocation() + TPCamera->GetRelativeLocation()));
			ApplyCameraEvent(true);
			break;
		default:
			;
	}
}

void AFPSDemoCharacter::TurnAtRate(float Rate)
{
	if (bIsDied || bIsSliding) return;
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AFPSDemoCharacter::LookUpAtRate(float Rate)
{
	if (bIsDied || bIsSliding) return;
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AFPSDemoCharacter::MoveForward(float Value)
{
	if (bIsDied || bIsSliding) return;
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AFPSDemoCharacter::MoveRight(float Value)
{
	if (bIsDied || bIsSliding) return;
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AFPSDemoCharacter::Jump()
{
	if (bIsDied || bIsSliding) return;
	Super::Jump();
}