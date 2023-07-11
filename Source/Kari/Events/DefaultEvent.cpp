#include "DefaultEvent.h"
#include <Engine/World.h>

#include "General/DefaultGameInstance.h"

UDefaultGameInstance* UDefaultEvent::GetGameInstance() const
{
	return Cast<UDefaultGameInstance>(GetWorld()->GetGameInstance());
}
