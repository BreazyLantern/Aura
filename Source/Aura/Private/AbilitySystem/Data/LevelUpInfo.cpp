// Copyright Druid Mechanics


#include "AbilitySystem/Data/LevelUpInfo.h"

int32 ULevelUpInfo::FindLevelForXP(int32 XP) const
{
	int32 Level = 1;
	bool bSearching = true;
	while (bSearching)
	{
		// Note this could be done using data table curves and such however we are setting them manually for the rewards and requirements per level.
		// LevelUpInformation[0] is placeholder
		// LevelUpInformation[1] = Level 1 Information
		// LevelUpInformation[2] = Level 1 Information
		if (LevelUpInformation.Num() - 1 <= Level) return Level;

		if (XP >= LevelUpInformation[Level].LevelUpRequirement)
		{
			++Level;
		}
		else
		{
			bSearching = false;
		}
	}

	return Level;
}
