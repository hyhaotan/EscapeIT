#pragma once

#include "CoreMinimal.h"
#include "DialogueData.generated.h"

UENUM(BlueprintType)
enum class EHorrorDialogueType : uint8
{
    PlayerReaction      UMETA(DisplayName = "Player Reaction"),
    PlayerMonologue     UMETA(DisplayName = "Player Monologue"),
    PlayerPanic         UMETA(DisplayName = "Player Panic"),
    
    InvestigateObject   UMETA(DisplayName = "Investigate Object"),
    ReadNote            UMETA(DisplayName = "Read Note"),
    DiscoverClue        UMETA(DisplayName = "Discover Clue"),
    
    RadioMessage        UMETA(DisplayName = "Radio Message"),
    PhoneCall           UMETA(DisplayName = "Phone Call"),
    VoiceRecording      UMETA(DisplayName = "Voice Recording"),
    
    Whisper             UMETA(DisplayName = "Whisper"),
    Hallucination       UMETA(DisplayName = "Hallucination"),
    EntityVoice         UMETA(DisplayName = "Entity Voice"),
    
    Narration           UMETA(DisplayName = "Narration"),
    Flashback           UMETA(DisplayName = "Flashback"),
    Tutorial            UMETA(DisplayName = "Tutorial"),
};

UENUM(BlueprintType)
enum class EHorrorIntensity : uint8
{
    Calm        UMETA(DisplayName = "Calm"),      
    Nervous     UMETA(DisplayName = "Nervous"),   
    Scared      UMETA(DisplayName = "Scared"),   
    Terrified   UMETA(DisplayName = "Terrified"),   
    Panic       UMETA(DisplayName = "Panic"),      
};

USTRUCT(BlueprintType)
struct FSubtitleLine
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitle")
    FText Name;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitle")
    FText Subtitle;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitle")
    USoundBase* Voice;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitle")
    float Duration = 3.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitle")
    bool bKeepPreviousName = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitle")
    bool bCanSkip = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitle")
    float DelayBeforeShow = 0.0f;
};

USTRUCT(BlueprintType)
struct FHorrorDialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    USoundBase* VoiceSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bKeepPreviousName = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Type")
    EHorrorDialogueType DialogueType = EHorrorDialogueType::PlayerReaction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Type")
    EHorrorIntensity Intensity = EHorrorIntensity::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror Effects")
    bool bPlayBreathingSound = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror Effects")
    bool bPlayHeartbeatSound = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror Effects")
    bool bShakeScreen = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror Effects")
    float ScreenShakeIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror Effects")
    bool bDistortAudio = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror Effects")
    bool bFlickerLights = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
    float DelayBeforeShow = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
    bool bInterruptible = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
    bool bOnlyPlayOnce = false;

    FSubtitleLine ToSubtitleLine() const
    {
        FSubtitleLine SubLine;
        SubLine.Name = SpeakerName;
        SubLine.Subtitle = DialogueText;
        SubLine.Voice = VoiceSound;
        SubLine.Duration = Duration;
        SubLine.bKeepPreviousName = bKeepPreviousName;
        SubLine.bCanSkip = bInterruptible;
        SubLine.DelayBeforeShow = DelayBeforeShow;
        return SubLine;
    }
};

USTRUCT(BlueprintType)
struct FHorrorDialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequence")
    FName SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequence")
    TArray<FHorrorDialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequence State")
    bool bHasBeenPlayed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequence Priority")
    int32 Priority = 0; 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequence Control")
    bool bBlockMovementDuringDialogue = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequence Control")
    bool bBlockPlayerInput = false;
};

USTRUCT(BlueprintType)
struct FDialogueTableRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FHorrorDialogueSequence DialogueSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata")
    FText SequenceDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata")
    FString ChapterName; 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata")
    TArray<FName> RequiredSequences; 
};

UCLASS()
class ESCAPEIT_API UDialogueDataHelpers : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Dialogue Helpers")
    static FSubtitleLine MakeSubtitleLine(
        const FText& Name,
        const FText& Subtitle,
        USoundBase* Voice = nullptr,
        float Duration = 3.0f,
        bool bKeepName = false,
        float Delay = 0.0f,
        bool Skip = true
    );

    UFUNCTION(BlueprintCallable, Category = "Dialogue Helpers")
    static FHorrorDialogueLine MakeHorrorLine(
        const FText& Speaker,
        const FText& Dialogue,
        USoundBase* Voice = nullptr,
        float Duration = 3.0f,
        EHorrorIntensity Intensity = EHorrorIntensity::Calm,
        bool bKeepName = false
    );

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue Helpers")
    static float GetIntensityAsFloat(EHorrorIntensity Intensity);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue Helpers")
    static EHorrorIntensity GetIntensityFromFloat(float Value);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue Helpers")
    static bool IsSupernaturalDialogue(EHorrorDialogueType Type);
};