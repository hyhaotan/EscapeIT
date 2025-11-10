#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GhostActor.generated.h"

UCLASS()
class ESCAPEIT_API AGhostActor : public AActor
{
	GENERATED_BODY()

public:
	AGhostActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class UStaticMeshComponent* GhostMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class USkeletalMeshComponent* GhostSkeletalMesh;

	// Appearance settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	float FadeInDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	bool bLookAtPlayer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	float DisappearAfterTime = 5.0f; // -1 = never disappear

	// Material settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	FName OpacityParameterName = "Opacity";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	FName EmissiveStrengthParameterName = "EmissiveStrength";

	// Control functions
	UFUNCTION(BlueprintCallable, Category = "Ghost Control")
	void ForceDisappear(); // Biến mất ngay lập tức

	UFUNCTION(BlueprintCallable, Category = "Ghost Control")
	void StartFadeOutNow(); // Bắt đầu fade out

	UFUNCTION(BlueprintCallable, Category = "Ghost Control")
	void PauseAllEffects(); // Tạm dừng tất cả hiệu ứng

	UFUNCTION(BlueprintCallable, Category = "Ghost Control")
	void ResumeAllEffects(); // Tiếp tục hiệu ứng

private:
	float CurrentFadeValue = 0.0f;
	float FadeTimer = 0.0f;
	float LifetimeTimer = 0.0f;
	FVector InitialLocation;
	float FloatOffset = 0.0f;
	bool bIsFadingIn = true;
	bool bIsFadingOut = false;
	bool bIsPaused = false;
	FTimerHandle DisappearTimerHandle;

	TArray<class UMaterialInstanceDynamic*> DynamicMaterials;

	void UpdateFade(float DeltaTime);
	void UpdateFloating(float DeltaTime);
	void UpdateRotation(float DeltaTime);
	void CreateDynamicMaterials();
	void UpdateMaterialOpacity(float Opacity);
	void StartFadeOut();
};