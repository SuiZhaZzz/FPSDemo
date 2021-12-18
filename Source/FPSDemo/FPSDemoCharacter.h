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

	/**  Property replication */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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

public:
	/** Current socre */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Score")
	int32 Score;

	void IncrementScore(int32 Amount);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camp")
	bool bIsPlayer;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsSliding;

public:
	/** Property settings */
	/** The player's maximum health. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Health")
	float MaxHealth;

	/** The player's current health. When reduced to 0, they are considered dead.*/
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, VisibleAnywhere, Category = "Health")
	float CurrentHealth;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	bool bIsDied;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
	class UAnimMontage* DeathMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
	UAnimMontage* HitMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	UParticleSystem* HitParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundCue* HitSound;

	/** RepNotify for changes made to current health.*/
	UFUNCTION()
	void OnRep_CurrentHealth();

	/** Response to health being updated. Called on the server immediately after modification, and on clients in response to a RepNotify*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Health")
	void OnHealthUpdate();

	/** Getter for Max Health.*/
	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	/** Getter for Current Health.*/
	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }

	/** Setter for Current Health. Clamps the value between 0 and MaxHealth and calls OnHealthUpdate. Should only be called on the server.*/
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetCurrentHealth(float healthValue);

	/** Event for taking damage. Overridden from APawn.*/
	UFUNCTION(BlueprintCallable, Category = "Health")
	float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

public:
	/** Weapon settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<class ABaseWeapon> WeaponClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	ABaseWeapon* EquippedWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<class ABullet> BulletClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<class AGrenade> GrenadeClass;

	/** The health aid bag can heal. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	float HealValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 MaxGrenade;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 MaxClip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 MaxAid;

	/** Set replicated properties. 
	 * Increment operation only be used on the server.
	 */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 GrenadeAmount;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 ClipAmount;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 AidAmount;

	/** Pickups */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	class APickup* ActiveOverlappingPickup;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	UParticleSystem* PickParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	class USoundCue* PickSound;

	FORCEINLINE void SetActiveOverlappingPickup(APickup* Pickup) { ActiveOverlappingPickup = Pickup; }

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void OnPickup();

	/** Handle pickup, increment and decrement operation can only be called on the server.
	 * Use LocalRole to ensure this.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Pickup")
	void HandlePickup();

	/** Increment the clip amount and destroy the clip instance. */
	void IncrementClip(class AClip* Clip);
	void DecrementClip();

	void IncrementGrenade(class AGrenBag* GrenBag);
	void DecrementGrenade();

	void IncrementAidBag(class AAidBag* AidBag);
	void DecrementAidBag();
public:
	/** Fire settings */

	/** Delay between shots in seconds. Used to control fire rate for our test projectile, 
	 * but also to prevent an overflow of server functions from binding SpawnProjectile directly to input.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	float FireRate;
	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	float ThrowRate;
	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	float ReloadRate;
	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	float HealRate;

	/** If true, we are in the process of firing projectiles. */
	bool bIsFiringWeapon;
	bool bIsThrowingWeapon;
	bool bIsReloading;
	bool bIsHealing;

	/** The same montage for fire and throw. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
	class UAnimMontage* FireMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
	UAnimMontage* ReloadMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
	UAnimMontage* HealMontage;

	/** Sound for reload and heal.
	 * Fire and throw sound set in the `BeginPlay()` function of the base projectile class.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundCue* ReloadSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundCue* HealSound;

	/** A timer handle used for providing the fire rate delay in-between spawns.*/
	FTimerHandle FiringTimer;
	FTimerHandle ThrowingTimer;
	FTimerHandle ReloadTimer;
	FTimerHandle HealTimer;

	bool bLMBDown;

	/** Function for beginning weapon fire.*/
	UFUNCTION(BlueprintCallable, Category = "Fire")
	void OnFire();
	UFUNCTION(BlueprintCallable, Category = "Fire")
	void ReleaseFire();

	/** Function for ending weapon fire. Once this is called, the player can use StartFire again.*/
	UFUNCTION(BlueprintCallable, Category = "Fire")
	void StopFire();

	/** Server function for spawning projectiles.
	 * any attempt to call it on a client will result in 
	 * the call being directed over the network to the authoritative Character on the server instead.
	 */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Fire")
	void HandleFire();

	/** Set for throwing grenade. */
	UFUNCTION(BlueprintCallable, Category = "Fire")
	void ThrowGrenade();
	
	UFUNCTION(BlueprintCallable, Category = "Fire")
	void StopThrow();

	UFUNCTION(Server, Reliable, Category = "Fire")
	void HandleThrow();

	UFUNCTION(BlueprintCallable, Category = "Fire")
	void ReloadAmmo();

	UFUNCTION(BlueprintCallable, Category = "Fire")
	void StopReload();

	UFUNCTION(Server, Reliable, Category = "Fire")
	void HandleReload();

	UFUNCTION(BlueprintCallable, Category = "Fire")
	void Heal();

	UFUNCTION(BlueprintCallable, Category = "Fire")
	void StopHeal();

	UFUNCTION(Server, Reliable, Category = "Fire")
	void HandleHeal();

	UFUNCTION(BlueprintCallable, Category = "Fire")
	FORCEINLINE bool GetEquippedState() { return bIsReloading || bIsHealing; }

public:
	/** Utilities */

	UFUNCTION(BlueprintCallable, Category = "Particles")
	void PlayParticles(UParticleSystem* Particles);

	UFUNCTION(BlueprintCallable, Category = "Sound")
	void PlaySound(USoundCue* Sound);

	UFUNCTION(BlueprintCallable, Category = "Montage")
	void PlayMontage(UAnimMontage* Montage);

	/** Run on the owning client. */
	UFUNCTION(Client, Reliable, Category = "Particles")
	void PlayParticlesClient(UParticleSystem* Particles);
	
	UFUNCTION(Client, Reliable, Category = "Sound")
	void PlaySoundClient(USoundCue* Sound);

	UFUNCTION(Client, Reliable, Category = "Montage")
	void PlayMontageClient(UAnimMontage* Montage);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Montage")
	void PlayMontageCast(UAnimMontage* Montage);

	/** Finish game function.
	 * Set `BlueprintNativeEvent` to expect some implementations in the blueprint.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gameplay")
	void WinGame();

	/** Server call to run on the owing client to finish game. */
	UFUNCTION(Client, Reliable, Category = "Gameplay")
	void WinGameClient();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gameplay")
	void LoseGame();

	UFUNCTION(Client, Reliable, Category = "Gameplay")
	void LoseGameClient();
protected:

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	virtual void Jump() override;

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
};

