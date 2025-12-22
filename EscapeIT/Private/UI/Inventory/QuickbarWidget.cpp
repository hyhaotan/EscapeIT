
#include "UI/Inventory/QuickbarWidget.h"
#include "Actor/Components/InventoryComponent.h"
#include "Actor/Components/FlashlightComponent.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "UI/Inventory/InventorySlotWidget.h"

void UQuickbarWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Initialize UI elements to hidden state
    if (BatteryBar)
    {
        BatteryBar->SetVisibility(ESlateVisibility::Collapsed);
        BatteryBar->SetPercent(1.0f);
    }

    if (BatteryIcon)
    {
        BatteryIcon->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (BatteryTextPercent)
    {
        BatteryTextPercent->SetVisibility(ESlateVisibility::Collapsed);
    }
    
    InventorySlotWidget = CreateWidget<UInventorySlotWidget>(GetWorld(),SlotWidgetClass);
}

void UQuickbarWidget::NativeDestruct()
{
    UnbindAllEvents();
    Super::NativeDestruct();
}

void UQuickbarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!bIsInitialized || !FlashlightComponent) return;

    if (!FlashlightComponent->IsEquipped()) return;

    // Throttle battery checks for performance
    BatteryCheckTimer += InDeltaTime;
    if (BatteryCheckTimer >= BATTERY_CHECK_INTERVAL)
    {
        BatteryCheckTimer = 0.0f;
        
        // Cache flashlight slot to avoid repeated searches
        if (CachedFlashlightSlot == -1)
        {
            CachedFlashlightSlot = FindFlashlightSlot();
        }

        if (CachedFlashlightSlot != -1)
        {
            UpdateBatteryBar();
            UpdateLowBatteryWarning(InDeltaTime);
        }
    }
}

void UQuickbarWidget::InitQuickBar(UInventoryComponent* InInventoryComp, UFlashlightComponent* InFlashlightComp)
{
    // Validate required components
    if (!InInventoryComp) return;
    if (!QuickbarContainer) return;
    if (!SlotWidgetClass) return;

    // Prevent double initialization
    if (bIsInitialized) UnbindAllEvents();

    InventoryComponent = InInventoryComp;
    FlashlightComponent = InFlashlightComp;

    CreateQuickbarSlots();

    // Bind inventory events with duplicate check
    if (InventoryComponent)
    {
        if (!InventoryComponent->OnInventoryUpdated.IsAlreadyBound(this, &UQuickbarWidget::OnInventoryUpdated))
        {
            InventoryComponent->OnInventoryUpdated.AddDynamic(this, &UQuickbarWidget::OnInventoryUpdated);
        }

        if (!InventoryComponent->OnItemAdded.IsAlreadyBound(this, &UQuickbarWidget::OnItemAdded))
        {
            InventoryComponent->OnItemAdded.AddDynamic(this, &UQuickbarWidget::OnItemAdded);
        }
    }

    // Bind flashlight events
    if (FlashlightComponent)
    {
        if (!FlashlightComponent->OnBatteryChanged.IsAlreadyBound(this, &UQuickbarWidget::OnBatteryChanged))
        {
            FlashlightComponent->OnBatteryChanged.AddDynamic(this, &UQuickbarWidget::OnBatteryChanged);
        }

        if (!FlashlightComponent->OnFlashlightToggled.IsAlreadyBound(this, &UQuickbarWidget::OnFlashlightToggled))
        {
            FlashlightComponent->OnFlashlightToggled.AddDynamic(this, &UQuickbarWidget::OnFlashlightToggled);
        }
        
        
        if (!FlashlightComponent->OnFlashlightStateChanged.IsAlreadyBound(this, &UQuickbarWidget::OnFlashlightStateChanged))
        {
            FlashlightComponent->OnFlashlightStateChanged.AddDynamic(this, &UQuickbarWidget::OnFlashlightStateChanged);
        }
        
        if (!FlashlightComponent->OnBatteryLow.IsAlreadyBound(this,&UQuickbarWidget::OnBatteryLow))
        {
            FlashlightComponent->OnBatteryLow.AddDynamic(this,&UQuickbarWidget::OnBatteryLow);
        }   
        
        if (!FlashlightComponent->OnBatteryDepleted.IsAlreadyBound(this,&UQuickbarWidget::OnBatteryDepleted))
        {
            FlashlightComponent->OnBatteryDepleted.AddDynamic(this,&UQuickbarWidget::OnBatteryDepleted);
        }   
    }

    bIsInitialized = true;
    RefreshQuickbar();
}

void UQuickbarWidget::UnbindAllEvents()
{
    if (InventoryComponent)
    {
        InventoryComponent->OnInventoryUpdated.RemoveDynamic(this, &UQuickbarWidget::OnInventoryUpdated);
        InventoryComponent->OnItemAdded.RemoveDynamic(this, &UQuickbarWidget::OnItemAdded);
    }

    if (FlashlightComponent)
    {
        FlashlightComponent->OnBatteryChanged.RemoveDynamic(this, &UQuickbarWidget::OnBatteryChanged);
        FlashlightComponent->OnFlashlightToggled.RemoveDynamic(this, &UQuickbarWidget::OnFlashlightToggled);
        FlashlightComponent->OnFlashlightImageChanged.RemoveDynamic(this, &UQuickbarWidget::UpdateFlashlightIcon);
        
        FlashlightComponent->OnFlashlightStateChanged.RemoveDynamic(this, &UQuickbarWidget::OnFlashlightStateChanged);
        
        FlashlightComponent->OnBatteryLow.RemoveDynamic(this, &UQuickbarWidget::OnBatteryLow);
        FlashlightComponent->OnBatteryDepleted.RemoveDynamic(this, &UQuickbarWidget::OnBatteryDepleted);
    }

    bIsInitialized = false;
}

void UQuickbarWidget::CreateQuickbarSlots()
{
    if (!QuickbarContainer || !SlotWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ CreateQuickbarSlots: Missing required components!"));
        return;
    }

    // Clear existing slots
    QuickbarSlots.Empty();
    QuickbarContainer->ClearChildren();

    // Create quickbar slots based on configuration
    for (int32 i = 0; i < QuickbarSlotCount; i++)
    {
        UInventorySlotWidget* NewSlot = CreateWidget<UInventorySlotWidget>(this, SlotWidgetClass);
        
        if (!NewSlot)
        {
            UE_LOG(LogTemp, Error, TEXT("❌ Failed to create slot widget %d"), i);
            continue;
        }

        // Configure slot
        NewSlot->SlotIndex = i;
        NewSlot->bIsQuickbarSlot = true;
        NewSlot->ParentInventoryWidget = nullptr;
        NewSlot->InventoryComponentRef = InventoryComponent;

        // Set hotkey display
        if (NewSlot->HotkeyText)
        {
            NewSlot->HotkeyText->SetText(FText::AsNumber(i + 1));
            NewSlot->HotkeyText->SetVisibility(ESlateVisibility::HitTestInvisible);
        }

        // Add to container with proper layout
        UHorizontalBoxSlot* BoxSlot = QuickbarContainer->AddChildToHorizontalBox(NewSlot);
        if (BoxSlot)
        {
            BoxSlot->SetPadding(FMargin(5.0f, 0.0f));
            BoxSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
            BoxSlot->SetHorizontalAlignment(HAlign_Fill);
            BoxSlot->SetVerticalAlignment(VAlign_Fill);
        }

        QuickbarSlots.Add(NewSlot);
    }

    UE_LOG(LogTemp, Log, TEXT("✅ Created %d quickbar slots"), QuickbarSlots.Num());
}

void UQuickbarWidget::RefreshQuickbar()
{
    if (!InventoryComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("⚠️ RefreshQuickbar called with null InventoryComponent"));
        return;
    }

    // Update all slots
    for (int32 i = 0; i < QuickbarSlots.Num(); i++)
    {
        UpdateSlot(i);
    }
    
    // Reset cached flashlight slot to force re-search
    CachedFlashlightSlot = -1;
    UpdateBatteryBarVisibility();
}

void UQuickbarWidget::UpdateSlot(int32 SlotIndex)
{
    if (!InventoryComponent || !QuickbarSlots.IsValidIndex(SlotIndex)) return;

    FInventorySlot SlotData = InventoryComponent->GetQuickbarSlot(SlotIndex);
    UInventorySlotWidget* SlotWidget = QuickbarSlots[SlotIndex];
    
    if (!SlotWidget) return;

    SlotWidget->UpdateSlot(SlotData);

    // Update equipped state
    bool bIsEquipped = (InventoryComponent->CurrentEquippedSlotIndex == SlotIndex);
    SlotWidget->SetEquipped(bIsEquipped);
}

void UQuickbarWidget::HighlightSlot(int32 SlotIndex)
{
    // Validate slot index
    if (!QuickbarSlots.IsValidIndex(SlotIndex)) return;

    // Clear previous highlight
    ClearHighlight();

    // Set new highlight
    CurrentSelectedSlot = SlotIndex;
    UInventorySlotWidget* NewSlot = QuickbarSlots[SlotIndex];
    
    if (NewSlot)
    {
        NewSlot->SetSelected(true);
    }
}

void UQuickbarWidget::ClearHighlight()
{
    if (CurrentSelectedSlot >= 0 && QuickbarSlots.IsValidIndex(CurrentSelectedSlot))
    {
        UInventorySlotWidget* PrevSlot = QuickbarSlots[CurrentSelectedSlot];
        if (PrevSlot && !bLowBatteryWarningActive)
        {
            PrevSlot->SetSelected(false);
        }
    }
    
    CurrentSelectedSlot = -1;
}

void UQuickbarWidget::OnInventoryUpdated()
{
    if (!bIsInitialized)
    {
        return;
    }
    
    //Invalidate cached flashlight slot
    CachedFlashlightSlot = -1;
    
    RefreshQuickbar();
}

void UQuickbarWidget::OnItemAdded(FName ItemID, int32 Quantity)
{
    if (!bIsInitialized)
    {
        return;
    }
    
    RefreshQuickbar();
    
    if (IsFlashlightItem(ItemID))
    {
        if (FlashlightComponent && FlashlightComponent->IsEquipped())
        {
            CachedFlashlightSlot = -1;
            UpdateBatteryBarVisibility();
            UE_LOG(LogTemp, Log, TEXT("Flashlight added - Battery bar updated"));
        }
    }
}

void UQuickbarWidget::OnBatteryChanged(float CurrentBattery, float MaxBattery)
{
    if (FlashlightComponent && FlashlightComponent->IsEquipped()) UpdateBatteryBar();
}

void UQuickbarWidget::OnFlashlightToggled(bool bIsOn)
{
    int32 FlashlightSlotIndex = FindFlashlightSlot();
    
    if (FlashlightSlotIndex == -1 || !QuickbarSlots.IsValidIndex(FlashlightSlotIndex))
    {
        return;
    }

    if (bIsOn)
    {
        HighlightSlot(FlashlightSlotIndex);
    }
    else if (CurrentSelectedSlot == FlashlightSlotIndex)
    {
        ResetSlotVisuals(FlashlightSlotIndex);
        CurrentSelectedSlot = -1;
    }
}

void UQuickbarWidget::OnFlashlightStateChanged(EFlashlightState NewState)
{
    // Update UI based on flashlight state
    switch (NewState)
    {
    case EFlashlightState::Unequipped:
        // Hide flashlight UI elements
        if (BatteryBar)
        {
            BatteryBar->SetVisibility(ESlateVisibility::Collapsed);
        }
        if (BatteryIcon)
        {
            BatteryIcon->SetVisibility(ESlateVisibility::Collapsed);
        }
        break;

    case EFlashlightState::Equipping:
        // Show "Equipping..." or play animation
        break;

    case EFlashlightState::Equipped:
        // Show flashlight UI elements
        if (BatteryBar)
        {
            BatteryBar->SetVisibility(ESlateVisibility::Visible);
        }
        if (BatteryIcon)
        {
            BatteryIcon->SetVisibility(ESlateVisibility::Visible);
        }
        
        // Update battery display
        if (FlashlightComponent)
        {
            OnBatteryChanged(
                FlashlightComponent->GetCurrentBattery(),
                FlashlightComponent->GetMaxBatteryDuration()
            );
        }
        break;

    case EFlashlightState::Unequipping:
        // Show "Unequipping..." or play animation
        break;
    }

    UE_LOG(LogTemp, Log, TEXT("QuickbarWidget: Flashlight state changed to %d"), (int32)NewState);
}

void UQuickbarWidget::OnBatteryLow()
{
    // Show low battery warning in UI
    if (BatteryWarningText)
    {
        BatteryWarningText->SetVisibility(ESlateVisibility::Visible);
        BatteryWarningText->SetColorAndOpacity(FLinearColor::Yellow);
        BatteryWarningText->SetText(FText::FromString(TEXT("Low Battery")));
    }
    
    // Optional: Play warning animation
    if (BatteryWarningAnimation)
    {
        PlayAnimation(BatteryWarningAnimation, 0.0f, 0); // Loop
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QuickbarWidget: Battery LOW"));
}

void UQuickbarWidget::OnBatteryDepleted()
{
    // Show battery depleted warning
    if (BatteryWarningText)
    {
        BatteryWarningText->SetVisibility(ESlateVisibility::Visible);
        BatteryWarningText->SetColorAndOpacity(FLinearColor::Red);
        BatteryWarningText->SetText(FText::FromString(TEXT("✖ Battery Depleted")));
    }
    
    // Stop warning animation, show critical state
    if (BatteryWarningAnimation)
    {
        StopAnimation(BatteryWarningAnimation);
    }
    
    UE_LOG(LogTemp, Error, TEXT("QuickbarWidget: Battery DEPLETED"));
}

void UQuickbarWidget::UpdateBatteryBarVisibility()
{
    if (!FlashlightComponent)
    {
        HideBatteryBar();
        return;
    }

    // Check multiple conditions
    bool bIsEquipped = FlashlightComponent->IsEquipped();
    int32 FlashlightSlotIndex = FindFlashlightSlot();
    bool bHasFlashlightInQuickbar = (FlashlightSlotIndex != -1);
    
    // Update cached slot
    CachedFlashlightSlot = FlashlightSlotIndex;
    
    // Show battery bar only if:
    // 1. Flashlight is in quickbar
    // 2. Flashlight is equipped
    bool bShouldShow = bHasFlashlightInQuickbar && bIsEquipped;
    
    if (bShouldShow)
    {
        ShowBatteryBar();
        UpdateBatteryBar();
    }
    else
    {
        HideBatteryBar();
        bLowBatteryWarningActive = false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("🔋 Battery Visibility: %s (QB:%s | Equip:%s | Slot:%d)"), 
        bShouldShow ? TEXT("SHOW") : TEXT("HIDE"),
        bHasFlashlightInQuickbar ? TEXT("Y") : TEXT("N"),
        bIsEquipped ? TEXT("Y") : TEXT("N"),
        FlashlightSlotIndex);
}

void UQuickbarWidget::ShowBatteryBar()
{
    if (BatteryBar)
    {
        BatteryBar->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    
    if (BatteryIcon)
    {
        BatteryIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
    } 
    
    if (BatteryTextPercent)
    {
        BatteryTextPercent->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
}

void UQuickbarWidget::HideBatteryBar()
{
    if (BatteryBar)
    {
        BatteryBar->SetVisibility(ESlateVisibility::Collapsed);
    }
    
    if (BatteryIcon)
    {
        BatteryIcon->SetVisibility(ESlateVisibility::Collapsed);
    }  
    
    if (BatteryTextPercent)
    {
        BatteryTextPercent->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UQuickbarWidget::SetBatterPercentText()
{
    if (!FlashlightComponent && !BatteryTextPercent) return;
    
    float Battery = FlashlightComponent->GetBatteryPercentage();
    
   float Raw = FlashlightComponent->GetBatteryPercentage();
    
    float Normalized = (Raw <= 1.01f) ? (Raw * 100.0f) : Raw;
    int32 Percent = FMath::RoundToInt(FMath::Clamp(Normalized,0.0f,100.0f));
    BatteryTextPercent->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), Percent)));
}

void UQuickbarWidget::UpdateBatteryBar()
{
    if (!FlashlightComponent || !BatteryBar)
    {
        return;
    }

    // Validate flashlight is equipped
    if (!FlashlightComponent->IsEquipped())
    {
        HideBatteryBar();
        return;
    }
    
    // Validate battery bar is visible
    if (BatteryBar->GetVisibility() != ESlateVisibility::HitTestInvisible)
    {
        return;
    }
    
    float BatteryPercent = FlashlightComponent->GetBatteryPercentage();
    BatteryPercent = FMath::Clamp(BatteryPercent, 0.0f, 100.0f);
    
    float BatteryValue = BatteryPercent / 100.0f;
    BatteryBar->SetPercent(BatteryValue);
    
    FLinearColor BarColor = GetBatteryColor(BatteryPercent);
    BatteryBar->SetFillColorAndOpacity(BarColor);
    
    SetBatterPercentText();
}

void UQuickbarWidget::UpdateLowBatteryWarning(float DeltaTime)
{
    if (!FlashlightComponent || CachedFlashlightSlot == -1)
    {
        if (bLowBatteryWarningActive)
        {
            DeactivateLowBatteryWarning();
        }
        return;
    }

    if (!FlashlightComponent->IsEquipped())
    {
        if (bLowBatteryWarningActive)
        {
            DeactivateLowBatteryWarning();
        }
        return;
    }

    if (!QuickbarSlots.IsValidIndex(CachedFlashlightSlot))
    {
        if (bLowBatteryWarningActive)
        {
            DeactivateLowBatteryWarning();
        }
        return;
    }

    float BatteryPercent = FlashlightComponent->GetBatteryPercentage();
    bool bIsLightOn = FlashlightComponent->IsLightOn();
    bool bShouldWarn = (BatteryPercent < LowBatteryThreshold && bIsLightOn);

    UInventorySlotWidget* FlashlightSlot = QuickbarSlots[CachedFlashlightSlot];
    if (!FlashlightSlot || !FlashlightSlot->SlotBorder)
    {
        return;
    }

    if (bShouldWarn)
    {
        if (!bLowBatteryWarningActive)
        {
            ActivateLowBatteryWarning();
        }

        // Pulsing effect
        float TimeSec = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        float PulseValue = FMath::Sin(TimeSec * PulseSpeed) * 0.5f + 0.5f;
        FLinearColor PulseColor = FMath::Lerp(CriticalBatteryColor, FLinearColor(0.1f, 0.0f, 0.0f, 1.0f), PulseValue);
        
        FlashlightSlot->SlotBorder->SetBrushColor(PulseColor);
    }
    else if (bLowBatteryWarningActive)
    {
        // Battery recharged or light turned off
        DeactivateLowBatteryWarning();
    }
}

void UQuickbarWidget::ActivateLowBatteryWarning()
{
    bLowBatteryWarningActive = true;
    UE_LOG(LogTemp, Warning, TEXT("LOW BATTERY WARNING ACTIVATED!"));
    
    // TODO: Play warning sound/animation
    // if (LowBatterySound) PlaySound(LowBatterySound);
}

void UQuickbarWidget::DeactivateLowBatteryWarning()
{
    bLowBatteryWarningActive = false;
    
    if (CachedFlashlightSlot >= 0)
    {
        ResetSlotVisuals(CachedFlashlightSlot);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Battery warning deactivated"));
}

void UQuickbarWidget::ResetSlotVisuals(int32 SlotIndex)
{
    if (!QuickbarSlots.IsValidIndex(SlotIndex)) return;

    UInventorySlotWidget* Slots = QuickbarSlots[SlotIndex];
    if (Slots)
    {
        Slots->UpdateVisuals();
        Slots->SetSelected(false);
    }
}

int32 UQuickbarWidget::FindFlashlightSlot() const
{
    if (!InventoryComponent)
    {
        return -1;
    }
    
    // Use cached value if valid
    if (CachedFlashlightSlot >= 0 && CachedFlashlightSlot < QuickbarSlotCount)
    {
        FInventorySlot CachedSlot = InventoryComponent->GetQuickbarSlot(CachedFlashlightSlot);
        if (CachedSlot.IsValid() && IsFlashlightItem(CachedSlot.ItemID))
        {
            return CachedFlashlightSlot; // Cache still valid
        }
    }
    
    // Cache invalid - search again
    for (int32 i = 0; i < QuickbarSlotCount; i++)
    {
        FInventorySlot SlotData = InventoryComponent->GetQuickbarSlot(i);
        
        if (SlotData.IsValid() && IsFlashlightItem(SlotData.ItemID))
        {
            return i;
        }
    }
    
    return -1;
}

bool UQuickbarWidget::IsFlashlightItem(FName ItemID) const
{
    if (!InventoryComponent || ItemID.IsNone())
    {
        return false;
    }

    FItemData ItemData;
    if (InventoryComponent->GetItemData(ItemID, ItemData))
    {
        return (ItemData.ItemType == EItemType::Tool && ItemData.ToolType == EToolType::Flashlight);
    }
    
    return false;
}

FLinearColor UQuickbarWidget::GetBatteryColor(float BatteryPercent) const
{
    if (BatteryPercent > 50.0f)
    {
        BatteryTextPercent->SetColorAndOpacity(HighBatteryColor);
        return HighBatteryColor;
    }
    else if (BatteryPercent > 20.0f)
    {
        BatteryTextPercent->SetColorAndOpacity(MediumBatteryColor);
        return MediumBatteryColor;
    }
    else if (BatteryPercent > LowBatteryThreshold)
    {
        BatteryTextPercent->SetColorAndOpacity(LowBatteryColor);
        return LowBatteryColor;
    }
    else
    {
        BatteryTextPercent->SetColorAndOpacity(CriticalBatteryColor);
        return CriticalBatteryColor;
    }
    
}

void UQuickbarWidget::UpdateFlashlightIcon(UTexture2D* Icon)
{
    if (!Icon) return;
    
    int32 FlashlightSlotIndex = FindFlashlightSlot();

    if (FlashlightSlotIndex == -1) return;

    if (!QuickbarSlots.IsValidIndex(FlashlightSlotIndex)) return;
    
    UInventorySlotWidget* Slots = QuickbarSlots[FlashlightSlotIndex];

    if (!Slots) return;

    if (Slots->ItemIcon)
    {
        Slots->ItemIcon->SetBrushFromTexture(Icon);
        Slots->ItemIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
}

bool UQuickbarWidget::ValidateQuickbarState()
{
    if (!InventoryComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("QuickbarWidget: No InventoryComponent!"));
        return false;
    }

    bool bIsValid = true;

    // Validate all quickbar slots
    for (int32 i = 0; i < QuickbarSlots.Num(); i++)
    {
        if (!QuickbarSlots[i])
        {
            UE_LOG(LogTemp, Error, TEXT("QuickbarSlot[%d] is NULL!"), i);
            bIsValid = false;
            continue;
        }

        int32 InvIndex = InventoryComponent->GetQuickbarInventoryIndex(i);
        
        if (InvIndex >= 0)
        {
            if (!InventoryComponent->InventorySlots.IsValidIndex(InvIndex))
            {
                UE_LOG(LogTemp, Error, TEXT("QuickbarSlot[%d] references invalid inventory index %d"), i, InvIndex);
                bIsValid = false;
            }
            else if (!InventoryComponent->InventorySlots[InvIndex].IsValid())
            {
                UE_LOG(LogTemp, Error, TEXT("QuickbarSlot[%d] references empty inventory slot %d"), i, InvIndex);
                bIsValid = false;
            }
        }
    }

    if (bIsValid)
    {
        UE_LOG(LogTemp, Log, TEXT("Quickbar state validation PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Quickbar state validation FAILED"));
    }

    return bIsValid;
}