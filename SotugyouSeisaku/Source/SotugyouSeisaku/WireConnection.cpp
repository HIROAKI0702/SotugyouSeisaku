#include "WireConnection.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "WireNode.h"
#include "SotugyouSeisakuCharacter.h"

/// @brief コンストラクタ　ワイヤー接続の各種設定
AWireConnection::AWireConnection()
{
	//毎フレームTickを呼ぶ
	PrimaryActorTick.bCanEverTick = true;

	//ルートコンポーネント作成
	mRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = mRoot;

	//スプラインコンポーネント作成（ワイヤーのパスを定義）
	mSpline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	mSpline->SetupAttachment(RootComponent);

	//デフォルト値を設定
	mWireThickness = 0.2f;
	mStartNode = nullptr;
	mEndNode = nullptr;
	mCarryingPlayer = nullptr;
	mDynamicMaterial = nullptr;
	bAttachedToPlayer = false;
}

/// @brief ゲーム開始時に呼ばれる初期化処理
void AWireConnection::BeginPlay()
{
	Super::BeginPlay();
}

/// @brief 毎フレーム呼ばれる更新処理
/// @param DeltaTime 前フレームからの経過時間
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

	//SplineMeshComponentを動的に生成
	if (!mSplineMesh && mWireMesh)
	{
		mSplineMesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass(), TEXT("DynamicSplineMesh"));
		mSplineMesh->SetMobility(EComponentMobility::Movable);//可動に設定
		mSplineMesh->AttachToComponent(mRoot, FAttachmentTransformRules::KeepRelativeTransform);
		mSplineMesh->RegisterComponent();
		mSplineMesh->SetStaticMesh(mWireMesh);
		mSplineMesh->SetForwardAxis(ESplineMeshAxis::Z);
		mSplineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);//コリジョン無効化

		if (mDynamicMaterial)
		{
			mSplineMesh->SetMaterial(0, mDynamicMaterial);
		}
	}

	//視覚的な表現を更新
	UpdateWireVisuals();
}

/// @brief ワイヤーの視覚的な表現を更新
void AWireConnection::UpdateWireVisuals()
{
	if (!mStartNode || !mSpline)
		return;

	//スタートとエンド位置を更新
	FVector StartPos = mStartNode->GetActorLocation();
	mSpline->SetLocationAtSplinePoint(0, StartPos, ESplineCoordinateSpace::World);

	FVector EndPos;
	if (mEndNode)
	{
		EndPos = mEndNode->GetActorLocation();
	}
	else if (bAttachedToPlayer && mCarryingPlayer)
	{
		EndPos = mCarryingPlayer->GetActorLocation() + FVector(0, 0, 100.0f);

	}
	else
	{
		return;
	}

    // スプラインに自然なたわみを追加
	float Distance = FVector::Distance(StartPos, EndPos);
	float SagAmount = FMath::Clamp(Distance * 0.1f, 20.0f, 150.0f);//距離に応じたたわみ
	FVector MidPos = (StartPos + EndPos) * 0.5f;
	MidPos.Z -= SagAmount;//中央を下方向にたわませる

	// 地面の高さを検出して貫通防止
	FHitResult Hit;
	FVector TraceStart = MidPos + FVector(0, 0, 2000.0f);//上から
	FVector TraceEnd = MidPos - FVector(0, 0, 5000.0f);//下方向に
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.bTraceComplex = false;

	if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
	{
		float GroundZ = Hit.Location.Z + 10.0f;//地面より少し上に設定
		if (MidPos.Z < GroundZ)
		{
			MidPos.Z = GroundZ;
		}
	}

	//スプライン更新（中間点を含む3点構成）
	mSpline->ClearSplinePoints();
	mSpline->AddSplinePoint(StartPos, ESplineCoordinateSpace::World);
	mSpline->AddSplinePoint(MidPos, ESplineCoordinateSpace::World);
	mSpline->AddSplinePoint(EndPos, ESplineCoordinateSpace::World);
	mSpline->UpdateSpline();

	//既存のSplineMeshを削除して再生成
	for (auto* Comp : mSplineMeshes)
	{
		if (Comp)
			Comp->DestroyComponent();
	}
	mSplineMeshes.Empty();

	//各スプライン区間ごとにメッシュを生成
	const int32 NumSegments = mSpline->GetNumberOfSplinePoints() - 1;

	for (int32 i = 0; i < NumSegments; ++i)
	{
		USplineMeshComponent* Segment = NewObject<USplineMeshComponent>(this);
		Segment->SetMobility(EComponentMobility::Movable);
		Segment->AttachToComponent(mSpline, FAttachmentTransformRules::KeepRelativeTransform);
		Segment->SetStaticMesh(mWireMesh);
		Segment->SetForwardAxis(ESplineMeshAxis::Z);
		Segment->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//太さ反映
		FVector2D WireScale(mWireThickness, mWireThickness);
		Segment->SetStartScale(WireScale);
		Segment->SetEndScale(WireScale);

		//色反映
		if (mDynamicMaterial)
			Segment->SetMaterial(0, mDynamicMaterial);

		//スプライン区間ごとの位置と接線を取得
		FVector StartPosLocal, StartTangent, EndPosLocal, EndTangent;
		mSpline->GetLocationAndTangentAtSplinePoint(i, StartPosLocal, StartTangent, ESplineCoordinateSpace::Local);
		mSpline->GetLocationAndTangentAtSplinePoint(i + 1, EndPosLocal, EndTangent, ESplineCoordinateSpace::Local);

		//スタートとエンド設定
		Segment->SetStartAndEnd(StartPosLocal, StartTangent, EndPosLocal, EndTangent);

		//登録・表示
		Segment->RegisterComponent();
		mSplineMeshes.Add(Segment);
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