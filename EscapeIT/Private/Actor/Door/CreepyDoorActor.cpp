
#include "Actor/Door/CreepyDoorActor.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Components/TimelineComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ACreepyDoorActor::ACreepyDoorActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// ============== TÍNH NĂNG 2: Tạo ánh sáng ==============
	DoorLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("DoorLight"));
	DoorLight->SetupAttachment(DoorFrame);
	DoorLight->SetRelativeLocation(FVector(-100.0f, 0.0f, 0.0f));
	DoorLight->SetIntensity(LightIntensityMin);
	DoorLight->SetLightColor(LightColorStart);
	DoorLight->SetAttenuationRadius(500.0f);
	DoorLight->SetVisibility(false);

	// ============== TÍNH NĂNG 3: Tạo particle systems ==============
	DustParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("DustParticles"));
	DustParticles->SetupAttachment(Door);
	DustParticles->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
	DustParticles->bAutoActivate = false;

	FogParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FogParticles"));
	FogParticles->SetupAttachment(DoorFrame);
	FogParticles->SetRelativeLocation(FVector(-50.0f, 0.0f, 0.0f));
	FogParticles->bAutoActivate = false;

	// ============== TÍNH NĂNG 5: Tạo shadow figure ==============
	ShadowFigure = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShadowFigure"));
	ShadowFigure->SetupAttachment(DoorFrame);
	ShadowFigure->SetRelativeLocation(ShadowStartLocation);
	ShadowFigure->SetRelativeScale3D(FVector(0.3f, 0.1f, 1.5f));
	ShadowFigure->SetVisibility(false);
	ShadowFigure->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Default values
	CurrentShakeTime = 0.0f;
	bIsShaking = false;
	LightFlickerTime = 0.0f;
	ShadowMoveProgress = 0.0f;
	bHasPaused = false;
	OpenAngle = 45.0f;
	PauseAtProgress = 0.5f; // FIX: Thêm giá trị mặc định
	ShadowDynamicMaterial = nullptr; // FIX: Khởi tạo pointer
}

void ACreepyDoorActor::BeginPlay()
{
	Super::BeginPlay();

	// Lưu rotation gốc của cửa
	if (Door)
	{
		OriginalDoorRotation = Door->GetRelativeRotation();
	}

	// Set particle templates nếu có
	if (DustEffect && DustParticles)
	{
		DustParticles->SetTemplate(DustEffect);
	}
	if (FogEffect && FogParticles)
	{
		FogParticles->SetTemplate(FogEffect);
	}

	// FIX: Tạo dynamic material CHỈ MỘT LẦN để tránh memory leak
	if (ShadowFigure && ShadowMaterial)
	{
		ShadowDynamicMaterial = ShadowFigure->CreateDynamicMaterialInstance(0, ShadowMaterial);
	}

	// FIX: Kiểm tra DoorTimeline tồn tại
	if (!DoorTimeline)
	{
		UE_LOG(LogTemp, Error, TEXT("DoorTimeline is null! Please setup timeline in parent class."));
		return;
	}

	// Tự động mở cửa sau 3 giây với hiệu ứng creepy
	FTimerHandle InitialOpenTimer;
	GetWorldTimerManager().SetTimer(InitialOpenTimer, this,
		&ACreepyDoorActor::OpenDoorWithCreepyEffects, 3.0f, false);
}

void ACreepyDoorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update shake effect
	if (bIsShaking)
	{
		UpdateDoorShake(DeltaTime);
	}

	// Update shadow movement
	if (ShadowMoveProgress > 0.0f && ShadowMoveProgress < 1.0f)
	{
		UpdateShadowMovement(DeltaTime);
	}
}

// ============================================================================
// TÍNH NĂNG CHÍNH: Mở cửa với tất cả hiệu ứng creepy
// ============================================================================
void ACreepyDoorActor::OpenDoorWithCreepyEffects()
{
	if (!DoorTimeline || DoorTimeline->IsPlaying())
	{
		return;
	}

	// FIX: Clear tất cả timer cũ trước khi bắt đầu
	ClearAllTimers();

	bIsOpen = true;
	bHasPaused = false;
	

	// Bắt đầu ánh sáng nhấp nháy
	StartLightFlicker();

	// Hiện bóng người và di chuyển
	ShowShadowFigure();

	// Bắt đầu mở cửa chậm
	DoorTimeline->SetPlayRate(0.3f);
	DoorTimeline->PlayFromStart();

	// Kích hoạt particle effects
	ActivateParticleEffects();

	// Tạm dừng cửa ở giữa chừng
	if (bEnableRandomBehavior && AnimationDuration > 0.0f)
	{
		// FIX: Tính toán thời gian pause chính xác
		float PauseTime = (AnimationDuration * PauseAtProgress) / DoorTimeline->GetPlayRate();
		GetWorldTimerManager().SetTimer(PauseTimerHandle, this,
			&ACreepyDoorActor::PauseDoorAtHalf, PauseTime, false);
	}
}

// ============================================================================
// TÍNH NĂNG 1: SHAKE EFFECT + ÂM THANH
// ============================================================================
void ACreepyDoorActor::StartDoorShake()
{
	if (bIsShaking || !Door)
	{
		return;
	}

	bIsShaking = true;
	CurrentShakeTime = 0.0f;

	// Timer để dừng shake
	GetWorldTimerManager().SetTimer(ShakeTimerHandle, this,
		&ACreepyDoorActor::StopDoorShake, ShakeDuration, false);
}

// FIX: Thêm DeltaTime parameter để tính toán chính xác
void ACreepyDoorActor::UpdateDoorShake(float DeltaTime)
{
	if (!Door || !bIsShaking)
	{
		return;
	}

	CurrentShakeTime += DeltaTime;

	// FIX: Lưu current rotation từ timeline, không ghi đè hoàn toàn
	FRotator TimelineRotation = Door->GetRelativeRotation();
	
	// Tạo random shake nhỏ CHỈ trên trục Yaw
	float ShakeAmount = FMath::Sin(CurrentShakeTime * 20.0f) * ShakeIntensity;
	
	// Áp dụng shake lên rotation hiện tại thay vì OriginalDoorRotation
	FRotator ShakeRotation = TimelineRotation;
	ShakeRotation.Yaw += ShakeAmount;

	Door->SetRelativeRotation(ShakeRotation);
}

void ACreepyDoorActor::StopDoorShake()
{
	bIsShaking = false;
	CurrentShakeTime = 0.0f;
	GetWorldTimerManager().ClearTimer(ShakeTimerHandle);
	
	// FIX: Không cần reset rotation, timeline sẽ tự quản lý
}

void ACreepyDoorActor::PlayCreepySound(USoundBase* Sound)
{
	if (Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
	}
}

// ============================================================================
// TÍNH NĂNG 2: ÁNH SÁNG THAY ĐỔI
// ============================================================================
void ACreepyDoorActor::StartLightFlicker()
{
	if (!DoorLight)
	{
		return;
	}

	DoorLight->SetVisibility(true);
	LightFlickerTime = 0.0f;

	// Timer để update light liên tục
	GetWorldTimerManager().SetTimer(LightFlickerTimerHandle, this,
		&ACreepyDoorActor::UpdateLightFlicker, LightFlickerSpeed, true);
}

void ACreepyDoorActor::UpdateLightFlicker()
{
	if (!DoorLight)
	{
		return;
	}

	LightFlickerTime += LightFlickerSpeed;

	// Random flicker intensity
	float RandomIntensity = FMath::RandRange(LightIntensityMin, LightIntensityMax);
	DoorLight->SetIntensity(RandomIntensity);

	// Transition màu sắc dựa theo progress của cửa
	if (DoorTimeline && AnimationDuration > 0.0f)
	{
		float DoorProgress = FMath::Clamp(
			DoorTimeline->GetPlaybackPosition() / AnimationDuration, 0.0f, 1.0f);
		TransitionLightColor(DoorProgress);
	}

	// Dừng flicker khi cửa mở xong
	if (DoorTimeline && !DoorTimeline->IsPlaying() && !DoorTimeline->IsReversing())
	{
		GetWorldTimerManager().ClearTimer(LightFlickerTimerHandle);
		DoorLight->SetIntensity(LightIntensityMax);
	}
}

void ACreepyDoorActor::TransitionLightColor(float Progress)
{
	if (!DoorLight)
	{
		return;
	}

	// Lerp từ màu đỏ sang xanh lạnh
	FLinearColor CurrentColor = FMath::Lerp(LightColorStart, LightColorEnd, Progress);
	DoorLight->SetLightColor(CurrentColor);
}

// ============================================================================
// TÍNH NĂNG 3: PARTICLE EFFECTS
// ============================================================================
void ACreepyDoorActor::ActivateParticleEffects()
{
	// Kích hoạt bụi
	if (DustParticles)
	{
		DustParticles->Activate(true);
	}

	// Kích hoạt sương mù sau 0.5 giây
	FTimerHandle FogDelayTimer;
	GetWorldTimerManager().SetTimer(FogDelayTimer, [this]()
		{
			if (FogParticles && IsValid(this))
			{
				FogParticles->Activate(true);
			}
		}, 0.5f, false);
}

// ============================================================================
// TÍNH NĂNG 4: CỬA TỰ ĐÓNG MỞ BẤT THƯỜNG
// ============================================================================
void ACreepyDoorActor::PauseDoorAtHalf()
{
	if (!DoorTimeline || bHasPaused)
	{
		return;
	}

	bHasPaused = true;

	// Dừng timeline
	DoorTimeline->Stop();

	// Bắt đầu shake
	StartDoorShake();

	// Tiếp tục mở sau khi shake xong
	GetWorldTimerManager().SetTimer(PauseTimerHandle, this,
		&ACreepyDoorActor::ResumeDoorOpening, ShakeDuration + 0.5f, false);
}

void ACreepyDoorActor::ResumeDoorOpening()
{
	if (!DoorTimeline)
	{
		return;
	}

	// FIX: Dừng shake trước khi tiếp tục
	if (bIsShaking)
	{
		StopDoorShake();
	}

	// Tiếp tục mở cửa
	DoorTimeline->Play();

	// Random: có 30% cơ hội đóng lại và mở lại
	if (bEnableRandomBehavior && FMath::RandRange(0.0f, 1.0f) < 0.3f)
	{
		GetWorldTimerManager().SetTimer(RandomCloseTimerHandle, this,
			&ACreepyDoorActor::CloseDoorRandomly, 2.0f, false);
	}
}

void ACreepyDoorActor::CloseDoorRandomly()
{
	if (!DoorTimeline)
	{
		return;
	}

	// FIX: Kiểm tra trạng thái trước khi đóng
	if (DoorTimeline->IsReversing())
	{
		return; // Đã đang đóng rồi
	}

	// Đóng cửa nhanh
	DoorTimeline->Reverse();

	// Mở lại sau 1 giây
	FTimerHandle ReopenTimer;
	GetWorldTimerManager().SetTimer(ReopenTimer, [this]()
		{
			if (DoorTimeline && IsValid(this))
			{
				// FIX: Chỉ mở lại nếu đang đóng
				if (DoorTimeline->IsReversing() || 
					DoorTimeline->GetPlaybackPosition() < AnimationDuration * 0.2f)
				{
					DoorTimeline->Play();
				}
			}
		}, 1.0f, false);
}

// ============================================================================
// TÍNH NĂNG 5: BÓNG ĐỔ/SILHOUETTE
// ============================================================================
void ACreepyDoorActor::ShowShadowFigure()
{
	if (!ShadowFigure)
	{
		return;
	}

	ShadowFigure->SetVisibility(true);
	ShadowFigure->SetRelativeLocation(ShadowStartLocation);
	ShadowMoveProgress = 0.0f;

	// FIX: Không cần timer riêng, dùng Tick để update
}

// FIX: Thêm DeltaTime parameter
void ACreepyDoorActor::UpdateShadowMovement(float DeltaTime)
{
	if (!ShadowFigure || ShadowMoveDuration <= 0.0f)
	{
		return;
	}

	ShadowMoveProgress += DeltaTime / ShadowMoveDuration;

	if (ShadowMoveProgress >= 1.0f)
	{
		ShadowMoveProgress = 1.0f;
		HideShadowFigure();
		return;
	}

	// Lerp vị trí
	FVector CurrentLocation = FMath::Lerp(
		ShadowStartLocation, ShadowEndLocation, ShadowMoveProgress);
	ShadowFigure->SetRelativeLocation(CurrentLocation);

	// FIX: Dùng material đã tạo sẵn, không tạo mới mỗi frame
	if (ShadowDynamicMaterial)
	{
		float Opacity = 1.0f - ShadowMoveProgress;
		ShadowDynamicMaterial->SetScalarParameterValue(FName("Opacity"), Opacity);
	}
}

void ACreepyDoorActor::HideShadowFigure()
{
	if (ShadowFigure)
	{
		ShadowFigure->SetVisibility(false);
	}

	ShadowMoveProgress = 0.0f;
}

void ACreepyDoorActor::UpdateDoorRotation_Implementation(float Value)
{
	Super::UpdateDoorRotation_Implementation(Value);

	if (!Door) return;
	
	// FIX: Lưu rotation để shake có thể sử dụng
	FRotator NewRotation = OriginalDoorRotation;
	NewRotation.Yaw += (OpenAngle * Value);
	Door->SetRelativeRotation(NewRotation);
}

void ACreepyDoorActor::CloseDoor_Implementation()
{
	Super::CloseDoor_Implementation();

	// Tắt tất cả hiệu ứng
	if (DoorLight)
	{
		DoorLight->SetVisibility(false);
	}

	if (DustParticles)
	{
		DustParticles->Deactivate();
	}

	if (FogParticles)
	{
		FogParticles->Deactivate();
	}

	HideShadowFigure();

	// FIX: Clear tất cả timers
	ClearAllTimers();
}

// FIX: Thêm helper function để clear tất cả timers
void ACreepyDoorActor::ClearAllTimers()
{
	GetWorldTimerManager().ClearTimer(LightFlickerTimerHandle);
	GetWorldTimerManager().ClearTimer(ShakeTimerHandle);
	GetWorldTimerManager().ClearTimer(PauseTimerHandle);
	GetWorldTimerManager().ClearTimer(RandomCloseTimerHandle);
	
	// Reset flags
	bIsShaking = false;
	bHasPaused = false;
}