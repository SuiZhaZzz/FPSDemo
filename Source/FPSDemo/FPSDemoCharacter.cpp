// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSDemoCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "BaseWeapon.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Bullet.h"
#include "Grenade.h"
#include "Animation/AnimInstance.h"

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

	Score = 0;
	GrenadeAmount = 5;
}

void AFPSDemoCharacter::BeginPlay()
{
	Super::BeginPlay();
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
	if (EquippedWeapon && EquippedWeapon->Ammo > 0)
	{
		if (FireParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FireParticles, GetActorLocation(), FRotator(0.f), true);
		}

		if (FireSound)
		{
			UGameplayStatics::PlaySound2D(this, FireSound);
		}

		// Play montage
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && FireMontage)
		{
			AnimInstance->Montage_Play(FireMontage, 1.0f);
		}

		const FRotator SpawnRotation = GetControlRotation();
		const FVector SpawnLocation = EquippedWeapon->Mesh->GetSocketLocation("Muzzle");

		if (BulletClass)
		{
			// Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;

			// Spawn the projectile at the ammo socket
			GetWorld()->SpawnActor<ABullet>(BulletClass, SpawnLocation, SpawnRotation, ActorSpawnParams);

			// Decrement ammo of equipped weapon
			EquippedWeapon->DecrementAmmo();

			if (EquippedWeapon->Ammo <= 0 && GrenadeAmount <= 0)
			{
				EndGame();
			}
		}
	}
}

void AFPSDemoCharacter::ThrowGrenade()
{

	if (GrenadeAmount > 0)
	{
		if (ThrowParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ThrowParticles, GetActorLocation(), FRotator(0.f), true);
		}

		if (ThrowSound)
		{
			UGameplayStatics::PlaySound2D(this, ThrowSound);
		}

		// Play montage
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && FireMontage)
		{
			AnimInstance->Montage_Play(FireMontage, 1.0f);
		}

		const FRotator SpawnRotation = GetControlRotation();
		const FVector SpawnLocation = EquippedWeapon->Mesh->GetSocketLocation("Muzzle");

		if (GrenadeClass)
		{
			// Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;

			// Spawn the projectile at the ammo socket
			GetWorld()->SpawnActor<AGrenade>(GrenadeClass, SpawnLocation, SpawnRotation, ActorSpawnParams);

			// Decrement ammo of grenade
			GrenadeAmount--;

			if (EquippedWeapon && EquippedWeapon->Ammo <= 0 && GrenadeAmount <= 0)
			{
				EndGame();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AFPSDemoCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
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

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AFPSDemoCharacter::OnResetVR);

	// Shift view mode
	PlayerInputComponent->BindAction("ShiftView", IE_Pressed, this, &AFPSDemoCharacter::ShiftViewMode);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSDemoCharacter::OnFire);
	PlayerInputComponent->BindAction("Grenade", IE_Pressed, this, &AFPSDemoCharacter::ThrowGrenade);
}

void AFPSDemoCharacter::OnResetVR()
{
	// If FPSDemo is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in FPSDemo.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AFPSDemoCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AFPSDemoCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AFPSDemoCharacter::ShiftViewMode()
{
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
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AFPSDemoCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AFPSDemoCharacter::MoveForward(float Value)
{
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
