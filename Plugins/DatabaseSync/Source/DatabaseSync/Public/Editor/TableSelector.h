#pragma once

#if WITH_EDITOR
	#include "IDetailCustomization.h"

	#include "PropertyHandle.h"

	#include "TableComponent.h"
	#include "SListView.h"

	class SComboButton;

	struct FTableListData
	{
		uint32 ID;
		FString Name = "Untitled";
	};

	class FTableSelector : public IDetailCustomization
	{
	public:
		virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
		
		static TSharedRef< IDetailCustomization > MakeInstance();

	private:
		FText GetCurrentItemLabel() const;
		TSharedRef<SWidget> GetListContent();
		void HandleMenuOpen();

		/** Return the representation of the the row names to display */
		TSharedRef<ITableRow> HandleRowNameComboBoxGenarateWidget(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable);

		/** Update the root data on a change of selection */
		void OnSelectionChanged(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo);

		/** Called by Slate when the filter box changes text. */
		void OnFilterTextChanged(const FText& InFilterText);

		// TArray<TSharedPtr<FTableListData>> TableListData;
		TArray<TSharedPtr<FString>> TableListNames;

		/** The comboButton objects */
		TSharedPtr<SComboButton> RowNameComboButton;
		TSharedPtr<class SSearchBox> SearchBox;
		TSharedPtr<SListView<TSharedPtr<FString>>> RowNameComboListView;

		TMap<uint32, FString> PopulatedList;

		TSharedPtr<IPropertyHandle> IDHandle;
		TWeakObjectPtr<UTableComponent> TableComponent;

		TSharedPtr<FString> CurrentItem;
		uint32 CurrentID = 0;
	};
#endif WITH_EDITOR