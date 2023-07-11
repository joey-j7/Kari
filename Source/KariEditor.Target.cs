

using UnrealBuildTool;
using System.Collections.Generic;

public class KariEditorTarget : TargetRules
{
	public KariEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		ExtraModuleNames.AddRange( new string[] { "Kari" } );
	}
}
