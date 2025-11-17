#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "PaTrolPath.h"
#include "GameFramework/Character.h"
#include "Animation/AnimMontage.h"
#include "BehaviorTree/BehaviorTree.h"
#include "NPC.generated.h"

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

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

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
};
