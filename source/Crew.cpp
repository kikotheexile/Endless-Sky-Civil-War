/* Crew.cpp
Copyright (c) 2019 by Luke Arndt

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "Crew.h"
#include "Files.h"
#include "GameData.h"
#include "PlayerInfo.h"

using namespace std;

void Crew::Load(const DataNode &node)
{
	if(node.Size() >= 2)
	{
		id = node.Token(1);
		name = id;
	}
	
	for(const DataNode &child : node)
	{
		if(child.Size() >= 2)
		{
			if(child.Token(0) == "name")
				name = child.Token(1);
			else if(child.Token(0) == "parked salary")
				parkedSalary = max((int)child.Value(1), 0);
			else if(child.Token(0) == "parked shares")
				parkedShares = max((int)child.Value(1), 0);
			else if(child.Token(0) == "place at")
				for(int crewNumber = 1; crewNumber < child.Size(); ++crewNumber)
					placeAt.push_back(max((int)child.Value(crewNumber), 0));
			else if(child.Token(0) == "ship population per member")
				shipPopulationPerMember = max((int)child.Value(1), 0);
			else if(child.Token(0) == "salary")
				salary = max((int)child.Value(1), 0);
			else if(child.Token(0) == "shares")
				shares = max((int)child.Value(1), 0);
			else
				child.PrintTrace("Skipping unrecognized attribute:");
		}
		else if(child.Token(0) == "avoids escorts")
			avoidsEscorts = true;
		else if(child.Token(0) == "avoids flagship")
			avoidsFlagship = true;
		else
			child.PrintTrace("Skipping incomplete attribute:");
	}
}



int64_t Crew::CalculateSalaries(const vector<shared_ptr<Ship>> &ships, const Ship * flagship, const bool includeExtras)
{
	int64_t totalSalaries = 0;

	for(const shared_ptr<Ship> &ship : ships)
	{
		totalSalaries += Crew::SalariesForShip(
			ship,
			ship.get() == flagship,
			includeExtras
		);
	}
	
	return totalSalaries;
}



int64_t Crew::CostOfExtraCrew(const vector<shared_ptr<Ship>> &ships, const Ship * flagship)
{
	// Calculate with and without extras and return the difference.
	return Crew::CalculateSalaries(ships, flagship, true)
		- Crew::CalculateSalaries(ships, flagship, false);
}



vector<pair<int64_t, string>> Crew::FleetSummary(const PlayerInfo &player)
{
	vector<pair<int64_t, string>> fleetSummary;
	
	// Add the total crew salaries to the fleet summary
	fleetSummary.push_back(make_pair(CalculateSalaries(player.Ships(), player.Flagship()), "crew salaries"));

	const Crew * playerCrew = GameData::Crews().Get("player");
	if(!playerCrew || playerCrew->Shares() <= 0)
	{
		Files::LogError("Missing player shares entry in crew data definitions; cannot use profit sharing");
		return fleetSummary;
	}

	// Total the shares of each ship in the fleet
	int64_t totalFleetShares = 0;
	for(const shared_ptr<Ship> &ship : player.Ships())
		totalFleetShares += SharesForShip(ship, ship.get() == player.Flagship());
	
	int64_t totalCrewShares = totalFleetShares - playerCrew->Shares();
	
	// Add the total crew shares to the fleet summary
	fleetSummary.push_back(make_pair(totalCrewShares, "crew shares"));
	
	// Add the player's shares to the fleet summary
	fleetSummary.push_back(make_pair(playerCrew->Shares(), "your shares"));

	// Add the current profit share % to the fleet summary
	fleetSummary.push_back(make_pair(totalCrewShares * 100 / totalFleetShares, "profit share %"));

	return fleetSummary;
}



int64_t Crew::NumberOnShip(const Crew &crew, const shared_ptr<Ship> &ship, const bool isFlagship, const bool includeExtras)
{	
	// If this is the flagship, check if this crew avoids the flagship.
	if(isFlagship && crew.AvoidsFlagship())
		return 0;
	// If this is an escort, check if this crew avoids escorts.
	if(!isFlagship && crew.AvoidsEscorts())
		return 0;
	
	const int64_t countableCrewMembers = includeExtras
		? ship->Crew()
		: ship->RequiredCrew();

	int64_t numberOnShip = 0;
	
	// Total up the placed crew members within the ship's countable crew
	for(int64_t crewNumber : crew.PlaceAt())
		if(crewNumber <= countableCrewMembers)
			++numberOnShip;
	
	// Prevent division by zero so that the universe doesn't implode.
	if(crew.ShipPopulationPerMember())
	{
		// Figure out how many of this kind of crew we have, by population.
		numberOnShip = max(
			numberOnShip,
			countableCrewMembers / crew.ShipPopulationPerMember()
		);
	}
	
	return numberOnShip;
}



int64_t Crew::SalariesForShip(const shared_ptr<Ship> &ship, const bool isFlagship, const bool includeExtras)
{
	// We don't need to pay dead people.
	if(ship->IsDestroyed())
		return 0;
	
	// Build a manifest of all of the crew members on the ship
	const map<const string, int64_t> manifest = ShipManifest(ship, isFlagship, includeExtras);
	
	// Sum up all of the crew's salaries
	// For performance, check if the ship is parked once, not every loop
	int64_t salariesForShip = 0;
	if(ship->IsParked())
		for(pair<const string, int64_t> entry : manifest)
			salariesForShip += GameData::Crews().Get(entry.first)->ParkedSalary() * entry.second;
	else
		for(pair<const string, int64_t> entry : manifest)
			salariesForShip += GameData::Crews().Get(entry.first)->Salary() * entry.second;
	
	return salariesForShip;
}



int64_t Crew::SharesForShip(const std::shared_ptr<Ship> &ship, const bool isFlagship, const bool includeExtras)
{
	// We don't need to pay dead people.
	if(ship->IsDestroyed())
		return 0;
	
	// Build a manifest of all of the crew members on the ship
	const map<const string, int64_t> manifest = ShipManifest(ship, isFlagship, includeExtras);

	int64_t sharesForShip = 0;
	// Sum up all of the crew's shares
	// For performance, check if the ship is parked once, not every loop
	if(ship->IsParked())
		for(pair<const string, int64_t> entry : manifest)
			sharesForShip += GameData::Crews().Get(entry.first)->ParkedShares() * entry.second;
	else
		for(pair<const string, int64_t> entry : manifest)
			sharesForShip += GameData::Crews().Get(entry.first)->Shares() * entry.second;
	
	return sharesForShip;
}



const map<const string, int64_t> Crew::ShipManifest(const shared_ptr<Ship> &ship, bool isFlagship, bool includeExtras)
{
	int64_t crewAccountedFor = 0;
	// A crew ID, the number on the ship, and their individual salary amount
	tuple<string, int64_t, int64_t> cheapestCrew;
	// Map of a crew ID to the crew type paired with the number on the ship
	map<const string, int64_t> manifest;
	
	// Check that we have crew data before proceeding
	if(GameData::Crews().size() < 1)
	{
		Files::LogError("Error: could not find any crew member definitions in the data files.");
		return manifest;
	}
	
	// Add up the salaries for all of the special crew members
	for(const pair<const string, Crew> &crewPair : GameData::Crews())
	{
		const Crew crew = crewPair.second;
		// Figure out how many of this type of crew are on this ship
		int numberOnShip = Crew::NumberOnShip(
			crew,
			ship,
			isFlagship,
			includeExtras
		);
		
		// Add the crew members to the manifest if there are any on the ship
		if(numberOnShip)
			manifest[crew.Id()] = numberOnShip;
		
		// Add the crew members to the total so far
		crewAccountedFor += numberOnShip;
		
		// If this is the cheapest crew type so far, keep track of it
		// Disqualify the player because there should be only one of those
		// Use non-parked salaries so that crew are consistent
		if(
			crew.Id() != "player" &&
			(
				get<0>(cheapestCrew).empty() ||
				crew.Salary() < get<2>(cheapestCrew)
			)
		)
			cheapestCrew = make_tuple(crew.Id(), numberOnShip, crew.Salary());
	}
	
	// Figure out how many crew members we still need to account for
	int64_t remainingCrewMembers = (includeExtras
			? ship->Crew()
			: ship->RequiredCrew()
		) - crewAccountedFor
		// If this is the flagship, one of the crew members is the player
		- isFlagship;
	
	// Fill out the ranks with the cheapest type of crew member
	manifest[get<0>(cheapestCrew)] = get<1>(cheapestCrew) + remainingCrewMembers;
	
	return manifest;
}



int64_t Crew::ShareProfit(const std::vector<std::shared_ptr<Ship>> &ships, const Ship * flagship, const int64_t grossProfit)
{
	if(grossProfit <= 0)
		return 0;
	
	const Crew * playerCrew = GameData::Crews().Get("player");
	if(!playerCrew || playerCrew->Shares() == 0)
		return 0;
	
	int64_t totalFleetShares = 0;
	
	for(const shared_ptr<Ship> &ship : ships)
	{
		totalFleetShares += Crew::SharesForShip(
			ship,
			ship.get() == flagship
		);
	}
	
	// If the player is the sole shareholder, return 0 directly.
	// This prevents us from sharing a small amount of profit due to
	// floating point rounding issues.
	if(playerCrew->Shares() == totalFleetShares)
		return 0;
	int64_t totalCrewShares = totalFleetShares - playerCrew->Shares();
	return grossProfit * totalCrewShares / (double)totalFleetShares;
}



bool Crew::AvoidsEscorts() const
{
	return avoidsEscorts;
}



bool Crew::AvoidsFlagship() const
{
	return avoidsFlagship;
}



int64_t Crew::ParkedSalary() const
{
	return parkedSalary;
}



int64_t Crew::ParkedShares() const
{
	return parkedShares;
}



int64_t Crew::Salary() const
{
	return salary;
}



int64_t Crew::Shares() const
{
	return shares;
}



int64_t Crew::ShipPopulationPerMember() const
{
	return shipPopulationPerMember;
}



const string &Crew::Id() const
{
	return id;
}



const string &Crew::Name() const
{
	return name;
}



const vector<int64_t> &Crew::PlaceAt() const
{
	return placeAt;
}
