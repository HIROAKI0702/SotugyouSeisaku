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
#include "Gimmick_PushBlock.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

/// @brief コンストラクタ　プレイヤーの各種初期設定
ASotugyouSeisakuCharacter::ASotugyouSeisakuCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void ASotugyouSeisakuCharacter::BeginPlay()
{
	Super::BeginPlay();
	PrevLocation = GetActorLocation();
}

void ASotugyouSeisakuCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//押していない時だけブロックを検出
	if (!bIsPushing)
	{
		CheckForGimmick();
	}

	//プレイヤーがブロックを押している場合
	if (bIsPushing && mTargetBlock)
	{
		//プレイヤーの１フレームの移動量を取得
		FVector DeltaMove = GetActorLocation() - PrevLocation;
		DeltaMove.Z = 0.f;//高さは無視

		//ブロックに移動量を渡す
		if (!DeltaMove.IsNearlyZero(0.001f))
		{
			mTargetBlock->MoveWithPlayer(DeltaMove);
		}
	}

	//次フレームに向けて位置を更新
	PrevLocation = GetActorLocation();
}

/// @brief コントロ－ラーが変更された場合に呼ばれる
void ASotugyouSeisakuCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

/// @brief 入力コンポーネントの設定
/// @param PlayerInputComponent 
void ASotugyouSeisakuCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		//ジャンプ処理
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//移動処理
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASotugyouSeisakuCharacter::Move);

		//視点移動処理
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASotugyouSeisakuCharacter::Look);

		//ブロックを押す/終了
		EnhancedInputComponent->BindAction(mPushAction, ETriggerEvent::Triggered, this, &ASotugyouSeisakuCharacter::StartPush);
		EnhancedInputComponent->BindAction(mPushAction, ETriggerEvent::Completed, this, &ASotugyouSeisakuCharacter::StopPush);
	}
}

/// @brief 移動処理
/// @param Value キーなどで入力された値を受け取る変数
void ASotugyouSeisakuCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

/// @brief 視点移動処理
/// @param Value マウス入力された値を受け取る変数
void ASotugyouSeisakuCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

/// @brief ギミックを検出する関数
void ASotugyouSeisakuCharacter::CheckForGimmick()
{
	//プレイヤーの現在位置取得
	FVector Start = GetActorLocation();
	//プレイヤーの前方方向にmPushDistance 分だけ進んだ位置を計算
	FVector End = Start + GetActorForwardVector() * mPushDistance;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);//自分は判定から除外

	//Start から End までの間に何かオブジェクトがあるか調べる
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		//押せるオブジェクトならmTargetBlockに代入
		mTargetBlock = Cast<AGimmick_PushBlock>(Hit.GetActor());
	}
	else
	{
		mTargetBlock = nullptr;
	}
}

/// @brief ブロックを押し開始、Fキー押下中はプレイヤーとブロックを連動させる
void ASotugyouSeisakuCharacter::StartPush()
{
	if (mTargetBlock)
	{
		//押せる位置にいるかチェック
		if (!mTargetBlock->CanBePushedByPlayer(GetActorLocation()))
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot push from this side!"));
			return;
		}

		bIsPushing = true;
		mTargetBlock->StartPushing(this);

		//ブロックとのコリジョンを無効化
		if (UCapsuleComponent* Capsule = GetCapsuleComponent())
		{
			Capsule->IgnoreActorWhenMoving(mTargetBlock, true);
		}

		//押している間は移動速度を下げる（重い感じを出す）
		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			//PlayAnimMontage(PushAnimMontage);
			Movement->MaxWalkSpeed = 200.f;//通常500→200に減速
		}
	}
}

/// @brief ブロック押し終了、Fキーを離したときプレイヤーとブロックの連動を終了
void ASotugyouSeisakuCharacter::StopPush()
{
	bIsPushing = false;

	if (mTargetBlock)
	{
		//ブロックとのコリジョンを再有効化
		if (UCapsuleComponent* Capsule = GetCapsuleComponent())
		{
			Capsule->IgnoreActorWhenMoving(mTargetBlock, false);
		}

		//移動速度を元に戻す
		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			Movement->MaxWalkSpeed = 500.f;
		}

		mTargetBlock->StopPushing();
		mTargetBlock = nullptr;
	}
}