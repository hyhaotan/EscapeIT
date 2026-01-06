#include "UI/ElectricCabinetWidget.h"

#include "EscapeITCharacter.h"
#include "Components/BorderSlot.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/UniformGridSlot.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

void UElectricCabinetWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Initialize dragging state variables
    bIsDragging = false;
    CurrentDragColor = EWireColor::None;
    CurrentPathID = 0;
    
    RemainingTime = RepairDuration;
    bTimerActive = true;
    bPuzzleFinished = false;
    
    // Validate grid configuration
    if (GridWidth <= 0 || GridHeight <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid grid configuration! GridWidth=%d, GridHeight=%d"), 
               GridWidth, GridHeight);
        
        // Set safe default values to prevent crash
        GridWidth = FMath::Max(1, GridWidth);
        GridHeight = FMath::Max(1, GridHeight);
    }
    
    // Validate number of wire pairs
    if (NumberOfWirePairs <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid NumberOfWirePairs=%d (must be > 0)"), NumberOfWirePairs);
        NumberOfWirePairs = 1;
    }
    
    // Validate GridPanel widget binding
    if (!GridPanel)
    {
        UE_LOG(LogTemp, Error, TEXT("GridPanel is nullptr! Widget binding failed!"));
        return;
    }
    
    // Bind close button callback
    if (CloseButton)
    {
        CloseButton->OnClicked.AddDynamic(this, &UElectricCabinetWidget::OnCloseButtonClicked);
    }
    
    // Set instruction text
    if (InstructionText)
    {
        InstructionText->SetText(FText::FromString(TEXT("Connect all colors from start to end points!")));
    }
    
    // Initialize the puzzle
    InitializePuzzle();
    
    // Update timer display initially
    UpdateRepairedPuzzleTime();
}

void UElectricCabinetWidget::NativeDestruct()
{
    Super::NativeDestruct();
    
    // Unbind callbacks to prevent memory leaks
    if (CloseButton)
    {
        CloseButton->OnClicked.RemoveDynamic(this, &UElectricCabinetWidget::OnCloseButtonClicked);
    }
}

void UElectricCabinetWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bTimerActive && !bPuzzleFinished)
    {
        OnRepairedPuzzleCountDown(InDeltaTime);
    }
}

void UElectricCabinetWidget::InitializePuzzle()
{
    // Clear all existing data
    GridCells.Empty();
    PositionToIndexMap.Empty();
    CellBorders.Empty();
    WirePairs.Empty();
    
    RemainingTime = RepairDuration;
    bPuzzleFinished = false;
    bTimerActive = true;
    
    // Create grid cells and initialize them as empty
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
            
            // Add cell and create position-to-index mapping
            int32 Index = GridCells.Add(Cell);
            PositionToIndexMap.Add(FIntPoint(X, Y), Index);
        }
    }
    
    // Generate random puzzle with guaranteed solution
    GenerateRandomPuzzle();
    
    // Mark start and end points on the grid
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
    
    // Create visual representation
    CreateGridUI();
    
    // Update progress display
    UpdateProgressDisplay();
    
    // Log final result
    if (WirePairs.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Puzzle generation failed - no wire pairs created!"));
    }
}

void UElectricCabinetWidget::GenerateRandomPuzzle()
{
    // Define available wire colors
    TArray<EWireColor> FixedColors = {
        EWireColor::Red,
        EWireColor::Blue,
        EWireColor::Yellow,
        EWireColor::Green,
        EWireColor::Orange
    };
    
    // Limit number of wire pairs to available colors
    int32 ActualPairs = FMath::Min(NumberOfWirePairs, FixedColors.Num());
    
    // Validate grid is large enough
    int32 MinCellsNeeded = ActualPairs * 2;
    int32 AvailableCells = GridWidth * GridHeight;
    
    if (AvailableCells < MinCellsNeeded)
    {
        UE_LOG(LogTemp, Error, TEXT("Grid too small! Need %d cells but only have %d"), MinCellsNeeded, AvailableCells);
        ActualPairs = AvailableCells / 2;
        
        if (ActualPairs <= 0)
        {
            return;
        }
    }
    
    // Try multiple times to generate a valid puzzle
    int32 MaxPuzzleAttempts = 50;
    bool bValidPuzzleFound = false;
    
    for (int32 Attempt = 0; Attempt < MaxPuzzleAttempts && !bValidPuzzleFound; Attempt++)
    {
        // Reset grid to empty state
        ResetGridToEmpty();
        
        // Generate puzzle by drawing complete paths first
        TArray<FWirePair> GeneratedWires;
        bool bAllWiresPlaced = true;
        
        for (int32 i = 0; i < ActualPairs; i++)
        {
            FWirePair NewWire;
            NewWire.Color = FixedColors[i];
            
            // Try to create a complete path for this wire
            if (GenerateCompletePath(NewWire))
            {
                GeneratedWires.Add(NewWire);
            }
            else
            {
                bAllWiresPlaced = false;
                break;
            }
        }
        
        // Check if all wires were successfully placed
        if (bAllWiresPlaced && GeneratedWires.Num() == ActualPairs)
        {
            // Convert solution to puzzle (remove paths, keep only start/end)
            ConvertSolutionToPuzzle(GeneratedWires);
            
            // Save generated puzzle
            WirePairs = GeneratedWires;
            bValidPuzzleFound = true;
        }
    }
    
    if (!bValidPuzzleFound)
    {
        UE_LOG(LogTemp, Error, TEXT("Could not generate valid puzzle after %d attempts"), MaxPuzzleAttempts);
        UE_LOG(LogTemp, Error, TEXT("Try: increase GridWidth/GridHeight or decrease NumberOfWirePairs"));
    }
}

bool UElectricCabinetWidget::GenerateCompletePath(FWirePair& OutWire)
{
    // Find random empty cell for start point
    FIntPoint StartPoint = FindRandomEmptyCell();
    if (StartPoint.X < 0)
    {
        return false;
    }
    
    // Try to create path from this start point
    int32 MaxPathAttempts = 100;
    
    for (int32 Attempt = 0; Attempt < MaxPathAttempts; Attempt++)
    {
        // Find random empty cell for end point (far enough from start)
        FIntPoint EndPoint = FindRandomEmptyCell();
        if (EndPoint.X < 0)
            continue;
        
        // FIXED: Ensure start and end points are different
        if (StartPoint == EndPoint)
            continue;
        
        // Check distance requirement
        float Distance = GetDistanceBetweenPoints(StartPoint, EndPoint);
        if (Distance < MinimumPointDistance)
            continue;
        
        // Try to generate path from Start to End
        TArray<FIntPoint> GeneratedPath;
        if (GeneratePathBetweenPoints(StartPoint, EndPoint, GeneratedPath))
        {
            // Success - mark entire path on grid
            OutWire.StartPoint = StartPoint;
            OutWire.EndPoint = EndPoint;
            OutWire.CurrentPath = GeneratedPath;
            OutWire.bIsComplete = true;
            
            // Mark all cells in path
            MarkPathOnGrid(GeneratedPath, OutWire.Color);
            
            return true;
        }
    }
    
    return false;
}

bool UElectricCabinetWidget::GeneratePathBetweenPoints(FIntPoint Start, FIntPoint End, TArray<FIntPoint>& OutPath)
{
    OutPath.Empty();
    
    // Use biased random walk algorithm - random movement but prefer direction toward goal
    TArray<FIntPoint> CurrentPath;
    CurrentPath.Add(Start);
    
    FIntPoint Current = Start;
    int32 MaxSteps = GridWidth * GridHeight * 2; // Limit steps to prevent infinite loop
    int32 Steps = 0;
    
    TSet<FIntPoint> VisitedInThisPath; // Avoid revisiting cells in same path
    VisitedInThisPath.Add(Start);
    
    while (Current != End && Steps < MaxSteps)
    {
        Steps++;
        
        // Calculate preferred direction (toward End)
        int32 DeltaX = End.X - Current.X;
        int32 DeltaY = End.Y - Current.Y;
        
        // Build list of possible directions with weights
        TArray<FIntPoint> PossibleDirections;
        TArray<float> DirectionWeights;
        
        // Four directions: Up, Down, Left, Right
        TArray<FIntPoint> AllDirections = {
            FIntPoint(1, 0),   // Right
            FIntPoint(-1, 0),  // Left
            FIntPoint(0, 1),   // Down
            FIntPoint(0, -1)   // Up
        };
        
        for (const FIntPoint& Dir : AllDirections)
        {
            FIntPoint Next = Current + Dir;
            
            // Check if within bounds
            if (Next.X < 0 || Next.X >= GridWidth || Next.Y < 0 || Next.Y >= GridHeight)
                continue;
            
            // Check if cell is available (or is destination)
            FGridCell* Cell = GetCellAt(Next);
            if (!Cell)
                continue;
            
            bool bIsDestination = (Next == End);
            bool bIsEmpty = (Cell->CellType == ECellType::Empty);
            bool bNotVisitedYet = !VisitedInThisPath.Contains(Next);
            
            if (bIsDestination || (bIsEmpty && bNotVisitedYet))
            {
                PossibleDirections.Add(Dir);
                
                // Calculate weight - prefer directions toward goal
                float Weight = 1.0f;
                
                // Increase weight if direction moves toward End
                if ((Dir.X > 0 && DeltaX > 0) || (Dir.X < 0 && DeltaX < 0))
                    Weight += 3.0f; // High priority for correct X direction
                if ((Dir.Y > 0 && DeltaY > 0) || (Dir.Y < 0 && DeltaY < 0))
                    Weight += 3.0f; // High priority for correct Y direction
                
                DirectionWeights.Add(Weight);
            }
        }
        
        // If no valid directions available, path generation failed
        if (PossibleDirections.Num() == 0)
        {
            return false; // Dead end
        }
        
        // Choose random direction with weights
        FIntPoint ChosenDirection = ChooseWeightedRandom(PossibleDirections, DirectionWeights);
        Current = Current + ChosenDirection;
        
        CurrentPath.Add(Current);
        VisitedInThisPath.Add(Current);
        
        // Check if reached destination
        if (Current == End)
        {
            OutPath = CurrentPath;
            return true;
        }
    }
    
    // Path generation timeout
    return false;
}

FIntPoint UElectricCabinetWidget::ChooseWeightedRandom(const TArray<FIntPoint>& Choices, const TArray<float>& Weights)
{
    if (Choices.Num() == 0)
        return FIntPoint(0, 0);
    
    if (Choices.Num() != Weights.Num())
    {
        // If no weights provided, choose uniformly random
        return Choices[FMath::RandRange(0, Choices.Num() - 1)];
    }
    
    // Calculate total weight
    float TotalWeight = 0.0f;
    for (float W : Weights)
        TotalWeight += W;
    
    // FIXED: Handle edge case where total weight is zero
    if (TotalWeight <= 0.0f)
    {
        return Choices[FMath::RandRange(0, Choices.Num() - 1)];
    }
    
    // Choose random value in [0, TotalWeight)
    float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
    
    // Find corresponding choice
    float CurrentSum = 0.0f;
    for (int32 i = 0; i < Choices.Num(); i++)
    {
        CurrentSum += Weights[i];
        if (RandomValue <= CurrentSum)
            return Choices[i];
    }
    
    // Fallback
    return Choices.Last();
}

FIntPoint UElectricCabinetWidget::FindRandomEmptyCell()
{
    TArray<FIntPoint> EmptyCells;
    
    // Collect all empty cells
    for (const FGridCell& Cell : GridCells)
    {
        if (Cell.CellType == ECellType::Empty)
        {
            EmptyCells.Add(Cell.GridPosition);
        }
    }
    
    if (EmptyCells.Num() == 0)
    {
        return FIntPoint(-1, -1); // No empty cells available
    }
    
    // Choose random cell
    int32 RandomIndex = FMath::RandRange(0, EmptyCells.Num() - 1);
    return EmptyCells[RandomIndex];
}

void UElectricCabinetWidget::MarkPathOnGrid(const TArray<FIntPoint>& Path, EWireColor Color)
{
    for (int32 i = 0; i < Path.Num(); i++)
    {
        FGridCell* Cell = GetCellAt(Path[i]);
        if (Cell)
        {
            // First cell is start point
            if (i == 0)
            {
                Cell->CellType = ECellType::StartPoint;
            }
            // Last cell is end point
            else if (i == Path.Num() - 1)
            {
                Cell->CellType = ECellType::EndPoint;
            }
            // Middle cells are path
            else
            {
                Cell->CellType = ECellType::Path;
            }
            
            Cell->WireColor = Color;
        }
    }
}

void UElectricCabinetWidget::ResetGridToEmpty()
{
    for (FGridCell& Cell : GridCells)
    {
        Cell.CellType = ECellType::Empty;
        Cell.WireColor = EWireColor::None;
        Cell.PathID = -1;
        Cell.bIsConnected = false;
    }
}

void UElectricCabinetWidget::ConvertSolutionToPuzzle(TArray<FWirePair>& Wires)
{
    // Reset grid
    ResetGridToEmpty();
    
    // Only mark start and end points
    for (FWirePair& Wire : Wires)
    {
        // Mark start point
        FGridCell* StartCell = GetCellAt(Wire.StartPoint);
        if (StartCell)
        {
            StartCell->CellType = ECellType::StartPoint;
            StartCell->WireColor = Wire.Color;
        }
        
        // Mark end point
        FGridCell* EndCell = GetCellAt(Wire.EndPoint);
        if (EndCell)
        {
            EndCell->CellType = ECellType::EndPoint;
            EndCell->WireColor = Wire.Color;
        }
        
        // Clear path - player must find it themselves
        Wire.CurrentPath.Empty();
        Wire.bIsComplete = false;
    }
}

bool UElectricCabinetWidget::IsPuzzleSolvable(const TArray<FWirePair>& WiresToCheck)
{
    // Early validation
    if (WiresToCheck.Num() == 0)
    {
        return false;
    }
    
    // Check if each wire has at least one valid path
    for (const FWirePair& Wire : WiresToCheck)
    {
        // Collect positions occupied by other wires' start/end points
        TArray<FIntPoint> OccupiedPositions;
        
        for (const FWirePair& OtherWire : WiresToCheck)
        {
            if (OtherWire.Color != Wire.Color)
            {
                OccupiedPositions.Add(OtherWire.StartPoint);
                OccupiedPositions.Add(OtherWire.EndPoint);
            }
        }

        // Use BFS to check if path exists
        if (!HasPathBetweenPoints(Wire.StartPoint, Wire.EndPoint, OccupiedPositions))
        {
            return false;
        }
    }
    
    // Check if there's enough space on grid
    int32 TotalCellsNeeded = 0;
    for (const FWirePair& Wire : WiresToCheck)
    {
        // Calculate minimum path length using Manhattan distance
        int32 MinPathLength = FMath::Abs(Wire.EndPoint.X - Wire.StartPoint.X) +
                             FMath::Abs(Wire.EndPoint.Y - Wire.StartPoint.Y) + 1;
        TotalCellsNeeded += MinPathLength;
    }
    
    // Don't use more than 80% of grid to avoid overcrowding
    int32 TotalAvailableCells = GridWidth * GridHeight;
    if (TotalCellsNeeded > TotalAvailableCells * 0.8f)
    {
        return false;
    }
    
    return true;
}

bool UElectricCabinetWidget::HasPathBetweenPoints(FIntPoint Start, FIntPoint End, const TArray<FIntPoint>& BlockedPositions)
{
    // Validate input
    if (Start.X < 0 || Start.X >= GridWidth || Start.Y < 0 || Start.Y >= GridHeight ||
        End.X < 0 || End.X >= GridWidth || End.Y < 0 || End.Y >= GridHeight)
    {
        return false;
    }
    
    // Use Breadth-First Search (BFS)
    TArray<FIntPoint> Queue;
    TSet<FIntPoint> Visited;
    
    Queue.Add(Start);
    Visited.Add(Start);
    
    // Four movement directions
    TArray<FIntPoint> Directions = {
        FIntPoint(0, 1),   // Down
        FIntPoint(0, -1),  // Up
        FIntPoint(1, 0),   // Right
        FIntPoint(-1, 0),  // Left
    };
    
    // BFS main loop
    while (Queue.Num() > 0)
    {
        FIntPoint Current = Queue[0];
        Queue.RemoveAt(0);

        // Check if reached destination
        if (Current == End)
        {
            return true;
        }
        
        // Explore all neighboring cells
        for (const FIntPoint& Dir : Directions)
        {
            FIntPoint Next = Current + Dir;
            
            // Check bounds
            if (Next.X < 0 || Next.X >= GridWidth || Next.Y < 0 || Next.Y >= GridHeight)
            {
                continue;
            }
            
            // Check if already visited
            if (Visited.Contains(Next))
            {
                continue;
            }
            
            // Check if blocked (allow going through if it's the destination)
            if (Next != End && BlockedPositions.Contains(Next))
            {
                continue;
            }
            
            // Add to exploration queue
            Queue.Add(Next);
            Visited.Add(Next);
        }
    }
    
    // No path found
    return false;
}

bool UElectricCabinetWidget::IsPositionValid(FIntPoint Position, const TArray<FIntPoint>& UsedPositions)
{
    // Check if within grid bounds
    if (Position.X < 0 || Position.X >= GridWidth || Position.Y < 0 || Position.Y >= GridHeight)
    {
        return false;
    }
    
    // Check if position is already used or too close to another point
    for (const FIntPoint& Used : UsedPositions)
    {
        // Exact same position
        if (Used == Position)
        {
            return false;
        }
        
        // Too close to existing point
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
    // Calculate Euclidean distance
    const float dx = static_cast<float>(A.X - B.X);
    const float dy = static_cast<float>(A.Y - B.Y);
    return FMath::Sqrt(FMath::Square(dx) + FMath::Square(dy));
}

FString UElectricCabinetWidget::GetColorName(EWireColor Color)
{
    // Convert wire color enum to string
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
    {
        UE_LOG(LogTemp, Error, TEXT("GridPanel is nullptr!"));
        return;
    }
    
    // Clear existing UI elements
    GridPanel->ClearChildren();
    CellBorders.Empty();
    
    // Create border widget for each cell
    for (int32 Y = 0; Y < GridHeight; Y++)
    {
        for (int32 X = 0; X < GridWidth; X++)
        {
            UBorder* CellBorder = NewObject<UBorder>(this);
            if (!CellBorder)
                continue;
            
            CellBorder->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f));
            CellBorder->SetPadding(FMargin(2.0f));
            
            // Add to grid panel
            UUniformGridSlot* GridSlot = GridPanel->AddChildToUniformGrid(CellBorder, Y, X);
            if (!GridSlot)
                continue;
            
            GridSlot->SetHorizontalAlignment(HAlign_Fill);
            GridSlot->SetVerticalAlignment(VAlign_Fill);
            
            // Store reference
            CellBorders.Add(CellBorder);
            
            // Update visual
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
    // Validate indices
    if (!CellBorders.IsValidIndex(CellIndex) || !GridCells.IsValidIndex(CellIndex))
    {
        return;
    }
    
    const FGridCell& Cell = GridCells[CellIndex];
    UBorder* Border = CellBorders[CellIndex];
    
    if (!Border)
    {
        return;
    }
    
    // Default color for empty cells
    FLinearColor CellColor = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
    // Start and end points are brighter
    if (Cell.CellType == ECellType::StartPoint || Cell.CellType == ECellType::EndPoint)
    {
        CellColor = GetColorForWireType(Cell.WireColor);
        CellColor.R = FMath::Min(1.0f, CellColor.R + 0.3f);
        CellColor.G = FMath::Min(1.0f, CellColor.G + 0.3f);
        CellColor.B = FMath::Min(1.0f, CellColor.B + 0.3f);
    }
    // Path cells use normal wire color
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
    if (bPuzzleFinished)
    {
        return FReply::Handled();
    }
    
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        FIntPoint GridPos = ScreenToGridPosition(InGeometry, InMouseEvent.GetScreenSpacePosition());
        
        if (GridPos.X < 0 || GridPos.Y < 0)
        {
            return FReply::Handled();
        }
        
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
        
        if (GridPos.X < 0 || GridPos.Y < 0)
        {
            return FReply::Handled();
        }
        
        UpdateDrag(GridPos);
        
        return FReply::Handled();
    }
    
    return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

void UElectricCabinetWidget::StartDrag(FIntPoint CellPosition)
{
    FGridCell* Cell = GetCellAt(CellPosition);
    if (!Cell)
    {
        return;
    }
    
    // Can start dragging from start point
    if (Cell->CellType == ECellType::StartPoint)
    {
        bIsDragging = true;
        CurrentDragColor = Cell->WireColor;
        CurrentPathID++;
        LastCellPosition = CellPosition;
        
        // Clear any existing path for this color
        ClearPath(CurrentDragColor);
        
        // Initialize new path
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
    // Can also start from existing path to modify it
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
    // Ignore if not dragging or position unchanged
    if (!bIsDragging || CellPosition == LastCellPosition)
        return;
    
    // Calculate movement delta
    int32 DeltaX = FMath::Abs(CellPosition.X - LastCellPosition.X);
    int32 DeltaY = FMath::Abs(CellPosition.Y - LastCellPosition.Y);
    
    // Only allow adjacent cell movement (no diagonal)
    if ((DeltaX == 1 && DeltaY == 0) || (DeltaX == 0 && DeltaY == 1))
    {
        FGridCell* Cell = GetCellAt(CellPosition);
        
        if (!Cell)
            return;
        
        // Find current wire
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
        {
            return;
        }

        // Handle backtracking on existing path
        if (Cell->CellType == ECellType::Path && Cell->WireColor == CurrentDragColor)
        {
            int32 CellIndexInPath = CurrentWire->CurrentPath.Find(CellPosition);

            if (CellIndexInPath != INDEX_NONE && CellIndexInPath < CurrentWire->CurrentPath.Num() - 1)
            {
                int32 NumToRemove = CurrentWire->CurrentPath.Num() - CellIndexInPath - 1;
                
                for (int32 i = 0; i < NumToRemove; i++)
                {
                    FIntPoint PosToRemove = CurrentWire->CurrentPath.Last();
                    FGridCell* CellToRemove = GetCellAt(PosToRemove);

                    if (CellToRemove && CellToRemove->CellType == ECellType::Path)
                    {
                        CellToRemove->CellType = ECellType::Empty;
                        CellToRemove->WireColor = EWireColor::None;
                        CellToRemove->PathID = -1;
                        
                        int32 Index = GetCellIndex(PosToRemove);
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
        
        // Check if blocked by another wire's path
        if (IsCellBlockedByOtherPath(CellPosition, CurrentDragColor))
        {
            AutoClearCurrentPath();
            return;
        }
        
        // Check if trying to reach wrong color endpoint
        if (Cell->CellType == ECellType::EndPoint && Cell->WireColor != CurrentDragColor)
        {
            AutoClearCurrentPath();
            return;
        }
        
        // Try to place path
        if (CanPlacePath(CellPosition, CurrentDragColor))
        {
            // Successfully reached correct endpoint
            if (Cell->CellType == ECellType::EndPoint && Cell->WireColor == CurrentDragColor)
            {
                CurrentWire->CurrentPath.Add(CellPosition);
                CurrentWire->bIsComplete = true;
                bIsDragging = false;
                
                UpdateProgressDisplay();
                
                // Check if puzzle complete
                if (CheckPuzzleComplete())
                {
                    OnPuzzleCompleted.Broadcast();
                }
            }
            // Extend path through empty cell
            else if (Cell->CellType == ECellType::Empty)
            {
                Cell->CellType = ECellType::Path;
                Cell->WireColor = CurrentDragColor;
                Cell->PathID = CurrentPathID;
                
                int32 CellIndex = GetCellIndex(CellPosition);
                UpdateCellVisual(CellIndex);
                
                CurrentWire->CurrentPath.Add(CellPosition);
                LastCellPosition = CellPosition;
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
    
    // Can place on empty cells
    if (Cell->CellType == ECellType::Empty)
        return true;
    
    // Can place on matching endpoint
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
    // Remove all path cells of specified color
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
    
    // Clear wire's path array
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
    
    // Blocked if it's a different color path
    if (Cell->CellType == ECellType::Path && Cell->WireColor != Color)
    {
        return true;
    }
    
    // Blocked if it's different color start/end point
    if ((Cell->CellType == ECellType::StartPoint || Cell->CellType == ECellType::EndPoint) 
        && Cell->WireColor != Color)
    {
        return true;
    }
    
    return false;
}

bool UElectricCabinetWidget::CheckPuzzleComplete()
{
    // Check if all wires are complete
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
    {
        return;
    }
    
    // Count completed wires
    int32 CompletedCount = 0;
    for (const FWirePair& Wire : WirePairs)
    {
        if (Wire.bIsComplete)
            CompletedCount++;
    }
    
    // Display progress
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
    {
        return FIntPoint(-1, -1);
    }
    
    if (GridWidth <= 0 || GridHeight <= 0)
    {
        return FIntPoint(-1, -1);
    }
    
    // Convert screen space to local space
    FGeometry GridGeometry = GridPanel->GetCachedGeometry();
    FVector2D GridLocalPos = GridGeometry.AbsoluteToLocal(ScreenPosition);
    
    // Calculate cell size
    FVector2D GridSize = GridGeometry.GetLocalSize();
    
    if (GridSize.X <= 0.0f || GridSize.Y <= 0.0f)
    {
        return FIntPoint(-1, -1);
    }
    
    float CellWidth = GridSize.X / static_cast<float>(GridWidth);
    float CellHeight = GridSize.Y / static_cast<float>(GridHeight);
    
    // Convert to grid coordinates
    int32 X = FMath::FloorToInt(GridLocalPos.X / CellWidth);
    int32 Y = FMath::FloorToInt(GridLocalPos.Y / CellHeight);
    
    // Validate coordinates
    if (X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight)
    {
        return FIntPoint(X, Y);
    }
    
    return FIntPoint(-1, -1);
}

void UElectricCabinetWidget::UpdateRepairedPuzzleTime()
{
    if (!RepairDuration) return;
    
    int32 Seconds = FMath::CeilToInt(RemainingTime);

    if (Seconds <= 5)
    {
        RepairTimeText->SetColorAndOpacity(FLinearColor::Red);
    }
    else if (Seconds <= 10)
    {
        RepairTimeText->SetColorAndOpacity(FLinearColor::Yellow);
    }
    else
    {
        RepairTimeText->SetColorAndOpacity(FLinearColor::White);
    }
    
    RepairTimeText->SetText(FText::AsNumber(Seconds));
    OnUpdateRepairedPuzzleTime.Broadcast(RemainingTime);
}


void UElectricCabinetWidget::OnRepairedPuzzleCountDown(float DeltaTime)
{
    if (!bTimerActive || bPuzzleFinished) return;
    
    RemainingTime -= DeltaTime;

    if (RemainingTime <= 0.0f)
    {
        RemainingTime = 0.0f;
    }
    
    UpdateRepairedPuzzleTime();

    if (RemainingTime <= 0.0f)
    {
        OnTimerExpired();
    }
}

void UElectricCabinetWidget::OnTimerExpired()
{
    if (bPuzzleFinished) return;
    
    bTimerActive = false;
    bPuzzleFinished = true;
    bIsDragging = false;
    
    OnPuzzleTimedOut.Broadcast();

    if (InstructionText)
    {
        InstructionText->SetText(FText::FromString(TEXT("TIME'S UP! You've been electrocuted!")));
        InstructionText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
    }

    if (HideAnim)
    {
        PlayAnimation(HideAnim);
        
        FWidgetAnimationDynamicEvent AnimationFinishedDelegate;
        AnimationFinishedDelegate.BindDynamic(this,&UElectricCabinetWidget::OnTimerExpiredAnimationFinished);
        BindToAnimationFinished(HideAnim,AnimationFinishedDelegate);
    }
    else
    {
        OnTimerExpiredAnimationFinished();
    }
}

void UElectricCabinetWidget::OnTimerExpiredAnimationFinished()
{
    SetVisibility(ESlateVisibility::Collapsed);
    
    FInputModeGameOnly InputMode;
    const auto PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }
    
    ResetTimer();
    OnUpdateRepairedPuzzleTime.Broadcast(RemainingTime);
    
    AEscapeITCharacter* PlayerChar = Cast<AEscapeITCharacter>(PC->GetPawn());
    if (PlayerChar)
    {
        PlayerChar->PlayAnimMontage(PlayerChar->ElectricShockAnim,1.0f);
    }
}

void UElectricCabinetWidget::PauseTimer()
{
    bTimerActive = false;
}

void UElectricCabinetWidget::ResumeTimer()
{
    if (!bPuzzleFinished)
    {
        bTimerActive = true;
    }
}

void UElectricCabinetWidget::ResetTimer()
{
    RemainingTime = RepairDuration;
    bTimerActive = true;
    bPuzzleFinished = false;
    UpdateRepairedPuzzleTime();
}

void UElectricCabinetWidget::OnCloseButtonClicked()
{
    FInputModeGameOnly InputMode;
    
    const auto PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }
    
    ResetTimer();
    OnUpdateRepairedPuzzleTime.Broadcast(RemainingTime);
    
    HideAnimWidget();
    
    FTimerHandle HideTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        HideTimerHandle,
        [this]()
        {
            SetVisibility(ESlateVisibility::Collapsed);
        },
        0.5f,
        false
    );
}
