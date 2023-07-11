

using UnrealBuildTool;
using System.Collections.Generic;

public class KariTarget : TargetRules
{
	public KariTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		ExtraModuleNames.AddRange( new string[] { "Kari" } );
	}
}
