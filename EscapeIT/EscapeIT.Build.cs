// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class EscapeIT : ModuleRules
{
    public EscapeIT(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {   "Core",
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
            "ApplicationCore"});

        PrivateDependencyModuleNames.AddRange(new string[] { });

        PublicIncludePaths.AddRange(new string[] {
            "EscapeIT",
        });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}