// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Gimmick_PushBlock.h"
#include "GameFramework/PlayerStart.h"
#include "SotugyouSeisakuCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class AGimmick_PlayerSwitch;
class AGimmick_FallFloorManager;
class UInteractWidget;


DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ASotugyouSeisakuCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* mPushAction;

public:
	ASotugyouSeisakuCharacter();
	
	//アニメーションブループリントから呼び出せるようにする
	UFUNCTION(BlueprintPure, Category = "Character State")
	bool IsPushingBlock() const { return bIsPushing; }

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	//インタラクトできるギミックを検出
	UFUNCTION()
	void CheckForGimmick();

	//押す開始/終了
	UFUNCTION()
	void StartPush();
	UFUNCTION()
	void StopPush();
			

protected:
	virtual void BeginPlay() override;

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	virtual void Tick(float DeltaTime) override;

	//プレイヤーがアンポゼスされた時に呼ばれる
	virtual void UnPossessed() override;

	//インタラクトUIを取得
	UInteractWidget* GetInteractWidget() const;

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	//プレイヤーを指定位置にリスポーンさせる関数
	UFUNCTION()
	void RespawnPlayer();

	//インタラクト可能なオブジェクトを探す
	UFUNCTION()
	void CheckForInteractables();

	//UIの更新
	UFUNCTION()
	void UpdateInteractUI();

	//現在インタラクト可能なオブジェクト
	UPROPERTY()
	TScriptInterface<class IInteractable> mCurrentInteractable;

	//インタラクトUI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UInteractWidget> mInteractWidgetClass;

	UPROPERTY()
	class UInteractWidget* mInteractWidget;

	//インタラクト検知の範囲
	UPROPERTY(EditAnywhere, Category = "Interaction")
	float mInteractCheckDistance = 300.0f;

	//インタラクト検知の半径
	UPROPERTY(EditAnywhere, Category = "Interaction")
	float mInteractCheckRadius = 100.0f;

	//インタラクト検知の間隔（秒）
	UPROPERTY(EditAnywhere, Category = "Interaction")
	float mInteractCheckInterval = 0.1f;

	//押すブロック
	UPROPERTY()
	AGimmick_PushBlock* mTargetBlock;

	UPROPERTY()
	AActor* mPlayerStart;

	//押しアニメーション
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Push")
	UAnimMontage* PushAnimMontage;

	//押している時の距離設定
	UPROPERTY(EditAnywhere,Category="Push")
	float mPushDistance = 0.0f;

	//リスポーンするZ座標
	UPROPERTY(EditAnywhere, Category = "Respawn")
	float mRespawnZ = 0.0f;

	//近くにある切り替えポイント
	UPROPERTY()
	AGimmick_PlayerSwitch* mNearbySwitchPoint = nullptr;

	//床のマネージャーへの参照
	UPROPERTY()
	TObjectPtr<AGimmick_FallFloorManager> mFloorManager;

	//プレイヤーがギミックブロックを押しているかどうか
	bool bIsPushing = false;

	//タイマーハンドル
	FTimerHandle mInteractCheckTimerHandle;
	//プレイヤーの前フレーム位置
	FVector PrevLocation;
	//プレイヤーの前フレーム回転値
	FRotator  PrevRotation;

	//切り替えポイントを設定（切り替えポイントから呼ばれる）
	void SetNearbySwitch(AGimmick_PlayerSwitch* SwitchPoint) { mNearbySwitchPoint = SwitchPoint; }

	//インタラクト（Fキー）
	void TryInteract();
};

