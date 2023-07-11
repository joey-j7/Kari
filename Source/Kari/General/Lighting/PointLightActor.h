#pragma once

#include "CoreMinimal.h"
#include "Engine/PointLight.h"

#include "Systems/Interfaces/Timeable.h"

#include "PointLightActor.generated.h"

UCLASS()
class KARI_API APointLightActor : public AActor, public ITimeable
{
	GENERATED_BODY()

public:
	APointLightActor() {}

	void OnMorning_Implementation() override;
	void OnEvening_Implementation() override;

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = TimedLight)
	bool ShowDuringDay = true;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = TimedLight)
	bool ShowDuringNight = true;
};