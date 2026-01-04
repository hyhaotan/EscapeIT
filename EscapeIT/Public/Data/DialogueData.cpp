#include "DialogueData.h"

FSubtitleLine UDialogueDataHelpers::MakeSubtitleLine(
    const FText& Name,
    const FText& Subtitle,
    USoundBase* Voice,
    float Duration,
    bool bKeepName,
    float Delay)
{
    FSubtitleLine Line;
    Line.Name = Name;
    Line.Subtitle = Subtitle;
    Line.Voice = Voice;
    Line.Duration = Duration;
    Line.bKeepPreviousName = bKeepName;
    Line.DelayBeforeShow = Delay;
    return Line;
}

FHorrorDialogueLine UDialogueDataHelpers::MakeHorrorLine(
    const FText& Speaker,
    const FText& Dialogue,
    USoundBase* Voice,
    float Duration,
    EHorrorIntensity Intensity,
    bool bKeepName)
{
    FHorrorDialogueLine Line;
    Line.SpeakerName = Speaker;
    Line.DialogueText = Dialogue;
    Line.VoiceSound = Voice;
    Line.Duration = Duration;
    Line.Intensity = Intensity;
    Line.bKeepPreviousName = bKeepName;
    
    if (Intensity >= EHorrorIntensity::Scared)
    {
        Line.bPlayBreathingSound = true;
    }
    if (Intensity >= EHorrorIntensity::Terrified)
    {
        Line.bPlayHeartbeatSound = true;
        Line.bShakeScreen = true;
        Line.ScreenShakeIntensity = 0.5f;
    }
    if (Intensity == EHorrorIntensity::Panic)
    {
        Line.bShakeScreen = true;
        Line.ScreenShakeIntensity = 1.0f;
        Line.bPlayHeartbeatSound = true;
        Line.bPlayBreathingSound = true;
    }
    
    return Line;
}

float UDialogueDataHelpers::GetIntensityAsFloat(EHorrorIntensity Intensity)
{
    switch (Intensity)
    {
        case EHorrorIntensity::Calm:
            return 0.0f;
        case EHorrorIntensity::Nervous:
            return 0.25f;
        case EHorrorIntensity::Scared:
            return 0.5f;
        case EHorrorIntensity::Terrified:
            return 0.75f;
        case EHorrorIntensity::Panic:
            return 1.0f;
        default:
            return 0.0f;
    }
}

EHorrorIntensity UDialogueDataHelpers::GetIntensityFromFloat(float Value)
{
    Value = FMath::Clamp(Value, 0.0f, 1.0f);
    
    if (Value < 0.2f)
        return EHorrorIntensity::Calm;
    else if (Value < 0.4f)
        return EHorrorIntensity::Nervous;
    else if (Value < 0.6f)
        return EHorrorIntensity::Scared;
    else if (Value < 0.8f)
        return EHorrorIntensity::Terrified;
    else
        return EHorrorIntensity::Panic;
}

bool UDialogueDataHelpers::IsSupernaturalDialogue(EHorrorDialogueType Type)
{
    return Type == EHorrorDialogueType::Whisper ||
           Type == EHorrorDialogueType::Hallucination ||
           Type == EHorrorDialogueType::EntityVoice;
}