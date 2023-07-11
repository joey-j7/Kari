#include "ControlOverwriter.h"
#include "Components/InputComponent.h"
AControlOverwriter::AControlOverwriter()
{

}

void AControlOverwriter::BeginPlay()
{
	Super::BeginPlay();

	if (!Input)
		Input = NewObject<UInputComponent>();

	Input->BindAxis("MoveX", this, &AControlOverwriter::OnMoveX);
	Input->BindAxis("MoveY", this, &AControlOverwriter::OnMoveY);

	Input->BindAction("Confirm", EInputEvent::IE_Pressed, this, &AControlOverwriter::OnConfirm);
	Input->BindAction("Cancel", EInputEvent::IE_Pressed, this, &AControlOverwriter::OnCancel);
}

void AControlOverwriter::OvertakeControls()
{
	
}

void AControlOverwriter::RestoreControls()
{
}
