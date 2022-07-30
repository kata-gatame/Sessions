// kata.codes
using UnrealBuildTool;

public class Sessions : ModuleRules
{
	public Sessions(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new [] { "Core", "OnlineSubsystem", "OnlineSubsystemSteam" });
		PrivateDependencyModuleNames.AddRange(new [] { "CoreUObject", "Engine", "UMG", "Slate", "SlateCore" });
	}
}