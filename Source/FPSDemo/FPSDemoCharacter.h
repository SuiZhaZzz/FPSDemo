// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSDemoCharacter.generated.h"

/** TPCamera setting */
USTRUCT(BlueprintType)
struct FTPCamSetting
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TPCameraSetting")
	float TargetArmLength;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TPCameraSetting")
	float TargetLagSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TPCameraSetting")
	FVector TargetSocketOffset;

public:
	FTPCamSetting() : TargetArmLength(0.f), TargetLagSpeed(0.f), TargetSocketOffset(FVector::ZeroVector) {}

	FTPCamSetting(float ArmLength, float LagSpeed, FVector SocketOffset) : TargetArmLength(ArmLength), TargetLagSpeed(LagSpeed), TargetSocketOffset(SocketOffset) {}
};

/** View mode */
UENUM(BlueprintType)
enum class EViewMode : uint8
{
	EVM_ThirdPerson			UMETA(DisplayName = "ThirdPerson"),
	EVM_FirstPerson			UMETA(DisplayName = "FirstPerson"),

	EMS_MAX				UMETA(DisplayName = "DefaultMAX")
};

UCLASS(config=Game)
class AFPSDemoCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Third person camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* TPBoom;

	/** First person camera boom */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* FPBoom;

	/** Third person follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TPCamera;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FPCamera;

public:
	AFPSDemoCharacter();

	virtual void BeginPlay() override;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Current view mode */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	EViewMode ViewMode;

	/** Current third person camera setting */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	FTPCamSetting CurCamSetting;

	/** Target third person camera setting */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	FTPCamSetting TarCamSetting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float TargetLagSpeed;

	FVector TPSocketOffset;

	/** Current socre */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Score")
	int32 Score;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<class ABaseWeapon> WeaponClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	ABaseWeapon* EquippedWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	class UParticleSystem* FireParticles;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	UParticleSystem* ThrowParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	class USoundCue* FireSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundCue* ThrowSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<class ABullet> BulletClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<class AGrenade> GrenadeClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 GrenadeAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
	class UAnimMontage* FireMontage;

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	/** Shift view mode */
	void ShiftViewMode();

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetTPBoom() const { return TPBoom; }
	FORCEINLINE class USpringArmComponent* GetFPBoom() const { return FPBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetTPCamera() const { return TPCamera; }
	FORCEINLINE class UCameraComponent* GetFPCamera() const { return FPCamera; }

	/** Apply target camer setting to third person camera */
	UFUNCTION(BlueprintCallable)
	void ApplyCamera(float LerpAlpha);

	/** Is third to first? */
	UFUNCTION(BlueprintImplementableEvent)
	void ApplyCameraEvent(bool bT2F);

	void IncrementScore(int32 Amount);

	UFUNCTION(BlueprintCallable)
	void OnFire();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenade();

	UFUNCTION(BlueprintImplementableEvent)
	void EndGame();
};

