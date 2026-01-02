// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/InteractableActor.h"

#include "EscapeITCameraManager.h"
#include "Components/WidgetComponent.h"
#include "EscapeITPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Inventory/InteractionPromptWidget.h"

// Sets default values
AInteractableActor::AInteractableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PromptWidget"));
	PromptWidget->SetupAttachment(RootComponent);
	PromptWidget->SetWidgetSpace(EWidgetSpace::World);
	PromptWidget->SetDrawSize(FVector2D(50.f,50.f));
	PromptWidget->SetPivot(FVector2D(0.5f,0.5f));
	PromptWidget->SetVisibility(false);
	
	bPlayerNearby = false;
}

// Called when the game starts or when spawned
void AInteractableActor::BeginPlay()
{
	Super::BeginPlay();
	InitializePromptWidget();
	ShowPrompt(false);
	
}

void AInteractableActor::Interact_Implementation(AActor* Interactor)
{
	if (!bPlayerNearby || !Interactor) return;
}

void AInteractableActor::OnInteractionBeginOverlap_Implementation(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this) return;
	
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn || !Pawn->IsPlayerControlled()) return;
	
	bPlayerNearby = true;
	ShowPrompt(true);
	NotifyPlayerControllerEnter(Pawn);
}

void AInteractableActor::OnInteractionEndOverlap_Implementation(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor || OtherActor == this) return;
	
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn || !Pawn->IsPlayerControlled()) return;
	
	bPlayerNearby = false;
	ShowPrompt(false);
	NotifyPlayerControllerLeave(Pawn);
}

void AInteractableActor::ShowPrompt(bool bShow)
{
	if (PromptWidget)
	{
		PromptWidget->SetVisibility(bShow);
	}

	if (InteractionPromptWidget) 
	{
		if (bShow)
		{
			InteractionPromptWidget->ShowPrompt();
		}
		else
		{
			InteractionPromptWidget->HidePrompt();
		}
	}
}

void AInteractableActor::InitializePromptWidget()
{
	if (!PromptWidget) return;

	if (InteractionPromptWidgetClass)
	{
		PromptWidget->SetWidgetClass(InteractionPromptWidgetClass);
		PromptWidget->InitWidget();

		if (UInteractionPromptWidget* Widget = Cast<UInteractionPromptWidget>(PromptWidget->GetWidget()))
		{
			InteractionPromptWidget = Widget;
		}
	}
}

void AInteractableActor::NotifyPlayerControllerEnter(AActor* Player)
{
	if (APawn* Pawn = Cast<APawn>(Player))
	{
		if (AEscapeITPlayerController* PC = Cast<AEscapeITPlayerController>(Pawn->GetController()))
		{
			PC->OnEnterInteractableRange(this);
		}
	}
}

void AInteractableActor::NotifyPlayerControllerLeave(AActor* Player)
{
	if (APawn* Pawn = Cast<APawn>(Player))
	{
		if (AEscapeITPlayerController* PC = Cast<AEscapeITPlayerController>(Pawn->GetController()))
		{
			PC->OnLeaveInteractableRange(this);
		}
	}
}

void AInteractableActor::StartHoldInteraction()
{
	if (InteractionPromptWidget)
	{
		InteractionPromptWidget->StartHold();
	}
}

void AInteractableActor::UpdateHoldProgress(float Progress)
{
	if (InteractionPromptWidget)
	{
		InteractionPromptWidget->UpdateHoldProgress(Progress);
	}
}

void AInteractableActor::CancelHoldInteraction()
{
	if (InteractionPromptWidget)
	{
		InteractionPromptWidget->CancelHold();
	}
}

void AInteractableActor::CompleteHoldInteraction()
{
	if (InteractionPromptWidget)
	{
		InteractionPromptWidget->CompleteHold();
	}
}

void AInteractableActor::UpdateWidgetRotation()
{
	if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(),0))
	{
		FVector CameraLocation = CameraManager->GetCameraLocation();
		FVector WidgetLocation = PromptWidget->GetComponentLocation();
		
		FRotator Look = (CameraLocation - WidgetLocation).Rotation();
		
		PromptWidget->SetWorldRotation(Look);
	}
}


