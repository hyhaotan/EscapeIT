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

    // === COMPONENTS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    class UStaticMeshComponent* GhostMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    class USkeletalMeshComponent* GhostSkeletalMesh;

    // === APPEARANCE SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float FadeInDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bLookAtPlayer = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float DisappearAfterTime = 5.0f; // -1 = never disappear

    // === MATERIAL SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    FName OpacityParameterName = "Opacity";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    FName EmissiveStrengthParameterName = "EmissiveStrength";

    // === JUMPSCARE SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumpscare")
    bool bEnableJumpscare = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumpscare", meta = (EditCondition = "bEnableJumpscare"))
    float JumpscareMinDelay = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumpscare", meta = (EditCondition = "bEnableJumpscare"))
    float JumpscareMaxDelay = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumpscare", meta = (EditCondition = "bEnableJumpscare", ClampMin = "0.0", ClampMax = "1.0"))
    float PlayerDetectionAngle = 0.7f; // Dot product threshold (0.7 ≈ 45° cone)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumpscare", meta = (EditCondition = "bEnableJumpscare"))
    float PlayerDetectionRange = 2000.0f; // Detection range in cm (20 meters)

    // === CONTROL FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Ghost Control")
    void ForceDisappear(); // Biến mất ngay lập tức

    UFUNCTION(BlueprintCallable, Category = "Ghost Control")
    void StartFadeOutNow(); // Bắt đầu fade out

    UFUNCTION(BlueprintCallable, Category = "Ghost Control")
    void PauseAllEffects(); // Tạm dừng tất cả hiệu ứng

    UFUNCTION(BlueprintCallable, Category = "Ghost Control")
    void ResumeAllEffects(); // Tiếp tục hiệu ứng

    UFUNCTION(BlueprintCallable, Category = "Ghost Control")
    bool CheckGhostSeePlayer(); // Kiểm tra người chơi có nhìn thấy ghost không

    // === BLUEPRINT EVENTS ===
    // Event này được gọi khi jumpscare trigger - implement trong Blueprint để thêm sound/effects
    UFUNCTION(BlueprintImplementableEvent, Category = "Ghost|Jumpscare")
    void OnJumpscareTriggered();

private:
    // === STATE VARIABLES ===
    float CurrentFadeValue = 0.0f;
    float FadeTimer = 0.0f;
    float LifetimeTimer = 0.0f;
    FVector InitialLocation;
    float FloatOffset = 0.0f;
    
    bool bIsFadingIn = true;
    bool bIsFadingOut = false;
    bool bIsPaused = false;
    bool bPlayerHasSeenGhost = false; // Track if player has spotted the ghost
    
    FTimerHandle DisappearTimerHandle;
    FTimerHandle JumpscareTimerHandle;

    TArray<class UMaterialInstanceDynamic*> DynamicMaterials;

    // === INTERNAL FUNCTIONS ===
    void UpdateFade(float DeltaTime);
    void UpdateRotation(float DeltaTime);
    void CreateDynamicMaterials();
    void UpdateMaterialOpacity(float Opacity);
    void StartFadeOut();
    void TriggerJumpscare(); // Hàm thực thi jumpscare
};