// Fill out your copyright notice in the Description page of Project Settings.

#include "WindowJumpscareActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Curves/CurveFloat.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "GameFramework/SpringArmComponent.h"
#include "EscapeIT/EscapeITCharacter.h"

AWindowJumpscareActor::AWindowJumpscareActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Setup components
	WindowFrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Window Frame"));
	RootComponent = WindowFrameMesh;

	LeftWindowHinge = CreateDefaultSubobject<USceneComponent>(TEXT("Left Window Hinge"));
	LeftWindowHinge->SetupAttachment(RootComponent);

	LeftWindowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Left Window"));
	LeftWindowMesh->SetupAttachment(LeftWindowHinge);

	RightWindowHinge = CreateDefaultSubobject<USceneComponent>(TEXT("Right Window Hinge"));
	RightWindowHinge->SetupAttachment(RootComponent);

	RightWindowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Right Window"));
	RightWindowMesh->SetupAttachment(RightWindowHinge);

	GhostMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ghost Actor"));
	GhostMesh->SetupAttachment(RootComponent);
	GhostMesh->SetVisibility(false);

	BoxCollission = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collission"));
	BoxCollission->SetupAttachment(RootComponent);
	BoxCollission->OnComponentBeginOverlap.AddDynamic(this, &AWindowJumpscareActor::OnOverlapBegin);

	GhostSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Ghost Spring Arm"));
	GhostSpringArm->SetupAttachment(GhostMesh,FName("head"));

	GhostCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Ghost Camera"));
	GhostCamera->SetupAttachment(GhostSpringArm);

	WindowTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Window Timeline"));
}

void AWindowJumpscareActor::BeginPlay()
{
	Super::BeginPlay();

	LeftHingeInitialRot = LeftWindowHinge->GetRelativeRotation();
	RightHingeInitialRot = RightWindowHinge->GetRelativeRotation();

	if (WindowTimeline && WindowCurve)
	{
		FOnTimelineFloat TimelineCallback;
		TimelineCallback.BindUFunction(this, FName("UpdateWindowRotation"));
		WindowTimeline->AddInterpFloat(WindowCurve, TimelineCallback);

		WindowTimeline->SetLooping(false);
		WindowTimeline->SetPlayRate(1.0f);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WindowTimeline or WindowCurve is not set!"));
	}
}

void AWindowJumpscareActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWindowJumpscareActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bHasTriggered)
		return;

	APlayerController* PlayerCon = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerCon)
		return;

	AActor* PlayerPawn = PlayerCon->GetPawn();
	if (PlayerPawn == OtherActor)
	{
		bHasTriggered = true;
		TriggerJumpscare();
	}
}

void AWindowJumpscareActor::UpdateWindowRotation(float Value)
{
	FRotator NewLeftRot = LeftHingeInitialRot;
	NewLeftRot.Yaw += LeftWindowOpenAngle * Value;
	LeftWindowHinge->SetRelativeRotation(NewLeftRot);

	FRotator NewRightRot = RightHingeInitialRot;
	NewRightRot.Yaw -= RightWindowOpenAngle * Value; 
	RightWindowHinge->SetRelativeRotation(NewRightRot);
}

void AWindowJumpscareActor::TriggerJumpscare()
{
	if (!GhostCamera) // Kiểm tra GhostCamera thay vì OwningCharacter
	{
		UE_LOG(LogTemp, Warning, TEXT("GhostCamera is not set!"));
		return;
	}

	TObjectPtr<APlayerController> PlayerCon = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerCon)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't get the player controller"));
		return;
	}

	// Lưu lại ViewTarget ban đầu để restore sau
	OriginalViewTarget = PlayerCon->GetViewTarget();

	// Chuyển camera sang Actor này (WindowJumpscareActor) - nó sẽ dùng GhostCamera
	PlayerCon->SetViewTargetWithBlend(this, 1.0f, EViewTargetBlendFunction::VTBlend_EaseInOut, 1.0f, false);

	if (WindowTimeline)
	{
		WindowTimeline->PlayFromStart();
	}

	if (GhostMesh)
	{
		GhostMesh->SetVisibility(true);
	}

	if (WindowOpenSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WindowOpenSound, GetActorLocation());
	}

	if (JumpscareSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, JumpscareSound, GetActorLocation());
	}

	GetWorldTimerManager().SetTimer(CloseWindowTimer, this, &AWindowJumpscareActor::CloseWindows, CloseDelayTime, false);
}

void AWindowJumpscareActor::CloseWindows()
{
	if (WindowTimeline)
	{
		WindowTimeline->Reverse();
	}

	if (GhostMesh)
	{
		GhostMesh->SetVisibility(false);
	}

	TObjectPtr<APlayerController> PlayerCon = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerCon && OriginalViewTarget)
	{
		PlayerCon->SetViewTargetWithBlend(OriginalViewTarget, 1.0f, EViewTargetBlendFunction::VTBlend_EaseInOut, 1.0f, false);
	}
}