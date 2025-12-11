// QuickbarWidget.cpp - IMPROVED VERSION

#include "QuickbarWidget.h"
#include "InventorySlotWidget.h"
#include "EscapeIT/Actor/Components/InventoryComponent.h"
#include "EscapeIT/Actor/Components/FlashlightComponent.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

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
        
        
        if (!FlashlightComponent->OnFlashlightImageChanged.IsAlreadyBound(this,&UQuickbarWidget::UpdateFlashlightIcon))
        {
            FlashlightComponent->OnFlashlightImageChanged.AddDynamic(this,&UQuickbarWidget::UpdateFlashlightIcon);
        }  
        
        if (!FlashlightComponent->OnFlashlightEquippedChanged.IsAlreadyBound(this,&UQuickbarWidget::OnFlashlightEquippedChanged))
        {
            FlashlightComponent->OnFlashlightEquippedChanged.AddDynamic(this,&UQuickbarWidget::OnFlashlightEquippedChanged);
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
        FlashlightComponent->OnFlashlightEquippedChanged.RemoveDynamic(this, &UQuickbarWidget::OnFlashlightEquippedChanged);
    }
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
    RefreshQuickbar();
}

void UQuickbarWidget::OnItemAdded(FName ItemID, int32 Quantity)
{
    RefreshQuickbar();
    
    if (IsFlashlightItem(ItemID))
    {
        if (FlashlightComponent->IsEquipped())
        {
            CachedFlashlightSlot = -1;
            UpdateBatteryBarVisibility();
            UE_LOG(LogTemp, Log, TEXT("✅ Flashlight added - Battery bar updated"));
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

void UQuickbarWidget::OnFlashlightEquippedChanged(bool bIsEquipped)
{
    CachedFlashlightSlot = -1;
    UpdateBatteryBarVisibility();

    if (bIsEquipped)
    {
        UpdateBatteryBar();
    }
    else
    {
        bLowBatteryWarningActive = false;
        if (CurrentSelectedSlot != -1)
        {
            ResetSlotVisuals(CurrentSelectedSlot);
        }
    }
}

void UQuickbarWidget::UpdateBatteryBarVisibility()
{
    if (!FlashlightComponent)
    {
        // Không có FlashlightComponent -> ẩn battery bar
        HideBatteryBar();
        return;
    }

    // Kiểm tra xem flashlight có được equipped không
    bool bIsEquipped = FlashlightComponent->IsEquipped();
    
    // Tìm flashlight slot (để đảm bảo flashlight vẫn trong quickbar)
    int32 FlashlightSlotIndex = FindFlashlightSlot();
    bool bHasFlashlightInQuickbar = (FlashlightSlotIndex != -1);
    
    // Update cached slot
    CachedFlashlightSlot = FlashlightSlotIndex;
    
    // ✅ CHỈ HIỆN KHI: Có flashlight trong quickbar VÀ đang được equipped
    bool bShouldShow = bHasFlashlightInQuickbar && bIsEquipped;
    
    if (bShouldShow)
    {
        ShowBatteryBar();
        UpdateBatteryBar(); // Update giá trị ngay lập tức
    }
    else
    {
        HideBatteryBar();
    }
    
    UE_LOG(LogTemp, Log, TEXT("🔋 Battery Bar: %s (InQuickbar: %s | Equipped: %s | Slot: %d)"), 
        bShouldShow ? TEXT("VISIBLE") : TEXT("HIDDEN"),
        bHasFlashlightInQuickbar ? TEXT("YES") : TEXT("NO"),
        bIsEquipped ? TEXT("YES") : TEXT("NO"),
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
}

void UQuickbarWidget::UpdateBatteryBar()
{
    if (!FlashlightComponent || !BatteryBar) return;

    if (!FlashlightComponent->IsEquipped()) return;
    
    float BatteryPercent = FlashlightComponent->GetBatteryPercentage();
    
    BatteryPercent = FMath::Clamp(BatteryPercent, 0.0f, 100.0f);
    float BatteryValue = BatteryPercent / 100.0f;
    
    BatteryBar->SetPercent(BatteryValue);
    
    FLinearColor BarColor = GetBatteryColor(BatteryPercent);
    BatteryBar->SetFillColorAndOpacity(BarColor);
}

void UQuickbarWidget::UpdateLowBatteryWarning(float DeltaTime)
{
    if (!FlashlightComponent || CachedFlashlightSlot == -1) return;
    if (!FlashlightComponent->IsEquipped()) return;

    float BatteryPercent = FlashlightComponent->GetBatteryPercentage();
    bool bShouldWarn = (BatteryPercent < LowBatteryThreshold && FlashlightComponent->IsLightOn());

    if (!QuickbarSlots.IsValidIndex(CachedFlashlightSlot)) return;

    UInventorySlotWidget* FlashlightSlot = QuickbarSlots[CachedFlashlightSlot];
    if (!FlashlightSlot || !FlashlightSlot->SlotBorder) return;

    if (bShouldWarn)
    {
        if (!bLowBatteryWarningActive)
        {
            bLowBatteryWarningActive = true;
            UE_LOG(LogTemp, Warning, TEXT("⚠️ LOW BATTERY WARNING ACTIVATED!"));
        }

        // Pulsing effect
        float PulseValue = FMath::Sin(GetWorld()->GetTimeSeconds() * PulseSpeed) * 0.5f + 0.5f;
        FLinearColor PulseColor = FMath::Lerp(CriticalBatteryColor, FLinearColor::Black, PulseValue);
        FlashlightSlot->SlotBorder->SetBrushColor(PulseColor);
    }
    else if (bLowBatteryWarningActive && BatteryPercent >= LowBatteryThreshold)
    {
        // Deactivate warning
        bLowBatteryWarningActive = false;
        ResetSlotVisuals(CachedFlashlightSlot);
        UE_LOG(LogTemp, Log, TEXT("✅ Battery warning deactivated"));
    }
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
    if (!InventoryComponent) return -1;
    
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
        return HighBatteryColor;
    }
    else if (BatteryPercent > 20.0f)
    {
        return MediumBatteryColor;
    }
    else if (BatteryPercent > LowBatteryThreshold)
    {
        return LowBatteryColor;
    }
    else
    {
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


