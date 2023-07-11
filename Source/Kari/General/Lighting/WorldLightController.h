#pragma once

#include "CoreMinimal.h"

#include "Engine/DirectionalLight.h"

#include "Systems/Interfaces/Timeable.h"

#include "Curves/CurveLinearColor.h"
#include "Curves/CurveFloat.h"

#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

#include "WorldLightController.generated.h"

UCLASS()
class KARI_API AWorldLightController : public AActor, public ITimeable
{
	GENERATED_BODY()

public:
	AWorldLightController();

	void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void SetSunDirection(const FRotator& LightDirection, float Hour);

	UFUNCTION(BlueprintCallable)
	UTimeManager* GetTimeManager() const { return TimeManager; }

protected:
	UFUNCTION(BlueprintCallable)
	void UpdateLight(int32 Year = 2020, int32 Month = 4, int32 Day = 1, int32 Hour = 16, int32 Minute = 0, int32 Second = 0);

	void UpdateFog(float Hour);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateWorldSphere();

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = TimedLight)
	UCurveLinearColor* ColorCurve = nullptr;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = TimedLight)
	FLinearColor CurrentColor = FLinearColor::White;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = TimedLight)
	UCurveFloat* IntensityCurve = nullptr;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = TimedLight)
	float CurrentIntensity = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = TimedLight)
	float IntensityScalar = 1.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = TimedLight)
	float SkyIntensityScalar = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = TimedLight)
	float LightFlickerIntensity = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = TimedLight)
	UCurveFloat* ShadowIntensityCurve = nullptr;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = TimedLight)
	float CurrentShadowIntensity = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = TimedLight)
	UCurveFloat* SunlightStrengthCurve = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = TimedLight)
	UCurveFloat* MoonlightStrengthCurve = nullptr;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = TimedLight)
	float SunlightStrength = 1.f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = TimedLight)
	float MoonlightStrength = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = TimedLight)
	FRotator SunDirection = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = TimedLight)
	float Dot = 0;

	UPROPERTY(Category = TimedLight, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UDirectionalLightComponent* SunLight = nullptr;
	
	UPROPERTY(Category = TimedLight, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UDirectionalLightComponent* MoonLight = nullptr;

	UPROPERTY(Category = TimedLight, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USkyLightComponent* SkyLight = nullptr;

	UPROPERTY(Category = TimedLight, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FRotator MoonDirection = FRotator(0.f, 40.f, 320.f);

	UPROPERTY(Category = TimedLight, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float Longitude = 4.79427f;

	UPROPERTY(Category = TimedLight, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float Latitude = 51.590191f;

	UPROPERTY(Category = TimedLight, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float TimeZone = 1.0f;

	UPROPERTY(Category = TimedLight, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float NorthOffset = 0.0f;

	UPROPERTY(Category = TimedLight, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float Azimuth = 0.0f;

	UPROPERTY(Category = TimedLight, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float Elevation = 0.0f;
	
	UPROPERTY(Category = TimedLight, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float CorrectedElevation = 0.0f;

	UPROPERTY(Category = TimedLight, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool IsDaylightSavingTime = true;

	UPROPERTY(Category = TimedLight, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", ClampMin = 0, ClampMax = 23))
	int32 PreviewHour = 16;

	UPROPERTY(Category = TimedLight, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", ClampMin = 0, ClampMax = 59))
	int32 PreviewMinute = 0;

	UTimeManager* TimeManager = nullptr;

	// Fog settings
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = ExponentialHeightFog)
	UCurveFloat* FogDensity = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = ExponentialHeightFog)
	float FogHeightFalloff = 0.2f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = ExponentialHeightFog)
	FExponentialHeightFogData SecondFogData;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = ExponentialHeightFog)
	UCurveLinearColor* FogInscatteringColor = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = ExponentialHeightFog)
	float FogMaxOpacity = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = ExponentialHeightFog)
	float StartDistance = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = ExponentialHeightFog)
	float FogCutoffDistance = 0.0f;

	UExponentialHeightFogComponent* ExponentialHeightFogComponent;
};