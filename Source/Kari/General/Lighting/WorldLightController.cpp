#include "WorldLightController.h"

#include "General/DefaultGameInstance.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "SunPosition.h"

#include "Core.h"

AWorldLightController::AWorldLightController()
{
	SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
	MoonLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("MoonLight"));

	MoonLight->SetRelativeRotation(FRotator::MakeFromEuler(FVector(180.f, 180.f, 180.f)));
	MoonLight->SetVisibility(false);

	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->CastShadows = false;

	ExponentialHeightFogComponent = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("ExponentialHeightFogComponent"));

#if WITH_EDITOR
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		UpdateLight(2020, 4, 1, PreviewHour, PreviewMinute, 0);
	}
#endif
}

void AWorldLightController::BeginPlay()
{
	Super::BeginPlay();

	UDefaultGameInstance* Instance = Cast<UDefaultGameInstance>(GetGameInstance());
	ensure(Instance);

	if (!Instance)
		return;

	TimeManager = Instance->TimeManager;
}

void AWorldLightController::SetSunDirection(const FRotator& LightDirection, float Hour)
{
	SunDirection = LightDirection;
	Dot = FVector::DotProduct(FVector::DownVector, LightDirection.Vector());

	SunLight->SetWorldRotation(LightDirection);
	MoonLight->SetWorldRotation(MoonDirection);

#if UE_BUILD_DEVELOPMENT
	if (!ColorCurve || !IntensityCurve || !ShadowIntensityCurve)
	{
		UE_LOG(LogTemp, Warning, TEXT("Color, intensity or shadow intensity curve have not been set in the WorldLightController!"));
		return;
	}
#endif

	CurrentColor = ColorCurve->GetLinearColorValue(Hour);
	CurrentIntensity = LightFlickerIntensity * IntensityCurve->GetFloatValue(Hour);
	CurrentShadowIntensity = ShadowIntensityCurve->GetFloatValue(Hour);
	MoonlightStrength = MoonlightStrengthCurve->GetFloatValue(Hour);
	SunlightStrength = SunlightStrengthCurve->GetFloatValue(Hour);

	const float SunlightIntensity = FMath::Lerp(0.f, CurrentIntensity, SunlightStrength) * IntensityScalar;
	const float MoonlightIntensity = FMath::Lerp(0.f, CurrentIntensity, MoonlightStrength) * IntensityScalar;
	const float SkylightIntensity = CurrentShadowIntensity * IntensityScalar * SkyIntensityScalar;

	/* Set Intensity */	
	SunLight->SetIntensity(SunlightIntensity);
	SunLight->SetLightColor(CurrentColor);
	SunLight->SetVisibility(SunlightStrength > 0.0f);

	MoonLight->SetIntensity(MoonlightIntensity);
	MoonLight->SetVisibility(MoonlightStrength > 0.0f);
	MoonLight->SetLightColor(CurrentColor);

	SkyLight->SetIntensity(SkylightIntensity);
	SkyLight->SetLightColor((FVector(CurrentColor) * 2.f).ComponentMin(FVector::OneVector));
}

void AWorldLightController::UpdateLight(int32 Year, int32 Month, int32 Day, int32 Hour, int32 Minute, int32 Second)
{
	FSunPositionData Data;
	USunPositionFunctionLibrary::GetSunPosition(Latitude, Longitude, TimeZone, IsDaylightSavingTime, Year, Month, Day, Hour, Minute, Second, Data);

	Elevation = Data.Elevation;
	CorrectedElevation = Data.CorrectedElevation;
	Azimuth = Data.Azimuth;

	const FRotator Direction = FRotator::MakeFromEuler(
		FVector(
			0.f,
			CorrectedElevation,
			Azimuth + NorthOffset
		)
	);

	float CurrentHour = (float)Hour + (float)Minute / 60.0f;

	SetSunDirection(Direction, CurrentHour);
	UpdateFog(CurrentHour);

	FEditorScriptExecutionGuard ScriptGuard;
	{
		UpdateWorldSphere();
	}
}

void AWorldLightController::UpdateFog(float Hour)
{
#if UE_BUILD_DEVELOPMENT
	if (!FogDensity || !FogInscatteringColor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Fog density or fog inscattering color curve have not been set in the WorldLightController!"));
		return;
	}
#endif

	const float CurrentFogDensity = FogDensity->GetFloatValue(Hour);
	const FLinearColor CurrentFogInscatteringColor = FogInscatteringColor->GetLinearColorValue(Hour);

	ExponentialHeightFogComponent->SetFogDensity(CurrentFogDensity);
	ExponentialHeightFogComponent->SetFogHeightFalloff(FogHeightFalloff);
	ExponentialHeightFogComponent->SecondFogData = SecondFogData;
	ExponentialHeightFogComponent->SetFogInscatteringColor(CurrentFogInscatteringColor);
	ExponentialHeightFogComponent->SetFogMaxOpacity(FogMaxOpacity);
	ExponentialHeightFogComponent->SetStartDistance(StartDistance);
	ExponentialHeightFogComponent->SetFogCutoffDistance(FogCutoffDistance);
}

#if WITH_EDITOR
void AWorldLightController::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	UpdateLight(2020, 4, 1, PreviewHour, PreviewMinute, 0);
}
#endif