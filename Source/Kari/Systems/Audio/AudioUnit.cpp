#include "AudioUnit.h"
#include "AudioManager.h"

float UAudioUnit::GetVolume() const
{
	if (Parent)
		return Volume * UserSetting * Modifier * Parent->GetVolume();

	return Volume * UserSetting;
}

void UAudioUnit::SetVolume(float V)
{
	if (Volume == V)
		return;

	Volume = V;
	Dirty = true;

	Manager->UpdateVolumes();
}

void UAudioUnit::SetModifier(float V)
{
	if (Modifier == V)
		return;

	Modifier = V;
	Dirty = true;

	Manager->UpdateVolumes();
}

void UAudioUnit::SetUserSetting(float V)
{
	if (UserSetting == V)
		return;

	UserSetting = V;
	Dirty = true;

	Manager->UpdateVolumes();
}

void UAudioUnit::SetParent(UAudioUnit* P)
{
	if (P == this)
		return;
	
	Parent = P;
	Dirty = true;

	Manager->UpdateVolumes();
}

void UAudioUnit::SaveUserSetting(FString Name)
{
	GConfig->SetFloat(TEXT("Audio"), *Name, UserSetting, GGameUserSettingsIni);
	GConfig->Flush(false);
}
