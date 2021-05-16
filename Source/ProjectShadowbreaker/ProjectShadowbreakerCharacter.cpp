// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectShadowbreakerCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

//////////////////////////////////////////////////////////////////////////
// AProjectShadowbreakerCharacter

AProjectShadowbreakerCharacter::AProjectShadowbreakerCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	/// <Player Variables> ///
	bIsOverlappingItem = false;
	bIsSprinting = false;
	bHasArmor = true;
	bIsZoomedIn = false;

	currentLevel = 1;
	upgradePoints = 5;

	strengthValue = 1;
	dexterityValue = 1;
	intellectValue = 1;
	
	playerHealth = 1.f;
	playerArmor = 1.f;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AProjectShadowbreakerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AProjectShadowbreakerCharacter::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AProjectShadowbreakerCharacter::StopSprinting);

	PlayerInputComponent->BindAction("Heal", IE_Pressed, this, &AProjectShadowbreakerCharacter::StartHealing);
	PlayerInputComponent->BindAction("Damage", IE_Pressed, this, &AProjectShadowbreakerCharacter::StartDamage);

	PlayerInputComponent->BindAction("EquipItem", IE_Pressed, this, &AProjectShadowbreakerCharacter::EquipItem);

	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &AProjectShadowbreakerCharacter::ZoomIn);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &AProjectShadowbreakerCharacter::ZoomOut);

	PlayerInputComponent->BindAxis("MoveForward", this, &AProjectShadowbreakerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AProjectShadowbreakerCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AProjectShadowbreakerCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AProjectShadowbreakerCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AProjectShadowbreakerCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AProjectShadowbreakerCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AProjectShadowbreakerCharacter::OnResetVR);
}


void AProjectShadowbreakerCharacter::OnResetVR()
{
	// If ProjectShadowbreaker is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in ProjectShadowbreaker.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AProjectShadowbreakerCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AProjectShadowbreakerCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AProjectShadowbreakerCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AProjectShadowbreakerCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AProjectShadowbreakerCharacter::MoveForward(float Value)
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

void AProjectShadowbreakerCharacter::MoveRight(float Value)
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

void AProjectShadowbreakerCharacter::Sprint()
{
	UE_LOG(LogTemp, Warning, TEXT("Sprinting!"));
	bIsSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = 1500.f;
}

void AProjectShadowbreakerCharacter::StopSprinting()
{
	UE_LOG(LogTemp, Warning, TEXT("Stopped Sprinting!"));
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
}

void AProjectShadowbreakerCharacter::StartHealing()
{
	Heal(0.02f);
}

void AProjectShadowbreakerCharacter::Heal(float _healAmount)
{
	UE_LOG(LogTemp, Warning, TEXT("We are healing for %f points"), _healAmount);
	playerHealth += _healAmount;
	
	if (playerHealth >= 1.f)
	{
		playerHealth = 1.f;
	}
}

void AProjectShadowbreakerCharacter::HealArmor(float _healAmount)
{
	UE_LOG(LogTemp, Warning, TEXT("We are healing armor for %f points"), _healAmount);
	playerArmor += _healAmount;
	bHasArmor = true;

	if (playerArmor >= 1.f)
	{
		playerArmor = 1.f;
	}
}


void AProjectShadowbreakerCharacter::StartDamage()
{
	TakeDamage(0.02f);
}

void AProjectShadowbreakerCharacter::TakeDamage(float _damageAmount)
{
	UE_LOG(LogTemp, Warning, TEXT("We are taking damage for %f points"), _damageAmount);

	if (bHasArmor)
	{
		playerArmor -= _damageAmount;
		if (playerArmor < 0)
		{
			bHasArmor = false;
			playerHealth += playerArmor;
			playerArmor = 0.f;
		}
	}
	else
	{
		playerHealth -= _damageAmount;

		if (playerHealth <= 0.f)
		{
			playerHealth = 0.f;
		}
	}
}

void AProjectShadowbreakerCharacter::EquipItem()
{
	UE_LOG(LogTemp, Warning, TEXT("Picked up an item"));
}

void AProjectShadowbreakerCharacter::ZoomIn()
{
	if (auto ThirdPersonCamera = GetCameraBoom())
	{
		UE_LOG(LogTemp, Warning, TEXT("We are now zooming in."));
		ThirdPersonCamera->TargetArmLength = 150.f;
		ThirdPersonCamera->TargetOffset = FVector(0.f, 80.f, 70.f);
		if (auto PlayerMovement = GetCharacterMovement())
		{
			PlayerMovement->MaxWalkSpeed = 300.f;
		}

		bIsZoomedIn = true;
	}
}

void AProjectShadowbreakerCharacter::ZoomOut()
{
	if (auto ThirdPersonCamera = GetCameraBoom())
	{
		UE_LOG(LogTemp, Warning, TEXT("We have stopped zooming in."));
		ThirdPersonCamera->TargetArmLength = 300.f;
		ThirdPersonCamera->TargetOffset = FVector(0.f, 0.f, 0.f);
		if (auto PlayerMovement = GetCharacterMovement())
		{
			PlayerMovement->MaxWalkSpeed = 600.f;
		}

		bIsZoomedIn = false;
	}
}
