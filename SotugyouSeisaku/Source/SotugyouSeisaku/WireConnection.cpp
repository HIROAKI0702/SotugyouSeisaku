// Fill out your copyright notice in the Description page of Project Settings.


#include "WireConnection.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "WireNode.h"
#include "SotugyouSeisakuCharacter.h"

// Sets default values

/// @brief コンストラクタ　各種設定
AWireConnection::AWireConnection()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//ルートコンポーネント作成
	mRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = mRoot;

	//スプラインコンポーネント作成（ワイヤーのパスを定義）
	mSpline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	mSpline->SetupAttachment(RootComponent);

	mSplineMesh = CreateDefaultSubobject<USplineMeshComponent>(TEXT("SplineMesh"));
	mSplineMesh->SetupAttachment(RootComponent);

	//デフォルト値を設定
	mWireThickness = 5.0f;
	mStartNode = nullptr;
	mEndNode = nullptr;
	mCarryingPlayer = nullptr;
	mDynamicMaterial = nullptr;
	bAttachedToPlayer = false;
}

// Called when the game starts or when spawned
void AWireConnection::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWireConnection::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//プレイヤーが持っている場合、常にワイヤーの位置を更新
	if (bAttachedToPlayer && mCarryingPlayer)
	{
		UpdateWireVisuals();
	}
}

/// @brief ワイヤーの接続を設定
/// @param StartNode 開始ノード
/// @param EndNode 終了ノード（nullの場合はプレイヤーが持っている状態）
/// @param Player ワイヤーを持っているプレイヤー（nullの場合は接続済み）
void AWireConnection::SetupConnection(AWireNode* StartNode, AWireNode* EndNode, ASotugyouSeisakuCharacter* Player)
{
	if (!StartNode)
	{
		return;
	}

	mStartNode = StartNode;
	mEndNode = EndNode;
	mCarryingPlayer = Player;

	//スプラインをクリアして新しいポイントを追加
	mSpline->ClearSplinePoints();
	mSpline->AddSplinePoint(StartNode->GetActorLocation(), ESplineCoordinateSpace::World);

	if (EndNode)
	{
		//接続完了時：エンドノードまで線を引く
		mSpline->AddSplinePoint(EndNode->GetActorLocation(), ESplineCoordinateSpace::World);
		bAttachedToPlayer = false;
	}
	else if (Player)
	{
		//プレイヤーが持っている時：プレイヤーの手まで線を引く
		FVector PlayerHandLocation = Player->GetActorLocation() + FVector(0, 0, 100.0f);
		mSpline->AddSplinePoint(PlayerHandLocation, ESplineCoordinateSpace::World);
		bAttachedToPlayer = true;
	}

	//ワイヤーの色を設定
	SetWireColor(StartNode->GetWireColorValue());

	//視覚的な表現を更新
	UpdateWireVisuals();
}

/// @brief ワイヤーの視覚的な表現を更新
void AWireConnection::UpdateWireVisuals()
{
	if (!mStartNode)
	{
		return;
	}

	//スタート地点は常に開始ノードの位置に更新
	mSpline->SetLocationAtSplinePoint(0, mStartNode->GetActorLocation(), ESplineCoordinateSpace::World);

	if (mEndNode)
	{
		//接続済みの場合：終了ノードの位置に更新
		mSpline->SetLocationAtSplinePoint(1, mEndNode->GetActorLocation(), ESplineCoordinateSpace::World);
	}
	else if (bAttachedToPlayer && mCarryingPlayer)
	{
		//プレイヤーが持っている場合：プレイヤーの手の位置に追従
		//TODO: 実際の手のソケット位置を使用する場合は調整が必要
		FVector PlayerHandLocation = mCarryingPlayer->GetActorLocation() + FVector(0, 0, 100.0f);
		mSpline->SetLocationAtSplinePoint(1, PlayerHandLocation, ESplineCoordinateSpace::World);
	}
}

/// @brief ワイヤーの色を設定
/// @param Color 設定する色
void AWireConnection::SetWireColor(FLinearColor Color)
{
	if (mWireMaterial)
	{
		//動的マテリアルインスタンスを作成
		mDynamicMaterial = UMaterialInstanceDynamic::Create(mWireMaterial, this);
		if (mDynamicMaterial)
		{
			//マテリアルの"Color"パラメータに色を設定
			mDynamicMaterial->SetVectorParameterValue(FName("Color"), Color);
		}
	}
}

/// @brief ワイヤーをプレイヤーにアタッチ
/// @param Player ワイヤーを持つプレイヤー
void AWireConnection::AttachToPlayer(ASotugyouSeisakuCharacter* Player)
{
	if (!Player)
	{
		return;
	}

	mCarryingPlayer = Player;
	bAttachedToPlayer = true;
}

