#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "PaTrolPath.h"
#include "GameFramework/Character.h"
#include "Animation/AnimMontage.h"
#include "BehaviorTree/BehaviorTree.h"
#include "NPC.generated.h"

UENUM(BlueprintType)
enum class ESearchPatternType : uint8
{
    Random      UMETA(DisplayName = "Random Search"),
    Spiral      UMETA(DisplayName = "Spiral Search"),
    Grid        UMETA(DisplayName = "Grid Search"),
    LastKnown   UMETA(DisplayName = "Last Known Position")
};

UCLASS()
class ESCAPEIT_API ANPC : public ACharacter
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    ANPC();

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Lấy BehaviorTree, PatrolPath, v.v... nếu bạn cần
    UBehaviorTree* GetBehaviorTree() const;
    APaTrolPath* GetPatrolPath() const;
    UAnimMontage* GetMontage() const;

    /** Custom function to toggle visibility */
    UFUNCTION(BlueprintCallable, Category = "AI")
    void ToggleInvestigationWidgetVisibility(bool bCanSeePlayer);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpScare")
    UCameraComponent* JumpScareCamera;

    UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "JumpScare")
    bool bUseJumpScareCamera = false;

    // Search Pattern Implementation Variables
    FVector SearchCenter;
    TArray<FVector> SearchPoints;
    int32 CurrentSearchIndex;
    float CurrentSpiralAngle;
    float CurrentSpiralRadius;
    bool bIsSearching;

    UPROPERTY(BlueprintReadWrite, Category = "AI|Search")
    FVector LastKnownPlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Search")
    int32 MaxSearchPoints = 8;

    // Search Pattern Functions
    UFUNCTION(BlueprintCallable, Category = "AI|Search")
    FVector GetNextSearchLocation();

    UFUNCTION(BlueprintCallable, Category = "AI|Search")
    void StartSearchPattern(const FVector& CenterLocation);

    UFUNCTION(BlueprintCallable, Category = "AI|Search")
    void StopSearchPattern();

    UFUNCTION(BlueprintCallable, Category = "AI|Search")
    void SetIsSearching(bool bSearching);

    UFUNCTION(BlueprintCallable, Category = "AI|Search")
    bool IsSearching() const;

    UPROPERTY(BlueprintReadWrite, Replicated)
    class AEscapeITCharacter* JumpScareTargetPlayer;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
	class UWidgetComponent* InvestigationWidgetComponent;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Override để cho phép NPC trả về góc nhìn từ JumpScareCamera
    virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;

    // Search Pattern Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Search")
    ESearchPatternType SearchPattern;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Search")
    float SearchRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Search")
    float GridCellSize = 200.0f;

    bool bIsWidgetVisible;

    FTimerHandle JumpScareTimerHandle;

    // Cache values for cleanup
    float CachedBlendTime;

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Replicated, Category = "JumpScare", meta = (AllowPrivateAccess))
    bool bIsPerformingJumpScare = false;
private:
    // Behavior Tree gán trong Editor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
    UBehaviorTree* Tree;

    // Patrol Path gán trong Editor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
    APaTrolPath* PatrolPath;

    // AnimMontage nếu bạn muốn NPC dùng
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, meta = (AllowPrivateAccess = "true"))
    UAnimMontage* Montage;

    // Widget investigation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
    UUserWidget* InvestigateWidget;

    // Helper functions for search patterns
    void GenerateRandomSearchPoints();
    void GenerateSpiralSearchPoints();
    void GenerateGridSearchPoints();
    FVector GetNextRandomPoint() const;
    FVector GetNextSpiralPoint();
    FVector GetNextGridPoint() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;
};
