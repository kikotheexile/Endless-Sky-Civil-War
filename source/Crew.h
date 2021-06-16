/* Crew.h
Copyright (c) 2019 by Luke Arndt

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef CREW_H_
#define CREW_H_

#include "Ship.h"

using namespace std;

class Crew
{
public:
	// Calculate the proportion of profit that must be shared with the fleet
	// To actually share profit, call Crew::ShareProfit instead.
	static double CalculateProfitShareRatio(
		const vector< shared_ptr<Ship> > &fleet,
		const Ship * flagship
	);
	
	// Calculate one day's salaries for the Player's fleet
	static int64_t CalculateSalaries(
		const vector< shared_ptr<Ship> > &fleet,
		const Ship * flagship,
		const bool includeExtras = true
	);
	
	// Calculate the total cost of the flagship's extra crew
	static int64_t CostOfExtraCrew(
		const vector< shared_ptr<Ship> > &fleet,
		const Ship * flagship
	);

	// Calculate the average morale of the fleet and return a description.
	// If you already know the total number of crew in the fleet,
	// you can pass it as an argument to prevent it from being recalculated.
	static string FleetMoraleDescription(
		const vector< shared_ptr<Ship> > &fleet,
		const int64_t totalFleetCrew = 0
	);

	// Generate a fleet summary for display
	static vector< pair<int64_t, string> > FleetSummary(
		const vector< shared_ptr<Ship> > &fleet,
		const Ship * flagship
	);
	
	// Figure out how many of a given crew member are on a ship
	static int64_t NumberOnShip(
		const Crew &crew,
		const shared_ptr<Ship> &ship,
		const bool isFlagship,
		const bool includeExtras = true
	);

	// Return the total number of profit shares for a ship
	static int64_t SharesForShip(
		const shared_ptr<Ship> &ship,
		const bool isFlagship,
		const bool includeExtras = true
	);

	// Calculate one day's salaries for a ship
	static int64_t SalariesForShip(
		const shared_ptr<Ship> &ship,
		const bool isFlagship,
		const bool includeExtras = true
	);

	// Share profits the fleet. Returns how many credits were distributed.
	// Calling this function triggers a MoraleAffected::ProfitShared event
	// for each ship that received a share of the profits.
	// If shareEverything is true, distributes the entire amount, ignoring the player's share.
	// If shareEverything is false, avoids distributing the player's share.
	static int64_t ShareProfit(
		const vector< shared_ptr<Ship> > &fleet,
		const Ship * flagship,
		const int64_t credits,
		const bool distributeEverything = false
	);

	static const string PROFIT_SHARING_DEBT_FAILURE;
	static const string PROFIT_SHARING_DEBT_PAYMENT;

	// Process one day's worth of profit sharing debt.
	// Calling this function triggers a MoraleAffected event for each ship, based on the eventType:
	// PROFIT_SHARING_DEBT_FAILURE: MoraleAffected::ProfitSharingDebtFailure
	// PROFIT_SHARING_DEBT_PAYMENT: MoraleAffected::ProfitSharingDebtPayment
	// It also triggers a MoraleAffected::ProfitSharingDebtOwed event.
	static void ProcessProfitSharingDebt(
		const vector< shared_ptr<Ship> > &fleet,
		const Ship * flagship,
		const int64_t missedPayment,
		const int64_t principal,
		const string eventType
	);

	static const string SALARY_FAILURE;
	static const string SALARY_PAYMENT;

	// Process one day's worth of salaries.
	// Calling this function triggers a MoraleAffected event for each ship, based on the eventType: 
	// SALARY_FAILURE: MoraleAffected::SalaryFailure
	// SALARY_PAYMENT: MoraleAffected::SalaryPayment
	static void ProcessSalaries(
		const vector< shared_ptr<Ship> > &fleet,
		const Ship * flagship,
		const double proportionPaid,
		const string eventType
	);

	// List the crew members on a ship, and how many there are of each type
	static const map<const string, int64_t> ShipManifest(
		const shared_ptr<Ship> &ship,
		bool isFlagship,
		bool includeExtras = true
	);

	// Load a definition for a crew member.
	void Load(const DataNode &node);
	
	bool AvoidsEscorts() const;
	bool AvoidsFlagship() const;
	int64_t ParkedSalary() const;
	int64_t ParkedShares() const;
	int64_t Salary() const;
	int64_t Shares() const;
	int64_t ShipPopulationPerMember() const;
	const string &Id() const;
	const string &Name() const;
	const vector<int64_t> &PlaceAt() const;

private:
	// If true, the crew member will not appear on escorts
	bool avoidsEscorts = false;
	// If true, the crew member will not appear on the flagship
	bool avoidsFlagship = false;
	// The number of credits paid daily while parked (minimum 0)
	int64_t parkedSalary = 0;
	// The crew member's profit shares while parked (minimum 0)
	int64_t parkedShares = 0;
	// The number of credits paid daily (minimum 0)
	int64_t salary = 0;
	// The crew member's shares in the fleet's profits (minimum 0)
	int64_t shares = 0;
	// Every nth crew member on the ship will be this crew member
	int64_t shipPopulationPerMember = 0;
	// The id that the crew member is stored against in GameData::Crews()
	string id;
	// The display name for this kind of crew members (plural, Title Case)
	string name;
	// The crew member will be placed at these crew member numbers if possible
	// Note: if multiple crew definitions claim the same crew positions,
	// we can end up paying for more crew than we expect to.
	// To avoid this, don't place different crew members in the same spots.
	// Example usage: "place at" 1 3 5 7 13
	vector<int64_t> placeAt;
};

#endif
