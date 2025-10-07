#include "QuickbarWidget.h"
#include "EscapeIT/Actor//Components/InventoryComponent.h"
#include "EscapeIT/Actor//Components/FlashlightComponent.h"
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

    // Initialize visual state
    RefreshQuickbar();
}

void UQuickbarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Update battery bar every frame for smooth animation
    if (FlashlightComponent && Slot1_Battery)
    {
        float BatteryPercent = FlashlightComponent->GetBatteryPercentage();
        UpdateBatteryDisplay(BatteryPercent);

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

            if (Slot1_Border)
            {
                Slot1_Border->SetBrushColor(PulseColor);
            }
        }
        else if (bLowBatteryWarningActive && BatteryPercent >= 10.0f)
        {
            bLowBatteryWarningActive = false;
            if (Slot1_Border)
            {
                Slot1_Border->SetBrushColor(NormalBorderColor);
            }
        }
    }

    // Update cooldowns for slots 2, 3, 4
    if (InventoryComponent)
    {
        for (int32 i = 1; i < 4; i++)
        {
            FInventorySlot SlotData = InventoryComponent->GetQuickbarSlot(i);
            if (SlotData.IsValid() && SlotData.CooldownRemaining > 0.0f)
            {
                FItemData ItemData;
                if (InventoryComponent->GetItemData(SlotData.ItemID, ItemData))
                {
                    float CooldownPercent = SlotData.CooldownRemaining / ItemData.UsageCooldown;
                    UpdateCooldownDisplay(i, CooldownPercent);
                }
            }
        }
    }
}

void UQuickbarWidget::Initialize(UInventoryComponent* InInventoryComp, UFlashlightComponent* InFlashlightComp)
{
    InventoryComponent = InInventoryComp;
    FlashlightComponent = InFlashlightComp;

    if (InventoryComponent)
    {
        // Bind to inventory updates
        InventoryComponent->OnInventoryUpdated.AddDynamic(this, &UQuickbarWidget::OnInventoryUpdated);
        ItemDataTable = InventoryComponent->ItemDataTable;
    }

    if (FlashlightComponent)
    {
        // Bind to battery and flashlight events
        FlashlightComponent->OnBatteryChanged.AddDynamic(this, &UQuickbarWidget::OnBatteryChanged);
        FlashlightComponent->OnFlashlightToggled.AddDynamic(this, &UQuickbarWidget::OnFlashlightToggled);
    }

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

    for (int32 i = 0; i < 4; i++)
    {
        UpdateSlot(i);
    }
}

void UQuickbarWidget::UpdateSlot(int32 SlotIndex)
{
    if (!InventoryComponent || SlotIndex < 0 || SlotIndex >= 4)
    {
        return;
    }

    FInventorySlot SlotData = InventoryComponent->GetQuickbarSlot(SlotIndex);
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
        ProgressBar = Slot2_Cooldown;
        Border = Slot2_Border;
        break;
    case 2:
        Icon = Slot3_Icon;
        Quantity = Slot3_Quantity;
        ProgressBar = Slot3_Cooldown;
        Border = Slot3_Border;
        break;
    case 3:
        Icon = Slot4_Icon;
        Quantity = Slot4_Quantity;
        ProgressBar = Slot4_Cooldown;
        Border = Slot4_Border;
        break;
    }

    if (!Icon || !Quantity || !Border)
    {
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
    if (ItemDataTable)
    {
        FItemData* Data = ItemDataTable->FindRow<FItemData>(SlotData.ItemID, TEXT("UpdateSlotVisuals"));
        if (Data)
        {
            ItemData = *Data;
        }
    }

    // Set icon
    if (ItemData.Icon)
    {
        Icon->SetBrushFromTexture(ItemData.Icon);
        Icon->SetColorAndOpacity(FLinearColor::White);
    }

    // Set quantity
    if (SlotData.Quantity > 1)
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
        if (SlotIndex == 0 && FlashlightComponent)
        {
            // Battery bar for flashlight
            ProgressBar->SetVisibility(ESlateVisibility::Visible);
            UpdateBatteryDisplay(FlashlightComponent->GetBatteryPercentage());
        }
        else if (SlotData.CooldownRemaining > 0.0f)
        {
            // Cooldown bar for other items
            ProgressBar->SetVisibility(ESlateVisibility::Visible);

            if (ItemData.UsageCooldown > 0.0f)
            {
                float CooldownPercent = SlotData.CooldownRemaining / ItemData.UsageCooldown;
                UpdateCooldownDisplay(SlotIndex, CooldownPercent);
            }
        }
        else
        {
            ProgressBar->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    // Set border color
    if (!bLowBatteryWarningActive || SlotIndex != 0)
    {
        Border->SetBrushColor(NormalBorderColor);
    }
}

void UQuickbarWidget::UpdateBatteryDisplay(float Percentage)
{
    if (!Slot1_Battery)
    {
        return;
    }

    Slot1_Battery->SetPercent(Percentage / 100.0f);

    // Color coding
    FLinearColor BatteryColor;
    if (Percentage > 50.0f)
    {
        BatteryColor = FLinearColor::Green;
    }
    else if (Percentage > 20.0f)
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

    Slot1_Battery->SetFillColorAndOpacity(BatteryColor);
}

void UQuickbarWidget::UpdateCooldownDisplay(int32 SlotIndex, float CooldownPercent)
{
    UProgressBar* ProgressBar = nullptr;

    switch (SlotIndex)
    {
    case 1: ProgressBar = Slot2_Cooldown; break;
    case 2: ProgressBar = Slot3_Cooldown; break;
    case 3: ProgressBar = Slot4_Cooldown; break;
    }

    if (ProgressBar)
    {
        ProgressBar->SetPercent(CooldownPercent);
        ProgressBar->SetFillColorAndOpacity(FLinearColor(0.5f, 0.5f, 1.0f, 0.8f)); // Blue tint
    }
}

void UQuickbarWidget::HighlightSlot(int32 SlotIndex)
{
    // Reset previous selection
    if (CurrentSelectedSlot >= 0 && CurrentSelectedSlot < 4)
    {
        UBorder* PrevBorder = nullptr;
        switch (CurrentSelectedSlot)
        {
        case 0: PrevBorder = Slot1_Border; break;
        case 1: PrevBorder = Slot2_Border; break;
        case 2: PrevBorder = Slot3_Border; break;
        case 3: PrevBorder = Slot4_Border; break;
        }
        if (PrevBorder && !bLowBatteryWarningActive)
        {
            PrevBorder->SetBrushColor(NormalBorderColor);
        }
    }

    // Highlight new slot
    CurrentSelectedSlot = SlotIndex;
    UBorder* NewBorder = nullptr;
    switch (SlotIndex)
    {
    case 0: NewBorder = Slot1_Border; break;
    case 1: NewBorder = Slot2_Border; break;
    case 2: NewBorder = Slot3_Border; break;
    case 3: NewBorder = Slot4_Border; break;
    }
    if (NewBorder)
    {
        NewBorder->SetBrushColor(SelectedBorderColor);

        // Brief scale animation (optional - can be done in Blueprint animation)
        // NewBorder->SetRenderScale(FVector2D(1.1f, 1.1f));
    }
}

void UQuickbarWidget::OnInventoryUpdated()
{
    RefreshQuickbar();
}

void UQuickbarWidget::OnBatteryChanged(float Percentage)
{
    UpdateBatteryDisplay(Percentage);
}

void UQuickbarWidget::OnFlashlightToggled(bool bIsOn)
{
    // Visual feedback when flashlight toggles
    if (Slot1_Border)
    {
        if (bIsOn)
        {
            HighlightSlot(0);
        }
        else if (CurrentSelectedSlot == 0)
        {
            Slot1_Border->SetBrushColor(NormalBorderColor);
            CurrentSelectedSlot = -1;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Flashlight toggled: %s"), bIsOn ? TEXT("ON") : TEXT("OFF"));
}