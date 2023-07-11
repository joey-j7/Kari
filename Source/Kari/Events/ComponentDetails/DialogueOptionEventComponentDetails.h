﻿#pragma once

#if WITH_EDITOR
#include "DetailLayoutBuilder.h"
#include "IDetailCustomization.h"
#include "IPropertyTypeCustomization.h"

class FDialogueOptionEventComponentDetails : public IDetailCustomization
{
public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	void OnItemSelected(TSharedPtr<FString> ItemSelected, ESelectInfo::Type SelectInfo);
	
	static TSharedRef<IDetailCustomization> MakeInstance();

	TWeakObjectPtr<class UDialogueOptionEventComponent> DialogueOptionEventComponent;

private:
	TArray<TSharedPtr<FString>> EventNames;
	int SelectedEventNameID;
};
#endif
