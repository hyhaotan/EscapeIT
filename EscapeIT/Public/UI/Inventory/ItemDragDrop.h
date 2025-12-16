// ItemDragDrop.h - Drag & Drop Operation for Inventory

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Data/ItemData.h"
#include "ItemDragDrop.generated.h"

/**
 * Drag & Drop operation for inventory items
 * Contains information about the dragged item and its source
 */
UCLASS()
class ESCAPEIT_API UItemDragDrop : public UDragDropOperation
{
	GENERATED_BODY()

public:
	// ========================================================================
	// DRAG SOURCE INFO
	// ========================================================================
    
	/** Index of the source slot (either inventory or quickbar) */
	UPROPERTY(BlueprintReadWrite, Category = "DragDrop")
	int32 SourceSlotIndex = -1;

	/** Is the item being dragged from quickbar? */
	UPROPERTY(BlueprintReadWrite, Category = "DragDrop")
	bool bIsFromQuickbar = false;

	// ========================================================================
	// ITEM DATA
	// ========================================================================
    
	/** The inventory slot being dragged */
	UPROPERTY(BlueprintReadWrite, Category = "DragDrop")
	FInventorySlot DraggedItemData;

	/** Visual widget to display during drag */
	UPROPERTY(BlueprintReadWrite, Category = "DragDrop")
	TObjectPtr<UUserWidget> DraggedVisualWidget;
};