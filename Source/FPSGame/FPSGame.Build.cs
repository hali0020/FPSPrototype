using UnrealBuildTool;

public class FPSGame : ModuleRules
{
    public FPSGame(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core", "CoreUObject", "Engine", "InputCore", "AnimGraphRuntime"
        });
    }
}
