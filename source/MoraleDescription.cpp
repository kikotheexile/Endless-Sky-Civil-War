/* MoraleDescription.cpp
Copyright (c) 2021 by Luke Arndt

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "MoraleDescription.h"
#include "DataNode.h"
#include "Files.h"
#include "GameData.h"

using namespace std;

// Loading

void MoraleDescription::Load(const DataNode &node)
{
	if(node.Size() >= 2)
		id = node.Token(1);
	
	for(const DataNode &child : node)
	{
		if(child.Size() >= 2)
		{
			if(child.Token(0) == "maximum morale")
				maximumMorale = child.Value(1);
			else if(child.Token(0) == "minimum morale")
				minimumMorale = child.Value(1);
			else if(child.Token(0) == "name")
				name = child.Token(1);
			else
				child.PrintTrace("Skipping unrecognized attribute:");
		}
		else
			child.PrintTrace("Skipping incomplete attribute:");
	}
}

const string MoraleDescription::GetMoraleDescription(
	const double moraleAmount
)
{
	for(
		map<string, MoraleDescription>::const_iterator it = GameData::MoraleDescriptions().begin();
		it != GameData::MoraleDescriptions().end();
	)
	{
		if(
			moraleAmount < it->second.MaximumMorale() &&
			moraleAmount >= it->second.MinimumMorale()
		)
			return it->second.Name();
		else
			++it;
	}

	Files::LogError(
		"Warning: No matching \"morale description\" for morale amount " +
		to_string(moraleAmount) +
		" in the game data."
	);

	return "(Missing)";
}


const double &MoraleDescription::MaximumMorale() const
{
	return maximumMorale;
}



const double &MoraleDescription::MinimumMorale() const
{
	return minimumMorale;
}



const string &MoraleDescription::Id() const
{
	return id;
}




const string &MoraleDescription::Name() const
{
	return name;
}
