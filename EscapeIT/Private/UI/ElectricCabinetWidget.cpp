#include "UI/ElectricCabinetWidget.h"

#include "../../../../../visual studio/Vs Product/DIA SDK/include/dia2.h"
#include "Components/BorderSlot.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/UniformGridSlot.h"

void UElectricCabinetWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    bIsDragging = false;
    CurrentDragColor = EWireColor::None;
    CurrentPathID = 0;
    
    if (CloseButton)
    {
        CloseButton->OnClicked.AddDynamic(this, &UElectricCabinetWidget::OnCloseButtonClicked);
    }
    
    if (InstructionText)
    {
        InstructionText->SetText(FText::FromString(TEXT("Connect all colors from start to end points!")));
    }
    
    InitializePuzzle();
}

void UElectricCabinetWidget::NativeDestruct()
{
    Super::NativeDestruct();
    
    if (CloseButton)
    {
        CloseButton->OnClicked.RemoveDynamic(this, &UElectricCabinetWidget::OnCloseButtonClicked);
    }
}

void UElectricCabinetWidget::InitializePuzzle()
{
    GridCells.Empty();
    PositionToIndexMap.Empty();
    CellBorders.Empty();
    WirePairs.Empty();
    
    for (int32 Y = 0; Y < GridHeight; Y++)
    {
        for (int32 X = 0; X < GridWidth; X++)
        {
            FGridCell Cell;
            Cell.GridPosition = FIntPoint(X, Y);
            Cell.CellType = ECellType::Empty;
            Cell.WireColor = EWireColor::None;
            Cell.PathID = -1;
            Cell.bIsConnected = false;
            
            int32 Index = GridCells.Add(Cell);
            PositionToIndexMap.Add(FIntPoint(X, Y), Index);
        }
    }
    
    GenerateRandomPuzzle();
    
    for (FWirePair& Wire : WirePairs)
    {
        if (FGridCell* StartCell = GetCellAt(Wire.StartPoint))
        {
            StartCell->CellType = ECellType::StartPoint;
            StartCell->WireColor = Wire.Color;
        }
        
        if (FGridCell* EndCell = GetCellAt(Wire.EndPoint))
        {
            EndCell->CellType = ECellType::EndPoint;
            EndCell->WireColor = Wire.Color;
        }
    }
    
    CreateGridUI();
    UpdateProgressDisplay();
}

void UElectricCabinetWidget::GenerateRandomPuzzle()
{
    TArray<FIntPoint> UsedPositions;
    
    TArray<EWireColor> FixedColors = {
        EWireColor::Red,
        EWireColor::Blue,
        EWireColor::Yellow,
        EWireColor::Green,
        EWireColor::Orange
    };
    
    int32 ActualPairs = FMath::Min(NumberOfWirePairs, FixedColors.Num());
    
    for (int32 i = 0; i < ActualPairs; i++)
    {
        FWirePair NewWire;
        
        NewWire.Color = FixedColors[i];
        
        FIntPoint StartPoint;
        int32 MaxAttempts = 100;
        int32 Attempts = 0;
        
        do
        {
            StartPoint = FIntPoint(
                FMath::RandRange(0, GridWidth - 1),
                FMath::RandRange(0, GridHeight - 1)
            );
            Attempts++;
        } 
        while (!IsPositionValid(StartPoint, UsedPositions) && Attempts < MaxAttempts);
        
        if (Attempts >= MaxAttempts)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to find valid start point for wire %d"), i);
            continue;
        }
        
        NewWire.StartPoint = StartPoint;
        UsedPositions.Add(StartPoint);
        
        float BestDistance = 0.0f;
        FIntPoint BestEndPoint = StartPoint;
        
        for (int32 Attempt = 0; Attempt < 50; Attempt++)
        {
            FIntPoint TestPoint = FIntPoint(
                FMath::RandRange(0, GridWidth - 1),
                FMath::RandRange(0, GridHeight - 1)
            );
            
            if (IsPositionValid(TestPoint, UsedPositions))
            {
                float Distance = GetDistanceBetweenPoints(StartPoint, TestPoint);
                if (Distance > BestDistance && Distance >= MinimumPointDistance)
                {
                    BestDistance = Distance;
                    BestEndPoint = TestPoint;
                }
            }
        }
        
        if (BestDistance > 0)
        {
            NewWire.EndPoint = BestEndPoint;
            UsedPositions.Add(BestEndPoint);
            NewWire.bIsComplete = false;
            WirePairs.Add(NewWire);
            
            UE_LOG(LogTemp, Log, TEXT("Generated %s wire: Start=(%d,%d), End=(%d,%d), Distance=%.1f"),
                *GetColorName(NewWire.Color),
                StartPoint.X, StartPoint.Y,
                BestEndPoint.X, BestEndPoint.Y,
                BestDistance);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to find valid end point for %s wire"), *GetColorName(NewWire.Color));
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Generated %d wire pairs with RANDOM POSITIONS"), WirePairs.Num());
}

bool UElectricCabinetWidget::IsPositionValid(FIntPoint Position, const TArray<FIntPoint>& UsedPositions)
{
    if (Position.X < 0 || Position.X >= GridWidth || Position.Y < 0 || Position.Y >= GridHeight)
    {
        return false;
    }
    
    for (const FIntPoint& Used : UsedPositions)
    {
        if (Used == Position)
        {
            return false;
        }
        
        float Distance = GetDistanceBetweenPoints(Position, Used);
        if (Distance < MinimumPointDistance)
        {
            return false;
        }
    }
    
    return true;
}

float UElectricCabinetWidget::GetDistanceBetweenPoints(FIntPoint A, FIntPoint B)
{
    const float dx = static_cast<float>(A.X - B.X);
    const float dy = static_cast<float>(A.Y - B.Y);
    return FMath::Sqrt(FMath::Square(dx) + FMath::Square(dy));
}

FString UElectricCabinetWidget::GetColorName(EWireColor Color)
{
    switch (Color)
    {
        case EWireColor::Red:    return TEXT("Red");
        case EWireColor::Blue:   return TEXT("Blue");
        case EWireColor::Yellow: return TEXT("Yellow");
        case EWireColor::Green:  return TEXT("Green");
        case EWireColor::Orange: return TEXT("Orange");
        default:                 return TEXT("Unknown");
    }
}

void UElectricCabinetWidget::CreateGridUI()
{
    if (!GridPanel)
        return;
    
    GridPanel->ClearChildren();
    CellBorders.Empty();
    
    for (int32 Y = 0; Y < GridHeight; Y++)
    {
        for (int32 X = 0; X < GridWidth; X++)
        {
            UBorder* CellBorder = NewObject<UBorder>(this);
            CellBorder->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f));
            CellBorder->SetPadding(FMargin(2.0f));
            
            UUniformGridSlot* GridSlot = GridPanel->AddChildToUniformGrid(CellBorder, Y, X);
            GridSlot->SetHorizontalAlignment(HAlign_Fill);
            GridSlot->SetVerticalAlignment(VAlign_Fill);
            
            CellBorders.Add(CellBorder);
            
            int32 Index = GetCellIndex(FIntPoint(X, Y));
            if (Index >= 0)
            {
                UpdateCellVisual(Index);
            }
        }
    }
}

void UElectricCabinetWidget::UpdateCellVisual(int32 CellIndex)
{
    if (!CellBorders.IsValidIndex(CellIndex) || !GridCells.IsValidIndex(CellIndex))
        return;
    
    const FGridCell& Cell = GridCells[CellIndex];
    UBorder* Border = CellBorders[CellIndex];
    
    if (!Border)
        return;
    
    FLinearColor CellColor = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
    if (Cell.CellType == ECellType::StartPoint || Cell.CellType == ECellType::EndPoint)
    {
        CellColor = GetColorForWireType(Cell.WireColor);
        CellColor.R = FMath::Min(1.0f, CellColor.R + 0.3f);
        CellColor.G = FMath::Min(1.0f, CellColor.G + 0.3f);
        CellColor.B = FMath::Min(1.0f, CellColor.B + 0.3f);
    }
    else if (Cell.CellType == ECellType::Path && Cell.WireColor != EWireColor::None)
    {
        CellColor = GetColorForWireType(Cell.WireColor);
    }
    
    Border->SetBrushColor(CellColor);
}

FLinearColor UElectricCabinetWidget::GetColorForWireType(EWireColor WireColor)
{
    switch (WireColor)
    {
        case EWireColor::Red:    return FLinearColor(0.8f, 0.1f, 0.1f);
        case EWireColor::Blue:   return FLinearColor(0.1f, 0.3f, 0.9f);
        case EWireColor::Yellow: return FLinearColor(0.9f, 0.9f, 0.1f);
        case EWireColor::Green:  return FLinearColor(0.1f, 0.8f, 0.1f);
        case EWireColor::Orange: return FLinearColor(1.0f, 0.5f, 0.0f);
        default:                 return FLinearColor(0.1f, 0.1f, 0.1f);
    }
}

FReply UElectricCabinetWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        FIntPoint GridPos = ScreenToGridPosition(InGeometry, InMouseEvent.GetScreenSpacePosition());
        StartDrag(GridPos);
        return FReply::Handled().CaptureMouse(this->TakeWidget());
    }
    
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UElectricCabinetWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bIsDragging)
    {
        EndDrag();
        return FReply::Handled().ReleaseMouseCapture();
    }
    
    return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UElectricCabinetWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (bIsDragging)
    {
        FIntPoint GridPos = ScreenToGridPosition(InGeometry, InMouseEvent.GetScreenSpacePosition());
        UpdateDrag(GridPos);
        return FReply::Handled();
    }
    
    return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

void UElectricCabinetWidget::StartDrag(FIntPoint CellPosition)
{
    FGridCell* Cell = GetCellAt(CellPosition);
    if (!Cell)
        return;
    
    if (Cell->CellType == ECellType::StartPoint)
    {
        bIsDragging = true;
        CurrentDragColor = Cell->WireColor;
        CurrentPathID++;
        LastCellPosition = CellPosition;
        
        ClearPath(CurrentDragColor);
        
        for (FWirePair& Wire : WirePairs)
        {
            if (Wire.Color == CurrentDragColor)
            {
                Wire.CurrentPath.Empty();
                Wire.CurrentPath.Add(CellPosition);
                Wire.bIsComplete = false;
                break;
            }
        }
    }
    else if (Cell->CellType == ECellType::Path)
    {
        for (FWirePair& Wire : WirePairs)
        {
            if (Wire.Color == Cell->WireColor)
            {
                int32 CellIndexInPath = Wire.CurrentPath.Find(CellPosition);
                if (CellIndexInPath != INDEX_NONE)
                {
                    bIsDragging = true;
                    CurrentDragColor = Cell->WireColor;
                    LastCellPosition = CellPosition;
                }
                break;
            }
        }
    }
}

void UElectricCabinetWidget::UpdateDrag(FIntPoint CellPosition)
{
    if (!bIsDragging || CellPosition == LastCellPosition)
        return;
    
    int32 DeltaX = FMath::Abs(CellPosition.X - LastCellPosition.X);
    int32 DeltaY = FMath::Abs(CellPosition.Y - LastCellPosition.Y);
    
    if ((DeltaX == 1 && DeltaY == 0) || (DeltaX == 0 && DeltaY == 1))
    {
        FGridCell* Cell = GetCellAt(CellPosition);
        
        if (!Cell)
            return;
        
        FWirePair* CurrentWire = nullptr;
        for (FWirePair& Wire : WirePairs)
        {
            if (Wire.Color == CurrentDragColor)
            {
                CurrentWire = &Wire;
                break;
            }
        }
        
        if (!CurrentWire)
            return;

        if (Cell->CellType == ECellType::Path && Cell->WireColor == CurrentDragColor)
        {
            int32 CellIndexInPath = CurrentWire->CurrentPath.Find(CellPosition);

            if (CellIndexInPath != INDEX_NONE && CellIndexInPath < CurrentWire->CurrentPath.Num() - 1)
            {
                int32 NumToRemove = CurrentWire->CurrentPath.Num() - CellIndexInPath - 1;
                
                for (int32 i =0;i < NumToRemove;i++)
                {
                    FIntPoint PostToRemove = CurrentWire->CurrentPath.Last();
                    FGridCell* CellToRemove = GetCellAt(PostToRemove);

                    if (CellToRemove && CellToRemove->CellType == ECellType::Path)
                    {
                        CellToRemove->CellType = ECellType::Empty;
                        CellToRemove->WireColor = EWireColor::None;
                        CellToRemove->PathID = -1;
                        
                        int32 Index = GetCellIndex(PostToRemove);
                        UpdateCellVisual(Index);
                    }
                    
                    CurrentWire->CurrentPath.RemoveAt(CurrentWire->CurrentPath.Num() - 1);
                }
                
                CurrentWire->bIsComplete = false;
                LastCellPosition = CellPosition;
                UpdateProgressDisplay();
                return;
            }
        }
        
        if (IsCellBlockedByOtherPath(CellPosition, CurrentDragColor))
        {
            UE_LOG(LogTemp, Log, TEXT("Path blocked by another color! Auto-clearing current path."));
            AutoClearCurrentPath();
            return;
        }
        
        if (Cell->CellType == ECellType::EndPoint && Cell->WireColor != CurrentDragColor)
        {
            UE_LOG(LogTemp, Log, TEXT("Wrong color endpoint! Auto-clearing current path."));
            AutoClearCurrentPath();
            return;
        }
        
        if (CanPlacePath(CellPosition, CurrentDragColor))
        {
            if (Cell->CellType == ECellType::EndPoint && Cell->WireColor == CurrentDragColor)
            {
                CurrentWire->CurrentPath.Add(CellPosition);
                CurrentWire->bIsComplete = true;
                bIsDragging = false;
                
                UE_LOG(LogTemp, Log, TEXT("Wire %s completed!"), *GetColorName(CurrentWire->Color));
                
                UpdateProgressDisplay();
                
                if (CheckPuzzleComplete())
                {
                    UE_LOG(LogTemp, Log, TEXT("All wires connected! Puzzle complete!"));
                    OnPuzzleCompleted.Broadcast();
                }
            }
            else if (Cell->CellType == ECellType::Empty)
            {
                Cell->CellType = ECellType::Path;
                Cell->WireColor = CurrentDragColor;
                Cell->PathID = CurrentPathID;
                
                int32 CellIndex = GetCellIndex(CellPosition);
                UpdateCellVisual(CellIndex);
                
                CurrentWire->CurrentPath.Add(CellPosition);
                LastCellPosition = CellPosition;
                
                UE_LOG(LogTemp, Verbose, TEXT("Path extended to (%d, %d)"), CellPosition.X, CellPosition.Y);
            }
        }
    }
}

void UElectricCabinetWidget::EndDrag()
{
    bIsDragging = false;
    CurrentDragColor = EWireColor::None;
}

bool UElectricCabinetWidget::CanPlacePath(FIntPoint Position, EWireColor Color)
{
    FGridCell* Cell = GetCellAt(Position);
    if (!Cell)
        return false;
    
    if (Cell->CellType == ECellType::Empty)
        return true;
    
    if (Cell->CellType == ECellType::EndPoint && Cell->WireColor == Color)
        return true;
    
    return false;
}

void UElectricCabinetWidget::AutoClearCurrentPath()
{
    if (CurrentDragColor == EWireColor::None)
        return;
    
    ClearPath(CurrentDragColor);
    
    bIsDragging = false;
    CurrentDragColor = EWireColor::None;
    
    UpdateProgressDisplay();
}

void UElectricCabinetWidget::ClearPath(EWireColor Color)
{
    for (FGridCell& Cell : GridCells)
    {
        if (Cell.CellType == ECellType::Path && Cell.WireColor == Color)
        {
            Cell.CellType = ECellType::Empty;
            Cell.WireColor = EWireColor::None;
            Cell.PathID = -1;
            
            int32 Index = GetCellIndex(Cell.GridPosition);
            UpdateCellVisual(Index);
        }
    }
    
    for (FWirePair& Wire : WirePairs)
    {
        if (Wire.Color == Color)
        {
            Wire.CurrentPath.Empty();
            Wire.bIsComplete = false;
            break;
        }
    }
}

bool UElectricCabinetWidget::IsCellBlockedByOtherPath(FIntPoint Position, EWireColor Color)
{
    FGridCell* Cell = GetCellAt(Position);
    if (!Cell)
        return false;
    
    if (Cell->CellType == ECellType::Path && Cell->WireColor != Color)
    {
        return true;
    }
    
    if ((Cell->CellType == ECellType::StartPoint || Cell->CellType == ECellType::EndPoint) 
        && Cell->WireColor != Color)
    {
        return true;
    }
    
    return false;
}

bool UElectricCabinetWidget::CheckPuzzleComplete()
{
    for (const FWirePair& Wire : WirePairs)
    {
        if (!Wire.bIsComplete)
            return false;
    }
    
    return WirePairs.Num() > 0;
}

void UElectricCabinetWidget::UpdateProgressDisplay()
{
    if (!ProgressText)
        return;
    
    int32 CompletedCount = 0;
    for (const FWirePair& Wire : WirePairs)
    {
        if (Wire.bIsComplete)
            CompletedCount++;
    }
    
    FString ProgressString = FString::Printf(TEXT("Progress: %d / %d"), CompletedCount, WirePairs.Num());
    ProgressText->SetText(FText::FromString(ProgressString));
}

FGridCell* UElectricCabinetWidget::GetCellAt(FIntPoint Position)
{
    int32* IndexPtr = PositionToIndexMap.Find(Position);
    if (IndexPtr && GridCells.IsValidIndex(*IndexPtr))
    {
        return &GridCells[*IndexPtr];
    }
    return nullptr;
}

int32 UElectricCabinetWidget::GetCellIndex(FIntPoint Position)
{
    int32* IndexPtr = PositionToIndexMap.Find(Position);
    return IndexPtr ? *IndexPtr : -1;
}

FIntPoint UElectricCabinetWidget::ScreenToGridPosition(const FGeometry& Geometry, const FVector2D& ScreenPosition)
{
    if (!GridPanel)
        return FIntPoint(-1, -1);
    
    FVector2D LocalPosition = Geometry.AbsoluteToLocal(ScreenPosition);
    FGeometry GridGeometry = GridPanel->GetCachedGeometry();
    FVector2D GridLocalPos = GridGeometry.AbsoluteToLocal(ScreenPosition);
    
    FVector2D GridSize = GridGeometry.GetLocalSize();
    float CellWidth = GridSize.X / GridWidth;
    float CellHeight = GridSize.Y / GridHeight;
    
    int32 X = FMath::FloorToInt(GridLocalPos.X / CellWidth);
    int32 Y = FMath::FloorToInt(GridLocalPos.Y / CellHeight);
    
    if (X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight)
    {
        return FIntPoint(X, Y);
    }
    
    return FIntPoint(-1, -1);
}

void UElectricCabinetWidget::OnCloseButtonClicked()
{
    RemoveFromParent();
}