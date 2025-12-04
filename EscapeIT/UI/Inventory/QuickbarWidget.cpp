#include "QuickbarWidget.h"
#include "EscapeIT/Actor/Components/InventoryComponent.h"
#include "EscapeIT/Actor/Components/FlashlightComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Border.h"

void UQuickbarWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Set hotkey texts
    if (Slot1_Hotkey) Slot1_Hotkey->SetText(FText::FromString("1"));
    if (Slot2_Hotkey) Slot2_Hotkey->SetText(FText::FromString("2"));
    if (Slot3_Hotkey) Slot3_Hotkey->SetText(FText::FromString("3"));
    if (Slot4_Hotkey) Slot4_Hotkey->SetText(FText::FromString("4"));
    
    if (Slot1_Battery) Slot1_Battery->SetVisibility(ESlateVisibility::Collapsed);
    if (Slot2_Battery) Slot2_Battery->SetVisibility(ESlateVisibility::Collapsed);
    if (Slot3_Battery) Slot3_Battery->SetVisibility(ESlateVisibility::Collapsed);
    if (Slot4_Battery) Slot4_Battery->SetVisibility(ESlateVisibility::Collapsed); 
    
    if (Slot1_IconBattery) Slot1_Battery->SetVisibility(ESlateVisibility::Collapsed);
    if (Slot2_IconBattery) Slot2_Battery->SetVisibility(ESlateVisibility::Collapsed);
    if (Slot3_IconBattery) Slot3_Battery->SetVisibility(ESlateVisibility::Collapsed);
    if (Slot4_IconBattery) Slot4_Battery->SetVisibility(ESlateVisibility::Collapsed);

    UE_LOG(LogTemp, Log, TEXT("QuickbarWidget: NativeConstruct called"));
}

void UQuickbarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!bIsInitialized || !FlashlightComponent)
    {
        return;
    }

    // Tìm slot nào có flashlight
    int32 FlashlightSlotIndex = FindFlashlightSlot();
    
    if (FlashlightSlotIndex != -1)
    {
        UProgressBar* BatteryBar = GetBatteryBarForSlot(FlashlightSlotIndex);
        UBorder* SlotBorder = GetBorderForSlot(FlashlightSlotIndex);
        
        if (BatteryBar)
        {
            float BatteryPercent = FlashlightComponent->GetBatteryPercentage();
            UpdateBatteryDisplay(FlashlightSlotIndex, BatteryPercent);

            // Low battery warning pulse
            if (BatteryPercent < 10.0f && FlashlightComponent->IsLightOn())
            {
                if (!bLowBatteryWarningActive)
                {
                    bLowBatteryWarningActive = true;
                }

                // Pulsing effect
                float PulseValue = FMath::Sin(GetWorld()->GetTimeSeconds() * 5.0f) * 0.5f + 0.5f;
                FLinearColor PulseColor = FMath::Lerp(LowBatteryColor, FLinearColor::Black, PulseValue);

                if (SlotBorder)
                {
                    SlotBorder->SetBrushColor(PulseColor);
                }
            }
            else if (bLowBatteryWarningActive && BatteryPercent >= 10.0f)
            {
                bLowBatteryWarningActive = false;
                if (SlotBorder)
                {
                    SlotBorder->SetBrushColor(NormalBorderColor);
                }
            }
        }
    }
}

void UQuickbarWidget::InitQuickBar(UInventoryComponent* InInventoryComp, UFlashlightComponent* InFlashlightComp)
{
    if (!InInventoryComp)
    {
        UE_LOG(LogTemp, Error, TEXT("QuickbarWidget::InitQuickBar - InventoryComponent is NULL!"));
        return;
    }

    InventoryComponent = InInventoryComp;
    FlashlightComponent = InFlashlightComp;
    ItemDataTable = InventoryComponent->ItemDataTable;

    // Bind to inventory updates
    if (!InventoryComponent->OnInventoryUpdated.IsAlreadyBound(this, &UQuickbarWidget::OnInventoryUpdated))
    {
        InventoryComponent->OnInventoryUpdated.AddDynamic(this, &UQuickbarWidget::OnInventoryUpdated);
        UE_LOG(LogTemp, Log, TEXT("QuickbarWidget: Bound to OnInventoryUpdated"));
    }

    if (!InventoryComponent->OnItemAdded.IsAlreadyBound(this, &UQuickbarWidget::OnItemAdded))
    {
        InventoryComponent->OnItemAdded.AddDynamic(this, &UQuickbarWidget::OnItemAdded);
        UE_LOG(LogTemp, Log, TEXT("✅ QuickbarWidget: Bound to OnItemAdded"));
    }

    if (FlashlightComponent)
    {
        if (!FlashlightComponent->OnBatteryChanged.IsAlreadyBound(this, &UQuickbarWidget::OnBatteryChanged))
        {
            FlashlightComponent->OnBatteryChanged.AddDynamic(this, &UQuickbarWidget::OnBatteryChanged);
            UE_LOG(LogTemp, Log, TEXT("QuickbarWidget: Bound to OnBatteryChanged"));
        }

        if (!FlashlightComponent->OnFlashlightToggled.IsAlreadyBound(this, &UQuickbarWidget::OnFlashlightToggled))
        {
            FlashlightComponent->OnFlashlightToggled.AddDynamic(this, &UQuickbarWidget::OnFlashlightToggled);
            UE_LOG(LogTemp, Log, TEXT("QuickbarWidget: Bound to OnFlashlightToggled"));
        }

        UE_LOG(LogTemp, Log, TEXT("QuickbarWidget: FlashlightComponent bound successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("QuickbarWidget: FlashlightComponent is NULL!"));
    }

    bIsInitialized = true;

    // Initial refresh
    RefreshQuickbar();

    UE_LOG(LogTemp, Log, TEXT("QuickbarWidget initialized successfully"));
}

void UQuickbarWidget::RefreshQuickbar()
{
    if (!InventoryComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("RefreshQuickbar: InventoryComponent is null!"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("RefreshQuickbar: Updating all slots"));

    for (int32 i = 0; i < 4; i++)
    {
        UpdateSlot(i);
    }
}

void UQuickbarWidget::UpdateSlot(int32 SlotIndex)
{
    if (!InventoryComponent || SlotIndex < 0 || SlotIndex >= 4)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateSlot: Invalid params - Comp=%s, Index=%d"),
            InventoryComponent ? TEXT("Valid") : TEXT("NULL"), SlotIndex);
        return;
    }

    FInventorySlot SlotData = InventoryComponent->GetQuickbarSlot(SlotIndex);

    // Debug log
    if (SlotData.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("UpdateSlot %d: Item=%s, Quantity=%d"),
            SlotIndex, *SlotData.ItemID.ToString(), SlotData.Quantity);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("UpdateSlot %d: EMPTY SLOT"), SlotIndex);
    }

    UpdateSlotVisuals(SlotIndex, SlotData);
}

void UQuickbarWidget::UpdateSlotVisuals(int32 SlotIndex, const FInventorySlot& SlotData)
{
    UImage* Icon = nullptr;
    UImage* IconBattery = nullptr;
    UTextBlock* Quantity = nullptr;
    UProgressBar* ProgressBar = nullptr;
    UBorder* Border = nullptr;

    // Get widget references
    switch (SlotIndex)
    {
    case 0:
        Icon = Slot1_Icon;
        IconBattery = Slot1_IconBattery;
        Quantity = Slot1_Quantity;
        ProgressBar = Slot1_Battery;
        Border = Slot1_Border;
        break;
    case 1:
        Icon = Slot2_Icon;
        IconBattery = Slot2_IconBattery;
        Quantity = Slot2_Quantity;
        ProgressBar = Slot2_Battery;
        Border = Slot2_Border;
        break;
    case 2:
        Icon = Slot3_Icon;
        IconBattery = Slot3_IconBattery;
        Quantity = Slot3_Quantity;
        ProgressBar = Slot3_Battery;
        Border = Slot3_Border;
        break;
    case 3:
        Icon = Slot4_Icon;
        IconBattery = Slot4_IconBattery;
        Quantity = Slot4_Quantity;
        ProgressBar = Slot4_Battery;
        Border = Slot4_Border;
        break;
    }

    // ✅ KIỂM TRA WIDGET CÓ TỒN TẠI KHÔNG
    if (!Icon || !Quantity || !Border || !IconBattery)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ UpdateSlotVisuals: Widget NULL for slot %d"), SlotIndex);
        return;
    }

    // Empty slot
    if (!SlotData.IsValid())
    {
        Icon->SetColorAndOpacity(EmptySlotColor);
        Icon->SetBrushFromTexture(nullptr);
        Quantity->SetText(FText::GetEmpty());
        Quantity->SetVisibility(ESlateVisibility::Collapsed);

        if (ProgressBar) ProgressBar->SetVisibility(ESlateVisibility::Collapsed);
        
        if (IconBattery) IconBattery->SetVisibility(ESlateVisibility::Collapsed);

        Border->SetBrushColor(EmptySlotColor);
        
        UE_LOG(LogTemp, Verbose, TEXT("  Slot %d: EMPTY"), SlotIndex);
        return;
    }

    // Get item data
    FItemData ItemData;
    bool bFoundItemData = false;

    if (ItemDataTable)
    {
        FItemData* Data = ItemDataTable->FindRow<FItemData>(SlotData.ItemID, TEXT("UpdateSlotVisuals"));
        if (Data)
        {
            ItemData = *Data;
            bFoundItemData = true;
            
            UE_LOG(LogTemp, Log, TEXT("✅ Slot %d: Found item data for '%s'"), 
                SlotIndex, *ItemData.ItemName.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ Slot %d: ItemID '%s' NOT FOUND in DataTable"),
                SlotIndex, *SlotData.ItemID.ToString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ UpdateSlotVisuals: ItemDataTable is NULL!"));
    }

    if (!bFoundItemData)
    {
        // Show error placeholder
        Icon->SetColorAndOpacity(FLinearColor::Red);
        Icon->SetBrushFromTexture(nullptr);
        Quantity->SetText(FText::FromString("?"));
        Quantity->SetVisibility(ESlateVisibility::Visible);
        Border->SetBrushColor(FLinearColor::Red);
        
        if (ProgressBar)
        {
            ProgressBar->SetVisibility(ESlateVisibility::Collapsed);
        }
        return;
    }

    // ✅ SET ICON
    if (ItemData.Icon)
    {
        Icon->SetBrushFromTexture(ItemData.Icon);
        Icon->SetColorAndOpacity(FLinearColor::White);
        
        UE_LOG(LogTemp, Log, TEXT("  ✅ Icon set for '%s'"), *ItemData.ItemName.ToString());
    }
    else
    {
        Icon->SetBrushFromTexture(nullptr);
        Icon->SetColorAndOpacity(FLinearColor::Gray);
        
        UE_LOG(LogTemp, Warning, TEXT("  ⚠ Item '%s' has NO icon texture!"), 
            *ItemData.ItemName.ToString());
    }
    
    // Set quantity
    bool bIsFlashlight = IsFlashlightItem(ItemData);
    
    if (!bIsFlashlight && SlotData.Quantity > 1)
    {
        Quantity->SetText(FText::AsNumber(SlotData.Quantity));
        Quantity->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        Quantity->SetVisibility(ESlateVisibility::Collapsed);
    }
    
    // Battery bar logic (chỉ cho flashlight)
    if (ProgressBar)
    {
        if (bIsFlashlight && FlashlightComponent)
        {
            ProgressBar->SetVisibility(ESlateVisibility::Visible);
            float BatteryPercent = FlashlightComponent->GetBatteryPercentage();
            UpdateBatteryDisplay(SlotIndex, BatteryPercent);
        }
        else
        {
            ProgressBar->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    // Set border color
    if (!bLowBatteryWarningActive || !bIsFlashlight)
    {
        Border->SetBrushColor(NormalBorderColor);
    }
}

void UQuickbarWidget::UpdateBatteryDisplay(int32 SlotIndex, float Percentage)
{
    UProgressBar* BatteryBar = GetBatteryBarForSlot(SlotIndex);

    if (!BatteryBar)
    {
        return;
    }
    
    // Convert percentage (0-100) to fill percent (0.0-1.0)
    float FillPercent = Percentage / 100.0f;
    BatteryBar->SetPercent(FillPercent);
    
    // Calculate color
    FLinearColor BatteryColor;
    
    if (Percentage > 50.0f)
    {
        BatteryColor = FLinearColor::Green;
    }
    else if (Percentage > 25.0f)
    {
        BatteryColor = FLinearColor::Yellow;
    }
    else if (Percentage > 10.0f)
    {
        BatteryColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
    }
    else
    {
        BatteryColor = LowBatteryColor; // Red
    }

    // Set color
    BatteryBar->SetFillColorAndOpacity(BatteryColor);
}

void UQuickbarWidget::HighlightSlot(int32 SlotIndex)
{
    // Reset previous selection
    if (CurrentSelectedSlot >= 0 && CurrentSelectedSlot < 4)
    {
        UBorder* PrevBorder = GetBorderForSlot(CurrentSelectedSlot);
        if (PrevBorder && !bLowBatteryWarningActive)
        {
            PrevBorder->SetBrushColor(NormalBorderColor);
        }
    }

    // Highlight new slot
    CurrentSelectedSlot = SlotIndex;
    UBorder* NewBorder = GetBorderForSlot(SlotIndex);
    
    if (NewBorder)
    {
        NewBorder->SetBrushColor(SelectedBorderColor);
    }

    UE_LOG(LogTemp, Log, TEXT("Quickbar: Highlighted slot %d"), SlotIndex);
}

void UQuickbarWidget::OnInventoryUpdated()
{
    UE_LOG(LogTemp, Log, TEXT("QuickbarWidget: OnInventoryUpdated called - Refreshing all slots"));
    RefreshQuickbar();
}

void UQuickbarWidget::OnItemAdded(FName ItemID, int32 Quantity)
{
    UE_LOG(LogTemp, Log, TEXT("🎯 QuickbarWidget: Item added - %s x%d"), 
        *ItemID.ToString(), Quantity);
    
    RefreshQuickbar();
}

void UQuickbarWidget::OnBatteryChanged(float CurrentBattery, float MaxBattery)
{
    // Calculate percentage
    float Percentage = (MaxBattery > 0.0f) ? (CurrentBattery / MaxBattery) * 100.0f : 0.0f;

    // Update battery display for flashlight slot ONLY
    int32 FlashlightSlotIndex = FindFlashlightSlot();
    if (FlashlightSlotIndex != -1)
    {
        UpdateBatteryDisplay(FlashlightSlotIndex, Percentage);
        
        UE_LOG(LogTemp, Log, TEXT("Battery updated: %.1f%% (%.1f/%.1fs)"), 
            Percentage, CurrentBattery, MaxBattery);
    }
}

void UQuickbarWidget::OnFlashlightToggled(bool bIsOn)
{
    // Tìm slot có flashlight
    int32 FlashlightSlotIndex = FindFlashlightSlot();
    
    if (FlashlightSlotIndex != -1)
    {
        UBorder* FlashlightBorder = GetBorderForSlot(FlashlightSlotIndex);
        
        if (FlashlightBorder)
        {
            if (bIsOn)
            {
                // Highlight slot khi bật đèn
                HighlightSlot(FlashlightSlotIndex);
            }
            else if (CurrentSelectedSlot == FlashlightSlotIndex)
            {
                // Reset border khi tắt đèn
                FlashlightBorder->SetBrushColor(NormalBorderColor);
                CurrentSelectedSlot = -1;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Flashlight toggled: %s"), bIsOn ? TEXT("ON") : TEXT("OFF"));
}

int32 UQuickbarWidget::FindFlashlightSlot()
{
    if (!InventoryComponent || !ItemDataTable)
    {
        return -1;
    }
    
    for (int32 i = 0; i < 4; i++)
    {
        FInventorySlot SlotData = InventoryComponent->GetQuickbarSlot(i);
        
        if (!SlotData.IsValid())
        {
            continue;
        }

        // Get item data để check category
        FItemData* ItemData = ItemDataTable->FindRow<FItemData>(SlotData.ItemID, TEXT("FindFlashlightSlot"));
        if (ItemData && IsFlashlightItem(*ItemData))
        {
            return i;
        }
    }
    
    return -1;
}

bool UQuickbarWidget::IsFlashlightItem(const FItemData& ItemData)
{
    if (ItemData.ItemType == EItemType::Tool)
    {
        if (ItemData.ToolType == EToolType::Flashlight)
        {
            return ItemData.ToolType == EToolType::Flashlight;
        }
    }
    return false;
}

UProgressBar* UQuickbarWidget::GetBatteryBarForSlot(int32 SlotIndex)
{
    switch (SlotIndex)
    {
        case 0: return Slot1_Battery;
        case 1: return Slot2_Battery;
        case 2: return Slot3_Battery;
        case 3: return Slot4_Battery;
        default: return nullptr;
    }
}

UBorder* UQuickbarWidget::GetBorderForSlot(int32 SlotIndex)
{
    switch (SlotIndex)
    {
        case 0: return Slot1_Border;
        case 1: return Slot2_Border;
        case 2: return Slot3_Border;
        case 3: return Slot4_Border;
        default: return nullptr;
    }
}

UImage* UQuickbarWidget::GetIconBatteryForSlot(int32 SlotIndex)
{
    switch (SlotIndex)
    {
    case 0: return Slot1_IconBattery;
    case 1: return Slot2_IconBattery;
    case 2: return Slot3_IconBattery;
    case 3: return Slot4_IconBattery;
        default: return nullptr;
    }
}
