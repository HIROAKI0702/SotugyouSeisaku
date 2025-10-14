// Fill out your copyright notice in the Description page of Project Settings.


#include "MirrorActor.h"
#include "LaserGimmickTargetActor.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AMirrorActor::AMirrorActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//ルートコンポーネント作成
	mRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = mRoot;

	//鏡のメッシュ
	mMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MirrorMesh"));
	mMesh->SetupAttachment(RootComponent);
}

/// @brief レーザーが鏡に当たったら、その角度通りに反射して次の方向へ進ませる関数
/// @param Hit レーザーが鏡に当たった時の衝突情報
/// @param IncomingDir レーザーが鏡に飛んできた方向
void AMirrorActor::ReflectLaser(const FHitResult& Hit, const FVector& IncomingDir)
{
	//鏡の表面の法線ベクトル
	FVector Normal = Hit.ImpactNormal;
	//入射ベクトルを法線で反射させた方向を計算
	FVector ReflectedDir = UKismetMathLibrary::GetReflectionVector(IncomingDir, Normal);

	//光の始点と終点
	FVector Start = Hit.ImpactPoint + ReflectedDir * 5.0f;
	FVector End = Start + ReflectedDir * ReflectDistance;

	//衝突判定の設定
	FHitResult ReflectHit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	//実際に光線を飛ばして何かに当たるか確認
	if (GetWorld()->LineTraceSingleByChannel(ReflectHit, Start, End, ECC_Visibility, Params))
	{
		//もし反射先が光受信機（ドアを開ける装置）なら
		if (ALaserGimmickTargetActor* Receiver = Cast<ALaserGimmickTargetActor>(ReflectHit.GetActor()))
		{
			//光が当たったと通知する
			Receiver->OnLightHit();
		}
	}
}
