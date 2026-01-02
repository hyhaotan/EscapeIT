#pragma once

#include "CoreMinimal.h"
#include "FlowPuzzleTypes.generated.h"

UENUM(BlueprintType)
enum class ECellType : uint8
{
	Empty       UMETA(DisplayName = "Empty"),
	StartPoint  UMETA(DisplayName = "Start Point"),
	EndPoint    UMETA(DisplayName = "End Point"),
	Path        UMETA(DisplayName = "Path")
};

UENUM(BlueprintType)
enum class EWireColor : uint8
{
	None    UMETA(DisplayName = "None"),
	Red     UMETA(DisplayName = "Red"),
	Blue    UMETA(DisplayName = "Blue"),
	Yellow  UMETA(DisplayName = "Yellow"),
	Green   UMETA(DisplayName = "Green"),
	Orange  UMETA(DisplayName = "Orange")
};

USTRUCT(BlueprintType)
struct FGridCell
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	ECellType CellType;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	EWireColor WireColor;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	int32 PathID; 
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool bIsConnected;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FIntPoint GridPosition;
	
	FGridCell()
		: CellType(ECellType::Empty)
		, WireColor(EWireColor::None)
		, PathID(-1)
		, GridPosition(FIntPoint::ZeroValue)
		, bIsConnected(false)
	{}
};

USTRUCT(BlueprintType)
struct FWirePair
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWireColor Color;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint StartPoint;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint EndPoint;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FIntPoint> CurrentPath;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsComplete;
    
	FWirePair()
		: Color(EWireColor::None)
		, StartPoint(FIntPoint::ZeroValue)
		, EndPoint(FIntPoint::ZeroValue)
		, bIsComplete(false)
	{}
};
