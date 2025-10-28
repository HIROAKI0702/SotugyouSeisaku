// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RespawnManager.generated.h"

class ASotugyouSeisakuCharacter;
class APlayerStart;

/// @brief キャラクターごとのリスポーン設定を保持する構造体
USTRUCT(BlueprintType)
struct FCharacterRespawnSettings
{
	GENERATED_BODY()

	//リスポーン対象のキャラクター
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn Settings")
	TObjectPtr<ASotugyouSeisakuCharacter> TargetCharacter;

	//このキャラクターがリスポーンするZ座標の閾値
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn Settings")
	float RespawnThresholdZ = -3000.0f;

	//このキャラクター専用のリスポーン地点
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn Settings")
	TObjectPtr<APlayerStart> CustomRespawnPoint;

	//この設定が有効かどうか
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn Settings")
	bool bEnabled = true;

	//デフォルトコンストラクタ
	FCharacterRespawnSettings()
		: TargetCharacter(nullptr)
		, RespawnThresholdZ(-3000.0f)
		, CustomRespawnPoint(nullptr)
		, bEnabled(true)
	{
	}
};

UCLASS()
class SOTUGYOUSEISAKU_API ARespawnManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARespawnManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	//キャラクターのリスポーン設定（最大3つ）
	UPROPERTY(EditAnywhere, Category = "Respawn Settings", meta = (DisplayName = "Character Respawn Settings"))
	TArray<FCharacterRespawnSettings> mCharacterSettings;

	//デフォルトのリスポーン地点（カスタム地点が設定されていない場合に使用）
	UPROPERTY(EditAnywhere, Category = "Respawn Settings")
	TObjectPtr<APlayerStart> mDefaultRespawnPoint;

	//全キャラクター共通のデフォルトZ座標閾値
	UPROPERTY(EditAnywhere, Category = "Respawn Settings", meta = (DisplayName = "Default Threshold Z"))
	float mDefaultThresholdZ = -3000.0f;

	//FallFloorManagerへの参照（床の再生成用）
	UPROPERTY()
	class AGimmick_FallFloorManager* mFloorManager;

	//各キャラクターのリスポーン処理をチェック
	void CheckCharacterRespawns();

	//指定されたキャラクターをリスポーンさせる
	void RespawnCharacter(ASotugyouSeisakuCharacter* Character, APlayerStart* RespawnPoint);
};
