// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "ItemDragDrop.generated.h"

class UInventorySlotWidget;

UCLASS()
class ESCAPEIT_API UItemDragDrop : public UDragDropOperation
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TObjectPtr<UInventorySlotWidget> SourceSlot;

	UPROPERTY()
	int32 SlotIndex;

	UPROPERTY()
	bool bIsQuickbarSlot;

	UPROPERTY()
	FName ItemID;
	
	UPROPERTY()
	int32 Quantity;

	UItemDragDrop()
		: SourceSlot(nullptr)
		, SlotIndex(-1)
		, bIsQuickbarSlot(false)
		, Quantity(1)
		, ItemID(NAME_None)
	{
	}
};
