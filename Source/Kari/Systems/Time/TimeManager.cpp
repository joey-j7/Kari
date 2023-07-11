#include "TimeManager.h"
#include "TimedActor.h"

#include "Events/TimeEvent.h"
#include "Systems/Components/TimeEventComponent.h"

struct FTimeEvent;
float UTimeManager::SecondsToDayTime = 1.f / SECONDS_PER_DAY;

UTimeManager::UTimeManager()
{
	Date = FDateTime::Now();
	SetTime(Date.GetHour(), Date.GetMinute(), Date.GetSecond());

	const FTableData& Per = GetTable("timePeriods");
	int32 i = 0;

	for (auto& Row : Per.Rows)
	{
		FPeriod P = FPeriod::ConvertRow(Row.Value, Row.Key);
		Periods.Emplace((EPeriod)i, P);

		i++;
	}

	const FTableData& Events = GetTable("timeEvents");
	i = 0;

	for (auto& Row : Events.Rows)
	{
		FTimedEvent Event = FTimedEvent::ConvertRow(Row.Value, Row.Key);
		TArray<FTimedEvent>* TimeEvent = PeriodEvents.Find((EPeriod)i);

		if (!TimeEvent)
		{
			TimeEvent = &PeriodEvents.Add((EPeriod)i);
		}

		TimeEvent->Add(Event);
		i++;
	}
	
	Periods.ValueSort([](const FPeriod& A, const FPeriod& B) {
		return A.Hour > B.Hour;
	});
}

void UTimeManager::Update(float DeltaTime)
{
	if (RealTimeSync)
	{
		Date = FDateTime::Now();
		CurrentTime = Date.ToUnixTimestamp() % SECONDS_PER_DAY;

		return;
	}

	CurrentTime += DeltaTime * Speed * Multiplier;

	while (CurrentTime > SECONDS_PER_DAY)
	{
		CurrentTime -= SECONDS_PER_DAY;
		CurrentDay++;
	}

	Date = FDateTime::Today() + FTimespan::FromSeconds(CurrentDay * SECONDS_PER_DAY + CurrentTime);
	UpdatePeriod();
}

void UTimeManager::Add(ATimedActor* Timeable)
{
	if (!Timeable)
		return;

	Timeables.AddUnique(Timeable);
	Timeable->OnPeriod(GetPeriod(), Timeable);
}

void UTimeManager::Remove(ATimedActor* Timeable)
{
	if (!Timeable || Timeables.Num() == 0)
		return;

	const int32 Find = Timeables.Find(Timeable);

	if (Find != INDEX_NONE)
		Timeables.RemoveAt(Find);
}

void UTimeManager::UseRealTime(bool Use)
{
	if (Use)
	{
		Speed = 1;

		Date = FDateTime::Now();
		CurrentTime = Date.ToUnixTimestamp() % SECONDS_PER_DAY;

		UpdatePeriod();
	}

	RealTimeSync = Use;
}

float UTimeManager::GetNormalizedDayTime() const
{
	return CurrentTime * SecondsToDayTime;
}

float UTimeManager::GetNormalizedPeriodTime() const
{
	switch (Period)
	{
	case E_NIGHT:
	{
		const float Start = FTimespan::FromHours(0).GetTotalSeconds();
		const float End = FTimespan::FromHours(6).GetTotalSeconds();
		const float Alpha = (CurrentTime - Start) / (End - Start);

		return FMath::Lerp(0.f, End - Start, Alpha) / (End - Start);
	}
	case E_MORNING:
	{
		const float Start = FTimespan::FromHours(6).GetTotalSeconds();
		const float End = FTimespan::FromHours(12).GetTotalSeconds();
		const float Alpha = (CurrentTime - Start) / (End - Start);

		return FMath::Lerp(0.f, End - Start, Alpha) / (End - Start);
	}
	case E_AFTERNOON:
	{
		const float Start = FTimespan::FromHours(12).GetTotalSeconds();
		const float End = FTimespan::FromHours(18).GetTotalSeconds();
		const float Alpha = (CurrentTime - Start) / (End - Start);

		return FMath::Lerp(0.f, End - Start, Alpha) / (End - Start);
	}
	case E_EVENING:
	{
		const float Start = FTimespan::FromHours(18).GetTotalSeconds();
		const float End = FTimespan::FromHours(24).GetTotalSeconds();
		const float Alpha = (CurrentTime - Start) / (End - Start);

		return FMath::Lerp(0.f, End - Start, Alpha) / (End - Start);
	}
	default:
		break;
	}

	return 0;
}

void UTimeManager::SetTimeMultiplier(float S)
{
	Multiplier = S;
}

void UTimeManager::SetTimeSpeed(float TimeSpeed)
{
	if (RealTimeSync || TimeSpeed < 0)
		return;

	Speed = TimeSpeed;
}

FString UTimeManager::GetTimeString(bool AddSeconds) const
{
	FString Hour = FString::FromInt(Date.GetHour());

	if (Hour.Len() < 2)
		Hour = "0" + Hour;

	FString Minute = FString::FromInt(Date.GetMinute());

	if (Minute.Len() < 2)
		Minute = "0" + Minute;

	FString Output = Hour + ":" + Minute;

	if (AddSeconds)
	{
		FString Seconds = FString::FromInt(Date.GetSecond());

		if (Seconds.Len() < 2)
			Seconds = "0" + Seconds;

		Output += ":" + Seconds;
	}

	return Output;
}

void UTimeManager::SetTime(int32 Hours, int32 Minutes, int32 Seconds)
{
	if (RealTimeSync)
		return;

	Hours = FMath::Abs(Hours) % 24;
	Minutes = FMath::Abs(Minutes) % 60;
	Seconds = FMath::Abs(Seconds) % 60;

	const FTimespan Time = FTimespan::FromHours(Hours) +
		FTimespan::FromMinutes(Minutes) +
		FTimespan::FromSeconds(Seconds);

	Date = FDateTime::FromUnixTimestamp(0) + Time;
	CurrentTime = Date.ToUnixTimestamp() % SECONDS_PER_DAY;

	UpdatePeriod();
}

void UTimeManager::ToggleTimeOfDay(EPeriod Period)
{
	const EPeriod period = Period == EPeriod::E_NUM ? GetPeriod() : Period;
	
	switch (period)
	{
	case E_MORNING:
	{
		SetTime(6);
		break;
	}
	case E_AFTERNOON:
	{
		SetTime(12);
		break;
	}
	case E_EVENING:
	{
		SetTime(18);
		break;
	}
	case E_NIGHT:
	{
		SetTime(0);
		break;
	}
	default:
		break;
	}
}

FString UTimeManager::GetDateString() const
{
	FString Day = FString::FromInt(Date.GetDay());

	if (Day.Len() < 2)
		Day = "0" + Day;

	FString Month = FString::FromInt(Date.GetMonth());

	if (Month.Len() < 2)
		Month = "0" + Month;

	const FString Year = FString::FromInt(Date.GetYear());

	return Day + "/" + Month + "/" + Year;
}

const FDateTime& UTimeManager::GetDate() const
{
	return Date;
}

void UTimeManager::Init()
{
	FirstFrame = true;
}

void UTimeManager::UpdateEvents()
{
	for (auto& Events : PeriodEvents)
	{
		if (Events.Key != Period)
			continue;

		for (auto& Event : Events.Value)
		{
			FTimeEvent TimeEvent;

			TimeEvent.PeriodID = Event.PeriodID;
			TimeEvent.CharacterID = Event.CharacterID;
			TimeEvent.EventName = Event.Name;
			TimeEvent.Value = Event.Value;

			UTimeEventComponent::OnTimeEventDelegate.Broadcast(TimeEvent, true);
		}
	}
}

void UTimeManager::UpdatePeriod()
{
	const EPeriod P = Period;

	for (auto& Per : Periods)
	{
		if (Date.GetHour() >= Per.Value.Hour && Date.GetMinute() >= Per.Value.Minute)
		{
			Period = Per.Key;
			break;
		}
	}

	if (Period == P)
	{
		if (FirstFrame)
		{
			UpdateEvents();
			FirstFrame = false;
		}

		return;
	}

	FirstFrame = false;

	switch (Period)
	{
	case E_NIGHT:
	{
		OnNight();
		break;
	}
	case E_MORNING:
	{
		OnMorning();
		break;
	}
	case E_AFTERNOON:
	{
		OnAfternoon();
		break;
	}
	case E_EVENING:
	{
		OnEvening();
		break;
	}
	default:
		break;
	}

	for (auto& Timeable : Timeables)
	{
		Timeable->OnPeriod(Period, Timeable);
	}

	OnPeriodChanged.Broadcast(Period);

	UpdateEvents();
}
