#include "Period.h"
#include "TableRow.h"

#include <string>

FTimedEvent FTimedEvent::ConvertRow(const FTableRow& Row, int32 ID)
{
	FTimedEvent Event;
	Event.ID = ID;

	if (Row.Fields.Num() == 0)
	{
		return Event;
	}

	for (auto& Field : Row.Fields)
	{
		if (Field.Key == "characterId")
		{
			double d;
			memcpy((void*)&d, Field.Value.Data.Get(), Field.Value.Size);

			Event.CharacterID = d;
		}

		else if (Field.Key == "name")
		{
			std::string desc;
			desc.resize(Field.Value.Size);

			memcpy((void*)desc.c_str(), Field.Value.Data.Get(), Field.Value.Size);
			Event.Name = UTF8_TO_TCHAR(desc.c_str());
		}

		else if (Field.Key == "periodId")
		{
			double d;
			memcpy((void*)&d, Field.Value.Data.Get(), Field.Value.Size);

			Event.PeriodID = d;
		}

		else if (Field.Key == "value")
		{
			double d;
			memcpy((void*)&d, Field.Value.Data.Get(), Field.Value.Size);

			Event.Value = d;
		}
	}

	return Event;
}

FPeriod FPeriod::ConvertRow(const FTableRow& Row, int32 ID)
{
	FPeriod Period;

	Period.Enum = (EPeriod)ID;
	Period.ID = ID;

	if (Row.Fields.Num() == 0)
	{
		return Period;
	}

	for (auto& Field : Row.Fields)
	{
		if (Field.Key == "hour")
		{
			double d;
			memcpy((void*)&d, Field.Value.Data.Get(), Field.Value.Size);

			Period.Hour = d;
		}

		else if (Field.Key == "minute")
		{
			double d;
			memcpy((void*)&d, Field.Value.Data.Get(), Field.Value.Size);

			Period.Minute = d;
		}

		else if (Field.Key == "name")
		{
			std::string desc;
			desc.resize(Field.Value.Size);

			memcpy((void*)desc.c_str(), Field.Value.Data.Get(), Field.Value.Size);
			Period.Name = UTF8_TO_TCHAR(desc.c_str());
		}
	}

	return Period;
}
