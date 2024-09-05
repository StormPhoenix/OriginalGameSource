using UnrealBuildTool;

public class OriginalGameEditor : ModuleRules
{
    public OriginalGameEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "BlueprintGraph",
                "Engine",
                "Slate",
                "SlateCore"
            }
        );
    }
}