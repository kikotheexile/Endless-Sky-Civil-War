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
#include "PlayerInfo.h"

class MoraleAffected
{
public:
	// One or more crew members have died in the fleet
	// Triggers the following events:
	// "death on ship"
	// "death in fleet"; includes ship where death occurred
	// The amount is applied for each crew member that died
	static void CrewMemberDeath(
		const PlayerInfo &player,
		const std::shared_ptr<Ship> &ship,
		const int64_t deathCount
	);

	// The player owes the fleet profit shares and has missed an incremental payment
	// Triggers the following events:
	// "profit debt failure"
	// "profit debt failure while parked"
	// The amount is applied for each credit in the missed payment
	static void ProfitDebtFailure(
		const PlayerInfo &player,
		const std::shared_ptr<Ship> &ship,
		const int64_t sharedProfit
	);

	// The player owes the fleet profit shares and a day has passed
	// Triggers the following events:
	// "profit debt owed"
	// "profit debt owed while parked"
	// The amount is applied for each credit in the payment
	static void ProfitDebtOwed(
		const PlayerInfo &player,
		const std::shared_ptr<Ship> &ship,
		const int64_t sharedProfit
	);

	// The player owes the fleet profit shares and has made an incremental payment
	// Triggers the following events:
	// "profit debt payment"
	// "profit debt payment while parked"
	// The amount is applied for each credit in the payment
	static void ProfitDebtPayment(
		const PlayerInfo &player,
		const std::shared_ptr<Ship> &ship,
		const int64_t sharedProfit
	);

	// Profit has been shared with the crew on the ship as a lump sum
	// Triggers the following events:
	// "profit shared"
	// "profit shared while parked"
	// The amount is applied for each credit of shared profit
	static void ProfitShared(
		const PlayerInfo &player,
		const std::shared_ptr<Ship> &ship,
		const int64_t sharedProfit
	);

	// The player has paid all of the daily crew salaries
	// Triggers "salary payment" event
	// The amount is applied for each credit in the payment
	static void SalaryPayment(
		const PlayerInfo &player
	);

	// The player has missed a salary payment
	// Triggers "salary failure" event
	// The amount is applied for each credit in the missed payment
	static void SalaryFailure(
		const PlayerInfo &player
	);



	// Load a definition for a morale event
	void Load(
		const DataNode &node
	);

	// The ship's morale changes by this number divided by the number of crew members it has.
	// For events that are diffused across the ship's crew as a whole.
	// Often multiplied by other factors, such as a number of credits.
	// amount = amountDividedByCrewMembers / ship->Crew():
	double AmountDividedByCrewMembers() const;

	// The ship's morale changes by this much for each crew member it has.
	// For events that have a more intense effect on large ships.
	// Often multiplied by other factors, such as a number of credits.
	// amount = amountPerCrewMember * ship->Crew():
	double AmountPerCrewMember() const;

	// A flat amount that the event changes the ship's morale by.
	// Never multiplied by any other factors.
	double FlatAmount() const;

	// The id that the morale event is stored against in GameData::Crews()
	const std::string &Id() const;

	// A message to display when the morale event occurs
	const std::string &Message() const;

private:

	// Get a MoraleAffected from the GameData and log an error if it's missing
	static const MoraleAffected *GetMoraleAffected(
		const std::string &id
	);

	// Apply morale change to the whole fleet in response to crew death
	static void DeathInFleet(
		const PlayerInfo &player,
		const int64_t deathCount
	);

	// Apply morale change to the ship that crew members died on
	static double DeathOnShip(
		const PlayerInfo &player,
		const std::shared_ptr<Ship> &ship,
		const int64_t deathCount
	);

	// Apply morale change to an active ship for a successful salary payment
	static double ShipSalaryPaymentActive(
		const PlayerInfo &player,
		const std::shared_ptr<Ship> &ship
	);

	// Apply morale change to a parked ship for a successful salary payment
	static double ShipSalaryPaymentParked(
		const PlayerInfo &player,
		const std::shared_ptr<Ship> &ship
	);


	// Instance members; see accessor methods for details
	double amountDividedByCrewMembers = 0.;
	double amountPerCrewMember = 0.;
	double flatAmount = 0.;
	std::string id;
	std::string message;
};

#endif
