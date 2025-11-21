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
        UE_LOG(LogTemp, Error, TEXT("QuickbarWidget::Initialize - InventoryComponent is NULL!"));
        return;
    }

    InventoryComponent = InInventoryComp;
    FlashlightComponent = InFlashlightComp;

    // Bind to inventory updates
    if (!InventoryComponent->OnInventoryUpdated.IsAlreadyBound(this, &UQuickbarWidget::OnInventoryUpdated))
    {
        InventoryComponent->OnInventoryUpdated.AddDynamic(this, &UQuickbarWidget::OnInventoryUpdated);
        UE_LOG(LogTemp, Log, TEXT("QuickbarWidget: Bound to OnInventoryUpdated"));
    }

    ItemDataTable = InventoryComponent->ItemDataTable;

    if (FlashlightComponent)
    {
        // Bind to battery and flashlight events
        if (!FlashlightComponent->OnBatteryChanged.IsAlreadyBound(this, &UQuickbarWidget::OnBatteryChanged))
        {
            FlashlightComponent->OnBatteryChanged.AddDynamic(this, &UQuickbarWidget::OnBatteryChanged);
        }

        if (!FlashlightComponent->OnFlashlightToggled.IsAlreadyBound(this, &UQuickbarWidget::OnFlashlightToggled))
        {
            FlashlightComponent->OnFlashlightToggled.AddDynamic(this, &UQuickbarWidget::OnFlashlightToggled);
        }

        UE_LOG(LogTemp, Log, TEXT("QuickbarWidget: Bound to FlashlightComponent events"));
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
    UTextBlock* Quantity = nullptr;
    UProgressBar* ProgressBar = nullptr;
    UBorder* Border = nullptr;

    // Get widget references based on slot index
    switch (SlotIndex)
    {
    case 0:
        Icon = Slot1_Icon;
        Quantity = Slot1_Quantity;
        ProgressBar = Slot1_Battery;
        Border = Slot1_Border;
        break;
    case 1:
        Icon = Slot2_Icon;
        Quantity = Slot2_Quantity;
        ProgressBar = Slot2_Battery;
        Border = Slot2_Border;
        break;
    case 2:
        Icon = Slot3_Icon;
        Quantity = Slot3_Quantity;
        ProgressBar = Slot3_Battery;
        Border = Slot3_Border;
        break;
    case 3:
        Icon = Slot4_Icon;
        Quantity = Slot4_Quantity;
        ProgressBar = Slot4_Battery;
        Border = Slot4_Border;
        break;
    }

    // Validate widget references
    if (!Icon || !Quantity || !Border)
    {
        UE_LOG(LogTemp, Error, TEXT("UpdateSlotVisuals: Widget is NULL for slot %d"), SlotIndex);
        return;
    }

    // Empty slot
    if (!SlotData.IsValid())
    {
        Icon->SetColorAndOpacity(EmptySlotColor);
        Icon->SetBrushFromTexture(nullptr);
        Quantity->SetText(FText::GetEmpty());
        Quantity->SetVisibility(ESlateVisibility::Collapsed);

        if (ProgressBar)
        {
            ProgressBar->SetVisibility(ESlateVisibility::Collapsed);
        }

        Border->SetBrushColor(EmptySlotColor);
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
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("UpdateSlotVisuals: ItemID '%s' not found in DataTable"),
                *SlotData.ItemID.ToString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UpdateSlotVisuals: ItemDataTable is NULL"));
    }

    if (!bFoundItemData)
    {
        // Show placeholder for missing data
        Icon->SetColorAndOpacity(FLinearColor::Red);
        Quantity->SetText(FText::FromString("?"));
        Quantity->SetVisibility(ESlateVisibility::Visible);
        Border->SetBrushColor(FLinearColor::Red);
        
        if (ProgressBar)
        {
            ProgressBar->SetVisibility(ESlateVisibility::Collapsed);
        }
        return;
    }

    // Set icon
    if (ItemData.Icon)
    {
        Icon->SetBrushFromTexture(ItemData.Icon);
        Icon->SetColorAndOpacity(FLinearColor::White);
    }
    else
    {
        Icon->SetBrushFromTexture(nullptr);
        Icon->SetColorAndOpacity(FLinearColor::Gray);
    }
    
    bool bIsFlashlight = IsFlashlightItem(SlotData.ItemID);

    if (!bIsFlashlight && SlotData.Quantity > 1)
    {
        Quantity->SetText(FText::AsNumber(SlotData.Quantity));
        Quantity->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        Quantity->SetVisibility(ESlateVisibility::Collapsed);
    }

    // Update progress bar
    if (ProgressBar)
    {
        if (bIsFlashlight && FlashlightComponent)
        {
            ProgressBar->SetVisibility(ESlateVisibility::Visible);
            UpdateBatteryDisplay(SlotIndex,FlashlightComponent->GetBatteryPercentage());
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

void UQuickbarWidget::UpdateBatteryDisplay(int32 SlotIndex,float Percentage)
{
    const auto BatteryBar = GetBatteryBarForSlot(SlotIndex);

    if (!BatteryBar) return;
    
    BatteryBar->SetPercent(Percentage);
    
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
        BatteryColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); 
    }
    else
    {
        BatteryColor = LowBatteryColor;
    }
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

void UQuickbarWidget::OnBatteryChanged(float Percentage)
{
    int32 FlashlightSlotIndex = FindFlashlightSlot();
    if (FlashlightSlotIndex != -1)
    {
        UpdateBatteryDisplay(FlashlightSlotIndex, Percentage);
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
                HighlightSlot(FlashlightSlotIndex);
            }
            else if (CurrentSelectedSlot == FlashlightSlotIndex)
            {
                FlashlightBorder->SetBrushColor(NormalBorderColor);
                CurrentSelectedSlot = -1;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Flashlight toggled: %s"), bIsOn ? TEXT("ON") : TEXT("OFF"));
}

int32 UQuickbarWidget::FindFlashlightSlot()
{
    if (!InventoryComponent) return -1;
    
    for (int32 i = 0; i < 4 ;i++)
    {
        FInventorySlot SlotData = InventoryComponent->GetQuickbarSlot(i);
        if (SlotData.IsValid() && IsFlashlightItem(SlotData.ItemID))
        {
            return i;
        }
    }
    return -1;
}

bool UQuickbarWidget::IsFlashlightItem(FName ItemID)
{
    return ItemID.ToString().Contains(TEXT("Flashlight"),ESearchCase::IgnoreCase);
}

UProgressBar* UQuickbarWidget::GetBatteryBarForSlot(int32 SlotIndex)
{
    switch (SlotIndex)
    {
        case 0: return Slot1_Battery;
        case 1: return Slot2_Battery;
        case 2: return Slot3_Battery;
        case 3: return Slot4_Battery;
        default:return nullptr;
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
