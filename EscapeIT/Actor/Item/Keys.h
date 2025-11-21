// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EscapeIT/Actor/ItemPickupActor.h"
#include "Keys.generated.h"

/**
 * Key types that match with chest lock types
 */
UENUM(BlueprintType)
enum class EKeyType : uint8
{
	RustyKey        UMETA(DisplayName = "Rusty Key"),
	GoldenKey       UMETA(DisplayName = "Golden Key"),
	SilverKey       UMETA(DisplayName = "Silver Key"),
	MasterKey       UMETA(DisplayName = "Master Key"),
	SpecialKey      UMETA(DisplayName = "Special Key"),
	RedKey          UMETA(DisplayName = "Red Key"),
	BlueKey         UMETA(DisplayName = "Blue Key"),
	GreenKey        UMETA(DisplayName = "Green Key")
};

UCLASS()
class ESCAPEIT_API AKeys : public AItemPickupActor
{
	GENERATED_BODY()
    
public: 
	AKeys();

	// ============================================
	// KEY PROPERTIES
	// ============================================
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key Settings")
	EKeyType KeyType;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key Settings")
	FName KeyID;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key Settings")
	bool bIsSingleUse;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key Settings")
	int32 MaxUses;

	// ============================================
	// KEY FUNCTIONS
	// ============================================
    
	UFUNCTION(BlueprintCallable, Category = "Key")
	EKeyType GetKeyType() const { return KeyType; }
    
	UFUNCTION(BlueprintCallable, Category = "Key")
	FName GetKeyID() const { return KeyID; }
    
	UFUNCTION(BlueprintCallable, Category = "Key")
	bool CanBeUsed() const;

protected:
	virtual void BeginPlay() override;

private:
	int32 CurrentUses;
};