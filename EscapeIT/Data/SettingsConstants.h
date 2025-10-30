
#pragma once

#include "CoreMinimal.h"

namespace SettingsConstants
{
    // ===== RESOLUTION LIMITS =====
    constexpr int32 MIN_RESOLUTION_X = 800;
    constexpr int32 MAX_RESOLUTION_X = 7680; // 8K
    constexpr int32 MIN_RESOLUTION_Y = 600;
    constexpr int32 MAX_RESOLUTION_Y = 4320; // 8K

    // ===== FOV LIMITS =====
    constexpr float MIN_FOV = 60.0f;
    constexpr float MAX_FOV = 120.0f;
    constexpr float DEFAULT_FOV = 90.0f;

    // ===== VOLUME LIMITS =====
    constexpr float MIN_VOLUME = 0.0f;
    //constexpr float MAX_VOLUME = 1.0f;

    // ===== SENSITIVITY LIMITS =====
    constexpr float MIN_SENSITIVITY = 0.1f;
    constexpr float MAX_SENSITIVITY = 3.0f;
    constexpr float DEFAULT_SENSITIVITY = 1.0f;

    // ===== GAMEPAD LIMITS =====
    constexpr float MIN_DEADZONE = 0.0f;
    constexpr float MAX_DEADZONE = 0.5f;
    constexpr float DEFAULT_DEADZONE = 0.2f;

    // ===== SAVE SYSTEM =====
    constexpr float AUTO_SAVE_INTERVAL = 30.0f; // seconds
    constexpr float DEFERRED_SAVE_DELAY = 2.0f; // seconds
    constexpr int32 MAX_BACKUP_COUNT = 5;
    constexpr int32 SETTINGS_VERSION = 1;

    // ===== FRAME RATE =====
    constexpr int32 MIN_FRAME_RATE = 30;
    constexpr int32 MAX_FRAME_RATE = 300;
    constexpr int32 UNLIMITED_FRAME_RATE = 0;

    // ===== DIFFICULTY MULTIPLIERS =====
    namespace Difficulty
    {
        namespace Easy
        {
            constexpr float SANITY_DRAIN = 0.7f;
            constexpr float AI_DETECTION = 0.7f;
            constexpr float AI_SPEED = 0.8f;
            constexpr float ENTITY_CHASE = 0.8f;
            constexpr float BATTERY_LIFE = 1.5f;
            constexpr float PUZZLE_HINT = 1.5f;
        }

        namespace Normal
        {
            constexpr float SANITY_DRAIN = 1.0f;
            constexpr float AI_DETECTION = 1.0f;
            constexpr float AI_SPEED = 1.0f;
            constexpr float ENTITY_CHASE = 1.0f;
            constexpr float BATTERY_LIFE = 1.0f;
            constexpr float PUZZLE_HINT = 1.0f;
        }

        namespace Hard
        {
            constexpr float SANITY_DRAIN = 1.3f;
            constexpr float AI_DETECTION = 1.3f;
            constexpr float AI_SPEED = 1.2f;
            constexpr float ENTITY_CHASE = 1.2f;
            constexpr float BATTERY_LIFE = 0.7f;
            constexpr float PUZZLE_HINT = 0.7f;
        }

        namespace Nightmare
        {
            constexpr float SANITY_DRAIN = 1.7f;
            constexpr float AI_DETECTION = 1.5f;
            constexpr float AI_SPEED = 1.4f;
            constexpr float ENTITY_CHASE = 1.5f;
            constexpr float BATTERY_LIFE = 0.5f;
            constexpr float PUZZLE_HINT = 0.5f;
        }
    }

    // ===== ACCESSIBILITY =====
    constexpr float MIN_HOLD_TIME = 0.1f;
    constexpr float MAX_HOLD_TIME = 5.0f;
    constexpr float DEFAULT_HOLD_TIME = 0.5f;

    // ===== PUZZLE SETTINGS =====
    constexpr float MIN_HINT_TIME = 10.0f;
    constexpr float MAX_HINT_TIME = 300.0f;
    constexpr float DEFAULT_HINT_TIME = 60.0f;

    // ===== RAY TRACING QUALITY SAMPLES =====
    constexpr int32 RT_LOW_SAMPLES = 1;
    constexpr int32 RT_MEDIUM_SAMPLES = 2;
    constexpr int32 RT_HIGH_SAMPLES = 4;

    // ===== MOTION BLUR SCALE =====
    constexpr float MOTION_BLUR_SCALE = 4.0f; // UE uses 0-4 scale

    // ===== SAVE SLOT NAMES =====
    const FString DEFAULT_SAVE_SLOT = TEXT("GameSettings");
    const FString BACKUP_PREFIX = TEXT("Backup_");
    const FString DEFAULT_PROFILE_NAME = TEXT("Default");
}