#pragma once
#if WITH_EDITOR
#include "DetailLayoutBuilder.h"
#include "IDetailCustomization.h"
#include "IPropertyTypeCustomization.h"

class FTimeEventComponentDetails : public IDetailCustomization
{
public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	void OnItemSelected(TSharedPtr<FString> ItemSelected, ESelectInfo::Type SelectInfo);
	
	static TSharedRef<IDetailCustomization> MakeInstance();

	TWeakObjectPtr<class UTimeEventComponent> TimeEventComponent;

private:
	TArray<TSharedPtr<FString>> EventNames;
	int SelectedEventNameID;
};
#endif