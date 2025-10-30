// Copyright Epic Games, Inc. All Rights Reserved.

#include "SotugyouSeisakuCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"//カメラコンポーネント
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"//スプリングアームコンポーネント
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"//入力システム
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Interactable.h"
#include "InteractWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Gimmick_PushBlock.h"
#include "Gimmick_PlayerSwitch.h"
#include "Gimmick_FallFloorManager.h"

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

	AutoPossessAI = EAutoPossessAI::Disabled;

	mPushDistance = 150.0f;
	mRespawnZ = -3000.0f;
}

void ASotugyouSeisakuCharacter::BeginPlay()
{
	Super::BeginPlay();

	//インタラクトUIを作成
	if (mInteractWidgetClass)
	{
		mInteractWidget = CreateWidget<UInteractWidget>(GetWorld(), mInteractWidgetClass);
		if (mInteractWidget)
		{
			mInteractWidget->AddToViewport(10);
			mInteractWidget->HideWidget();
		}
	}

	//定期的にインタラクト可能なオブジェクトをチェック
	GetWorldTimerManager().SetTimer(
		mInteractCheckTimerHandle,
		this,
		&ASotugyouSeisakuCharacter::CheckForInteractables,
		mInteractCheckInterval,
		true
	);

	//初期位置を保存
	PrevLocation = GetActorLocation();
	PrevRotation = GetActorRotation();

	//レベル内の PlayerStart を検索
	TArray<AActor*> Starts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), Starts);

	if (Starts.Num() > 0)
	{
		mPlayerStart = Starts[0]; //最初の PlayerStart を使用
	}

	// FallFloorManagerを検索
	TArray<AActor*> FoundManagers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGimmick_FallFloorManager::StaticClass(), FoundManagers);
	if (FoundManagers.Num() > 0)
	{
		mFloorManager = Cast<AGimmick_FallFloorManager>(FoundManagers[0]);
	}
}

void ASotugyouSeisakuCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//プレイヤーの位置が指定Z座標より下に行けば復活
	if (GetActorLocation().Z < mRespawnZ)
	{
		RespawnPlayer();
	}

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

		//プレイヤー中心での回転
		float DeltaYaw = GetActorRotation().Yaw - PrevRotation.Yaw;
		if (!FMath::IsNearlyZero(DeltaYaw, 0.01f))
		{
			mTargetBlock->RotateAroundPlayer(GetActorLocation(), DeltaYaw);
		}
	}

	//次フレームに向けて位置を更新
	PrevLocation = GetActorLocation();
	PrevRotation = GetActorRotation();
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

		EnhancedInputComponent->BindAction(mPushAction, ETriggerEvent::Triggered, this, &ASotugyouSeisakuCharacter::TryInteract);

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

/// @brief インタラクト可能なオブジェクトを捜索
void ASotugyouSeisakuCharacter::CheckForInteractables()
{
	//キャラクターの前方をチェック
	FVector Start = GetActorLocation();
	FVector ForwardVector = GetActorForwardVector();
	FVector End = Start + (ForwardVector * mInteractCheckDistance);

	//球体トレース用のパラメータ
	FCollisionShape Sphere = FCollisionShape::MakeSphere(mInteractCheckRadius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	TArray<FHitResult> HitResults;
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		Start,
		End,
		FQuat::Identity,
		ECC_Visibility,
		Sphere,
		QueryParams
	);

	//最も近いインタラクト可能なオブジェクトを探す
	AActor* ClosestInteractable = nullptr;
	float ClosestDistance = mInteractCheckDistance;

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && HitActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
			{
				IInteractable* Interactable = Cast<IInteractable>(HitActor);
				if (Interactable && Interactable->Execute_CanInteract(HitActor, this))
				{
					float Distance = FVector::Dist(GetActorLocation(), HitActor->GetActorLocation());
					if (Distance < ClosestDistance)
					{
						ClosestDistance = Distance;
						ClosestInteractable = HitActor;
					}
				}
			}
		}
	}

	//現在のインタラクト可能オブジェクトを更新
	if (ClosestInteractable)
	{
		mCurrentInteractable.SetObject(ClosestInteractable);
		mCurrentInteractable.SetInterface(Cast<IInteractable>(ClosestInteractable));
	}
	else
	{
		mCurrentInteractable.SetObject(nullptr);
		mCurrentInteractable.SetInterface(nullptr);
	}

	//UIを更新
	UpdateInteractUI();
}

/// @brief UIをアップデートする関数
void ASotugyouSeisakuCharacter::UpdateInteractUI()
{
	if (!mInteractWidget)
	{
		return;
	}

	//コントローラーが有効かチェック（切り替え中は無視）
	if (!Controller || !Controller->IsLocalPlayerController())
	{
		mInteractWidget->HideWidget();
		return;
	}

	if (mCurrentInteractable.GetObject())
	{
		IInteractable* Interactable = mCurrentInteractable.GetInterface();
		if (Interactable)
		{
			AActor* InteractableActor = Cast<AActor>(mCurrentInteractable.GetObject());
			FText InteractText = Interactable->Execute_GetInteractText(InteractableActor);
			mInteractWidget->SetInteractText(InteractText);
			mInteractWidget->ShowWidget();
		}
	}
	else
	{
		mInteractWidget->HideWidget();
	}
}

/// @brief リスポーン関数
void ASotugyouSeisakuCharacter::RespawnPlayer()
{
	if (mPlayerStart)
	{
		//位置と向きをリセット
		SetActorLocation(mPlayerStart->GetActorLocation());
		SetActorRotation(mPlayerStart->GetActorRotation());

		// 落下速度をリセット（物理挙動がある場合）
		UCharacterMovementComponent* MoveComp = GetCharacterMovement();
		if (MoveComp)
		{
			MoveComp->Velocity = FVector::ZeroVector;
		}

		//すべての床を再生成
		if (mFloorManager)
		{
			mFloorManager->RespawnAllFloors();
		}
	}
}

/// @brief 切り替えポイントにインタラクトする関数
void ASotugyouSeisakuCharacter::TryInteract()
{
	//近くに切り替えポイントがあればインタラクト
	if (mNearbySwitchPoint)
	{
		mNearbySwitchPoint->OnInteract(this);
	}
}

/// @brief 現在のインタラクト用ウィジェットを取得する関数
/// @return 現在キャラクターが保持しているインタラクト用ウィジェットのポインタ
UInteractWidget* ASotugyouSeisakuCharacter::GetInteractWidget() const
{
	return mInteractWidget;
}

/// @brief コントローラーからキャラクターの所有権が外れたときに呼ばれる関数
void ASotugyouSeisakuCharacter::UnPossessed()
{
	Super::UnPossessed();

	//所有権が外れたらUIを隠す
	if (mInteractWidget)
	{
		mInteractWidget->HideWidget();
	}
}