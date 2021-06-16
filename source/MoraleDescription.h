/* MoraleDescription.h
Copyright (c) 2021 by Luke Arndt

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef MORALE_DESCRIPTION_H_
#define MORALE_DESCRIPTION_H_

#include "DataNode.h"

using namespace std;

class MoraleDescription
{
public:
	// Load a definition for a morale description record
	void Load(const DataNode &node);
	
	// Get a morale description name from the GameData and log an error if it's missing
	static const string GetMoraleDescription(
		const double moraleAmount
	);

	const string &Id() const;

protected:

	const double &MaximumMorale() const;
	const double &MinimumMorale() const;
	const string &Name() const;

private:


	double maximumMorale;
	double minimumMorale;
	string id;
  string name;
};

#endif
