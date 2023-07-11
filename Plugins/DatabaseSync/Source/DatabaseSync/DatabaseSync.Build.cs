// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class DatabaseSync : ModuleRules
{
	public DatabaseSync(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        List<string> publicDependencyModuleList = new List<string>()
        {
            "Core",
            "Http",
            "Json",
            "JsonUtilities",
            "UMG",
            "Engine",
            "CoreUObject"
        };

        if (Target.Type == TargetRules.TargetType.Editor)
        {
            publicDependencyModuleList.Add("UnrealEd");
            publicDependencyModuleList.Add("BlueprintGraph");
            publicDependencyModuleList.Add("GraphEditor");
            publicDependencyModuleList.Add("PropertyEditor");
            publicDependencyModuleList.Add("SlateCore");
            publicDependencyModuleList.Add("Slate");
            publicDependencyModuleList.Add("EditorStyle");
            publicDependencyModuleList.Add("Kismet");
            publicDependencyModuleList.Add("KismetCompiler");
        }

        PublicDependencyModuleNames.AddRange(publicDependencyModuleList.ToArray());


        List<string> privateDependencyModuleList = new List<string>()
        { 
            "Projects",
            "InputCore",
            "CoreUObject",
            "Engine",
            "Slate",
            "SlateCore",

        };

        if (Target.Type == TargetRules.TargetType.Editor)
        {
            privateDependencyModuleList.Add("LevelEditor");
            privateDependencyModuleList.Add("PropertyEditor");
            privateDependencyModuleList.Add("EditorStyle");
            privateDependencyModuleList.Add("UnrealEd");
        }

        PrivateDependencyModuleNames.AddRange(privateDependencyModuleList.ToArray());
	}
}
