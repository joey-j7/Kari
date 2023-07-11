#if WITH_EDITOR
	#include "TableSelector.h"
	#include "TableComponent.h"

	#include "SWidget.h"
	#include "STextBlock.h"
	#include "SComboButton.h"
	#include "SSearchBox.h"

		#include "PropertyCustomizationHelpers.h"
		#include "PropertyEditing.h"


	#include "TableDatabase.h"
	#include "TableBinary.h"
	#include "SlateApplication.h"

	#include <cassert>
	#include "GameFramework/Actor.h"

	#define LOCTEXT_NAMESPACE "TableSelectorList"

	TSharedRef<IDetailCustomization> FTableSelector::MakeInstance()
	{
		return MakeShareable(new FTableSelector);
	}


	void FTableSelector::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
	{
		TArray<TWeakObjectPtr<UObject>> Objects;
		DetailBuilder.GetObjectsBeingCustomized(Objects);

		if (Objects[0].Get()->IsA(UTableComponent::StaticClass()))
			TableComponent = Cast<UTableComponent>(Objects[0].Get());
		else
			TableComponent = (UTableComponent*)Cast<AActor>(
				Objects[0].Get())->GetComponentByClass(UTableComponent::StaticClass()
			);

		auto Binary = TableDatabase::Get().GetBinary(TableComponent->GetTableName());

		const FString& LinkColumn = TableComponent->GetLinkedColumn();
		const uint32 LinkId = TableComponent->GetLinkedID();

		PopulatedList = LinkColumn != "" && LinkId != UINT_MAX ? Binary.PopulateWithLink(
			TableComponent->GetDropdownColumn(),
			LinkColumn,
			LinkId
		) : Binary.Populate(TableComponent->GetDropdownColumn());

		IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(
			"Database Properties",
			FText::FromString(TEXT("Database Properties")),
			ECategoryPriority::Important
		);

		IDHandle = DetailBuilder.GetProperty("ID");
		IDHandle->GetValue(CurrentID);
		DetailBuilder.HideProperty(IDHandle);

		const FSimpleDelegate OnChange = FSimpleDelegate::CreateLambda([=]
		{
			TableComponent->OnTableIDChanged();
		});

		IDHandle->SetOnPropertyValueChanged(OnChange);

		auto LinkedTableCols = DetailBuilder.GetProperty("LinkedTableColumns");
		DetailBuilder.HideProperty(LinkedTableCols);

		auto TableLnks = DetailBuilder.GetProperty("TableLinks");
		DetailBuilder.HideProperty(TableLnks);

		const FString* s = PopulatedList.Find(CurrentID);

		if (s)
		{
			CurrentItem = MakeShareable(new FString(*s));
		}
		else if (CurrentID != UINT_MAX)
		{
			CurrentItem = MakeShareable(new FString("[" + FString::FromInt(CurrentID) + "] [INVALID ENTRY]"));
		}

		Category.AddCustomRow(FText::FromString("Database Properties"), false)
		        .NameContent()
			[
				SNew(STextBlock)
				.Text(FText::FromString("Entity"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			.ValueContent()
			.MaxDesiredWidth(0.0f) // don't constrain the combo button width
			[
				SAssignNew(RowNameComboButton, SComboButton)
				.IsEnabled(!TableComponent->DropdownDisabled)
				.ToolTipText(this, &FTableSelector::GetCurrentItemLabel)
				.OnGetMenuContent(this, &FTableSelector::GetListContent)
				.OnComboBoxOpened(this, &FTableSelector::HandleMenuOpen)
				.ContentPadding(FMargin(2.0f, 2.0f))
				.ButtonContent()
				[
					SNew(STextBlock)
					.Text(this, &FTableSelector::GetCurrentItemLabel)
				]
			];
	}

	FText FTableSelector::GetCurrentItemLabel() const
	{
		if (!CurrentItem.IsValid())
			return FText::FromString("None");

		return FText::FromString(*CurrentItem);
	}

	TSharedRef<SWidget> FTableSelector::GetListContent()
	{
		SAssignNew(RowNameComboListView, SListView<TSharedPtr<FString>>)
			.ListItemsSource(&TableListNames)
			.OnSelectionChanged(this, &FTableSelector::OnSelectionChanged)
			.OnGenerateRow(this, &FTableSelector::HandleRowNameComboBoxGenarateWidget)
			.SelectionMode(ESelectionMode::Single);

		// Ensure no filter is applied at the time the menu opens
		OnFilterTextChanged(FText::GetEmpty());

		if (CurrentItem.IsValid())
		{
			RowNameComboListView->SetSelection(CurrentItem);
		}

		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()

			.AutoHeight()
			[
				SAssignNew(SearchBox, SSearchBox)
				.OnTextChanged(this, &FTableSelector::OnFilterTextChanged)
			]

			+ SVerticalBox::Slot()

			.FillHeight(1.f)
			[
				SNew(SBox)
				.MaxDesiredHeight(600)
				[
					RowNameComboListView.ToSharedRef()
				]
			];
	}

	void FTableSelector::HandleMenuOpen()
	{
		FSlateApplication::Get().SetKeyboardFocus(SearchBox);
	}

	TSharedRef<ITableRow> FTableSelector::HandleRowNameComboBoxGenarateWidget(TSharedPtr<FString> InItem,
	                                                                          const TSharedRef<STableViewBase>& OwnerTable)
	{
		return SNew(STableRow<TSharedPtr<FString>>, OwnerTable).Padding(FMargin(5.f, 5.f))
		[
			SNew(STextBlock).Text(FText::FromString(*InItem))
		];
	}

	void FTableSelector::OnSelectionChanged(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo)
	{
		if (SelectedItem.IsValid())
		{
			CurrentItem = SelectedItem;

			FString Left, Right;
			SelectedItem->Split(TEXT("]"), &Left, &Right);
			Left.RemoveAt(0);

			uint32_t i = FCString::Atoi(*Left);

			if (i != CurrentID)
			{
				CurrentID = i;
				IDHandle->SetValue(CurrentID);
			}

			// Close the combo
			RowNameComboButton->SetIsOpen(false);
		}
	}

	void FTableSelector::OnFilterTextChanged(const FText& InFilterText)
	{
		const FString CurrentFilterText = InFilterText.ToString();
		TableListNames.Empty();

		/** Get the properties we wish to work with */
		for (auto it = PopulatedList.begin(); it != PopulatedList.end(); ++it)
		{
			if (CurrentFilterText == TEXT("") || it.Value().Contains(CurrentFilterText))
			{
				TableListNames.Add(MakeShareable(new FString(it.Value())));
			}
		}

		RowNameComboListView->RequestListRefresh();
	}

	#undef LOCTEXT_NAMESPACE

#endif
