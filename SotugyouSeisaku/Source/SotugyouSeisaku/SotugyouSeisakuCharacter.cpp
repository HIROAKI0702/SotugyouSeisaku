// Copyright Epic Games, Inc. All Rights Reserved.

#include "SotugyouSeisakuCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ASotugyouSeisakuCharacter

ASotugyouSeisakuCharacter::ASotugyouSeisakuCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASotugyouSeisakuCharacter::BeginPlay()
{
	Super::BeginPlay();
	PrevLocation = GetActorLocation();
}

void ASotugyouSeisakuCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckForGimmick();

	//プレイヤーとブロックの動きを連動させる
	if (bIsPushing && mTargetBlock)
	{
		//プレイヤーの１フレームの移動量を取得
		FVector DeltaMove = GetActorLocation() - PrevLocation;
		DeltaMove.Z = 0.f;

		//ブロックに移動量を渡す
		if (!DeltaMove.IsNearlyZero())//移動量が微小なら無視
		{
			UE_LOG(LogTemp, Warning, TEXT("DeltaMove: %s"), *DeltaMove.ToString());
			mTargetBlock->MoveWithPlayer(DeltaMove);
		}
	}

	PrevLocation = GetActorLocation();
}

void ASotugyouSeisakuCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ASotugyouSeisakuCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASotugyouSeisakuCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASotugyouSeisakuCharacter::Look);

		//ブロックを押す/終了
		EnhancedInputComponent->BindAction(mPushAction, ETriggerEvent::Triggered, this, &ASotugyouSeisakuCharacter::StartPush);
		EnhancedInputComponent->BindAction(mPushAction, ETriggerEvent::Completed, this, &ASotugyouSeisakuCharacter::StopPush);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ASotugyouSeisakuCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ASotugyouSeisakuCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

/// @brief ギミックを検出する関数
void ASotugyouSeisakuCharacter::CheckForGimmick()
{
	//カメラ or プレイヤーの前方にレイを飛ばして判定
	FVector Start = GetActorLocation();
	FVector End = Start + GetActorForwardVector() * mPushDistance;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		mTargetBlock = Cast<AGimmick_PushBlock>(Hit.GetActor());
		//UE_LOG(LogTemp, Error, TEXT("Target Block Detected: %s"), *GetNameSafe(mTargetBlock));
	}
	else
	{
		mTargetBlock = nullptr;
	}
}

/// @brief Fキーを押してブロックを押す関数
void ASotugyouSeisakuCharacter::StartPush()
{
	if (mTargetBlock)
	{
		bIsPushing = true;
		mTargetBlock->StartPushing(this);

		//押すアニメーションを再生（例）
		//PlayAnimMontage(PushAnimMontage);
	}
}

/// @brief ブロックを押すのをやめる関数
void ASotugyouSeisakuCharacter::StopPush()
{
	bIsPushing = false;

	if (mTargetBlock)
	{
		mTargetBlock->StopPushing();
	}
}
