#pragma once

UENUM(BlueprintType)
enum class EMatchType : uint8
{
	EMT_FFA UMETA(DisplayName = "Free For All"),
	EMT_CTF UMETA(DisplayName = "Capture the Flag"),

	EMT_MAX UMETA(DisplayName = "DefaultMAX")
};