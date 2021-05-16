// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ProjectShadowbreakerCharacter.generated.h"

UCLASS(config=Game)
class AProjectShadowbreakerCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	AProjectShadowbreakerCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Allows the Character to begin sprinting */
	void Sprint();

	/** Allows the character to stop sprinting */
	void StopSprinting();

	/** Zoom the camera in (Aim-Down-Sights) */
	void ZoomIn();

	/** Zoom the camera out (Blind-fire) */
	void ZoomOut();

	/** Allows the character to equip an item they are close to */
	void EquipItem();

	// Heal Functions //
	// Test Healing
	void StartHealing();
	
	// Heal Character
	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float _healAmount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void HealArmor(float _healAmount);

	// End Heal Functions //

	// Damage Functions //
	// Test Damage
	void StartDamage();
	
	// Damage Character
	UFUNCTION(BlueprintCallable, Category = "Health")
	void TakeDamage(float _damageAmount);

	// End Damage Functions //

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

	/** Determines if the character is overlapping an equippable item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items")
	bool bIsOverlappingItem;

	/** Determines when the character is sprinting */
	bool bIsSprinting;

	/** Determines if the character has armor */
	bool bHasArmor;

	/** Determines if the character is currently zoomed-in with their weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bIsZoomedIn;

	/** The character's current level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	int currentLevel;

	/** The amount of available upgrade points the character currently has */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	int upgradePoints;

	/** The amount of strength the character currently has */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	int strengthValue;

	/** The amount of dexterity the character currently has */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	int dexterityValue;

	/** The amount of intellect the character currently has */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	int intellectValue;

	/** The amount of health the character currently has */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float playerHealth;

	/** The amount of armor the character currently has */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float playerArmor;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

