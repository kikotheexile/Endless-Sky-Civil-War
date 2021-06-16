/* MoraleAffected.h
Copyright (c) 2021 by Luke Arndt

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef MORALE_AFFECTED_H_
#define MORALE_AFFECTED_H_

#include "DataNode.h"
#include "Ship.h"

using namespace std;

class MoraleAffected
{
public:
	// One or more crew members have died in the fleet
	// Triggers the following events:
	// Fleet + Ship: "death in fleet"
	// Ship: "death on ship"
	// The amount is multiplied by the number of crew members that died
	static void CrewMemberDeath(
		const vector< shared_ptr<Ship> > &fleet,
		const shared_ptr<Ship> &ship,
		const int64_t deathCount
	);

	// The player's fleet has disabled an enemy ship
	// Triggers the following events:
	// Fleet: "enemy ship disabled"
	// The amount is multiplied by the cost of the enemy ship
	static void EnemyShipDisabled(
		const vector< shared_ptr<Ship> > &fleet,
		const shared_ptr<Ship> &enemyShip
	);

	// One of the fleet's ships has been destroyed
	// Triggers the following events:
	// Fleet: "fleet ship destroyed"
	// The amount is multiplied by the cost of the ship.
	// The crew deaths are handled separately by CrewMemberDeath.
	static void FleetShipDestroyed(
		const vector< shared_ptr<Ship> > &fleet,
		const shared_ptr<Ship> &ship
	);

	// The player's fleet has disabled an enemy ship
	// Triggers the following events:
	// Fleet + Ship: "fleet ship disabled"
	// Ship: "ship disabled"
	// The amount is multiplied by the cost of the ship
	static void FleetShipDisabled(
		const vector< shared_ptr<Ship> > &fleet,
		const shared_ptr<Ship> &ship
	);

	// Profit has been shared with the crew on the ship as a lump sum
	// Triggers the following events:
	// Ship: "profit shared"
	// The amount is multiplied by the number of credits of shared profit
	// Returns the amount that morale has changed on the ship
	static double ProfitShared(
		const shared_ptr<Ship> &ship,
		const int64_t sharedProfit
	);

	// The player owes the fleet profit shares and has missed an incremental payment
	// Triggers the following events:
	// Ship: "profit sharing debt failure"
	// The amount is multiplied by the number of credits in the missed payment
	static double ProfitSharingDebtFailure(
		const shared_ptr<Ship> &ship,
		const int64_t missedPaymentAmount
	);

	// The player owes the fleet profit shares and a day has passed
	// Triggers the following events:
	// Ship: "profit sharing debt owed"
	// The amount is multiplied by the number of credits in the payment
	static double ProfitSharingDebtOwed(
		const shared_ptr<Ship> &ship,
		const int64_t profitShareDebtOwed
	);

	// The player owes the fleet profit shares and has made an incremental payment
	// Triggers the following events:
	// Ship: "profit sharing debt payment"
	// The amount is multiplied by the number of credits in the payment
	static double ProfitSharingDebtPayment(
		const shared_ptr<Ship> &ship,
		const int64_t sharedProfit
	);

	// The player has missed a salary payment
	// Triggers the following events:
	// Fleet: "salary failure"
	// The amount for each ship is multiplied by how many credits were missing in that ship's payment
	static double SalaryFailure(
		const shared_ptr<Ship> &ship,
		const int64_t missingCredits
	);

	// The player has paid all of the daily crew salaries
	// Triggers the following events:
	// Fleet: "salary payment"
	// The amount for each ship is multiplied by that ship's total salary payment
	static double SalaryPayment(
		const shared_ptr<Ship> &ship,
		const int64_t credits
	);



	// Load a definition for a morale affected record
	void Load(const DataNode &node);

	// Returns true if this MoraleAffected event does not apply to parked ships
	const bool &IgnoresParkedShips() const;

	// The ship's morale changes by this number divided by the number of crew members it has.
	// For events that are diffused across the ship's crew as a whole.
	// Often multiplied by other factors, such as a number of credits.
	// amount = amountDividedByCrewMembers / ship->Crew():
	const double &AmountDividedByCrewMembers() const;

	// The ship's morale changes by this much for each crew member it has.
	// For events that have a more intense effect on large ships.
	// Often multiplied by other factors, such as a number of credits.
	// amount = amountPerCrewMember * ship->Crew():
	const double &AmountPerCrewMember() const;

	// A flat amount that the event changes the ship's morale by.
	// For events that apply evenly to all ships, regardless of crew count.
	// Often multiplied by other factors, such as a number of credits.
	const double &FlatAmount() const;

	// If an affected ship is parked, multiply the morale change by this amount.
	// Defaults to 1.
	const double &ParkedMultiplier() const;

	// A message to display when the morale affected record is applied to the player's fleet.
	const string &FleetMessage() const;

	// The id that the morale affected record is stored against in GameData::Crews().
	const string &Id() const;

	// A message to display when the morale affected record is applied to a single ship.
	// If the record is applied to the whole fleet, this will be ignored.
	const string &ShipMessage() const;

private:

	static const bool SHOW_DEBUG_MESSAGES;

	// Get a MoraleAffected from the GameData and log an error if it's missing
	static const MoraleAffected *GetMoraleAffected(
		const string &id
	);

	// Apply a MoraleAffected event to a single ship
	// Returns the amount that it changed the morale by
	static void AffectFleetMorale(
		const vector< shared_ptr<Ship> > &fleet,
		const MoraleAffected * moraleAffected,
		const double multiplier
	);

	// Apply a MoraleAffected event to a single ship
	// Returns the amount that it changed the morale by
	static double AffectShipMorale(
		const shared_ptr<Ship> &ship,
		const MoraleAffected * moraleAffected,
		const double multiplier,
		const bool silenceMessage = false
	);

	// Build a message that we can display in response to a morale change
	static string BuildShipMessage(
		const MoraleAffected * moraleAffected,
		const shared_ptr<Ship> &ship,
		const string moraleDescription
	);

	// Supporting function that builds a sentence-linking word based on a given message
	static string GetLinkingWord(
		const string message
	);

	// Instance members; see accessor methods for details
	double amountDividedByCrewMembers = 0.;
	double amountPerCrewMember = 0.;
	double flatAmount = 0.;
	double parkedMultiplier = 1.;
	string fleetMessage;
	string id;
	string shipMessage;
};

#endif
