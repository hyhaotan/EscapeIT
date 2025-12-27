// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class EscapeIT : ModuleRules
{
    public EscapeIT(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {  
        });

        PrivateDependencyModuleNames.AddRange(new string[] 
        {  "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "AIModule",
            "GameplayTasks",
            "CommonUI",
            "NavigationSystem",
            "Paper2D",
            "HeadMountedDisplay",
            "EnhancedInput",
            "UMG",
            "SlateCore",
            "Slate",
            "Niagara",
            "GameplayCameras",
            "Json",
            "JsonUtilities",
            "RHI",
            "RenderCore",
            "ApplicationCore",
            "PhysicsCore",
            "LevelSequence",       
            "MovieScene",        
            "MovieSceneTracks"
        });

        PublicIncludePaths.AddRange(new string[] {
            "EscapeIT/Public",
        });
        
        PrivateIncludePaths.AddRange(new string[]
        {
            "EscapeIT/Private"
        });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}