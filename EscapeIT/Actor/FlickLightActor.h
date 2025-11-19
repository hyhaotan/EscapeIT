// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GhostActor.h"
#include "FlickLightActor.generated.h"

UCLASS()
class ESCAPEIT_API AFlickLightActor : public AActor
{
	GENERATED_BODY()

public:
	AFlickLightActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class UStaticMeshComponent* LightMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class UPointLightComponent* PointLight;

	// Ghost settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost")
	TSubclassOf<AGhostActor> GhostActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost")
	FVector GhostSpawnLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost")
	FRotator GhostSpawnRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost")
	float GhostFadeInDuration = 2.0f;

	// Flicker settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker Settings")
	float FlickerDuration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker Settings")
	float MinFlickerInterval = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker Settings")
	float MaxFlickerInterval = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker Settings")
	float DelayBeforeFlicker = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker Settings")
	bool bIntensifyFlickerOverTime = true; // Đèn chớp nhanh dần

	// Light intensity settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Settings")
	float NormalLightIntensity = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Settings")
	float FlickerLightIntensity = 5000.0f; // Đèn sáng hơn khi chớp

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Settings")
	bool bEnableLightColorChange = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Settings")
	FLinearColor NormalLightColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f); // Vàng nhạt

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Settings")
	FLinearColor FlickerLightColor = FLinearColor(1.0f, 0.3f, 0.1f, 1.0f); // Đỏ cam

	// Camera shake
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	TSubclassOf<class UCameraShakeBase> LightFlickerCameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	TSubclassOf<class UCameraShakeBase> GhostAppearCameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraShakeScale = 1.0f;

	// Particle effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	class UParticleSystem* ElectricalSparkParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	class UParticleSystem* GhostAppearParticle;

	// Dramatic pause
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
	bool bEnableDramaticPause = true; // Tạm dừng trước khi ghost xuất hiện

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
	float DramaticPauseDuration = 0.8f; // Tắt đèn hoàn toàn trước khi ghost xuất hiện

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
	int32 NumberOfFlickersBeforeGhost = 8; // Số lần chớp trước khi ghost xuất hiện

	UFUNCTION(BlueprintCallable, Category = "Light Control")
	void StartFlickerSequence();

	UFUNCTION(BlueprintCallable, Category = "Light Control")
	void StopSequenceImmediately();

	UFUNCTION(BlueprintCallable, Category = "Light Control")
	void ResetSequence(); // Reset về trạng thái ban đầu

	UFUNCTION(BlueprintCallable, Category = "Light Control")
	void RestartSequence(); // Reset và bắt đầu lại ngay

	// Auto reset settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
	bool bAutoResetAfterComplete = false; // Tự động reset sau khi hoàn thành

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
	float AutoResetDelay = 10.0f; // Thời gian chờ trước khi reset tự động

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
	bool bLoopSequence = false; // Lặp lại sequence liên tục

private:
	bool bIsFlickering = false;
	float FlickerTimer = 0.0f;
	float NextFlickerTime = 0.0f;
	float TotalFlickerTime = 0.0f;
	bool bHasSpawnedGhost = false;
	bool bIsLightOn = true;
	bool bSequenceStarted = false;
	bool bInDramaticPause = false;
	float DramaticPauseTimer = 0.0f;
	int32 FlickerCount = 0;

	class UAudioComponent* AmbientDroneAudioComponent;
	class UParticleSystemComponent* SparkParticleComponent;
	TArray<AActor*> SpawnedGhosts; // Theo dõi ghost đã spawn
	FTimerHandle AutoResetTimerHandle;
	FTimerHandle DelayTimerHandle;

	void UpdateFlicker(float DeltaTime);
	void ToggleLight();
	void SpawnGhost();
	void StopFlicker();
	void TriggerCameraShake(TSubclassOf<class UCameraShakeBase> ShakeClass);
	void StartDramaticPause();
	void UpdateDramaticPause(float DeltaTime);
	float GetCurrentFlickerInterval();
	void ResetAllVariables(); // Reset tất cả biến về mặc định
	void DestroySpawnedGhosts(); // Xóa tất cả ghost đã spawn
	void HandleAutoReset(); // Xử lý auto reset
};