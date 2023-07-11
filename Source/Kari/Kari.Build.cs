

using UnrealBuildTool;
using System.Collections.Generic;

public class Kari : ModuleRules
{
	public Kari(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		if (Target.Configuration != UnrealTargetConfiguration.Shipping)
		{
			MinFilesUsingPrecompiledHeaderOverride = 1;
			bFasterWithoutUnity = true;
		}

        List<string> privateDependencyModuleList = new List<string>()
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "UMG",
            "FMODStudio",
            "SunPosition",
            "DatabaseSync",
            "Niagara",
            "SteamUEr"
        };

        //  4.23+
        if (Target.Type == TargetRules.TargetType.Editor)
        {
            //privateDependencyModuleList.Add("DatabaseSync");
            privateDependencyModuleList.Add("Slate");
            privateDependencyModuleList.Add("SlateCore");
            privateDependencyModuleList.Add("UnrealEd");
            privateDependencyModuleList.Add("BlueprintGraph");
            privateDependencyModuleList.Add("GraphEditor");
            privateDependencyModuleList.Add("PropertyEditor");
            privateDependencyModuleList.Add("EditorStyle");
        }

        PrivateDependencyModuleNames.AddRange(privateDependencyModuleList.ToArray());

        PublicDependencyModuleNames.AddRange(new string[] {  });
	}
}
