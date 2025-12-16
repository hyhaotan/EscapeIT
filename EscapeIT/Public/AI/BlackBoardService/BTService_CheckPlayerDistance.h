#include "BehaviorTree/BTService.h"
#include "BTService_CheckPlayerDistance.generated.h"

UCLASS()
class ESCAPEIT_API UBTService_CheckPlayerDistance : public UBTService
{
    GENERATED_BODY()

    UBTService_CheckPlayerDistance();

public:
    // ngưỡng kích hoạt jump scare (cm)
    UPROPERTY(EditAnywhere, Category = "JumpScare")
    float TriggerDistance = 200.0f;

    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};