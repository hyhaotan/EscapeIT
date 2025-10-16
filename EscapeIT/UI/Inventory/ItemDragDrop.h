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
	UInventorySlotWidget* SourceSlot;

	UPROPERTY()
	int32 SlotIndex;

	UPROPERTY()
	bool bIsQuickbarSlot;

	UPROPERTY()
	FName ItemID;

	UItemDragDrop()
		: SourceSlot(nullptr)
		, SlotIndex(-1)
		, bIsQuickbarSlot(false)
		, ItemID(NAME_None)
	{
	}
};
