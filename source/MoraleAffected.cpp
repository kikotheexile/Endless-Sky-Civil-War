/* MoraleAffected.cpp
Copyright (c) 2019 by Luke Arndt

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "MoraleAffected.h"
#include "Crew.h"
#include "DataNode.h"
#include "Files.h"
#include "GameData.h"
#include "PlayerInfo.h"

using namespace std;

// Loading

void MoraleAffected::Load(const DataNode &node)
{
	if(node.Size() >= 2)
		id = node.Token(1);
	
	for(const DataNode &child : node)
	{
		if(child.Size() >= 2)
		{
			if(child.Token(0) == "amount divided by crew members")
				amountDividedByCrewMembers = child.Value(1);
			else if(child.Token(0) == "amount per crew member")
				amountPerCrewMember = child.Value(1);
			else if(child.Token(0) == "flat amount")
				flatAmount = child.Value(1);
			else if(child.Token(0) == "message")
				message = child.Token(1);
			else
				child.PrintTrace("Skipping unrecognized attribute:");
		}
		else
			child.PrintTrace("Skipping incomplete attribute:");
	}
}

const MoraleAffected * MoraleAffected::GetMoraleAffected(const std::string &id)
{
	const MoraleAffected * moraleAffected = GameData::MoraleAffecteds().Get(id);
	if(!moraleAffected)
		Files::LogError("\nMissing \"morale affected\" definition: \"" + id + "\"");
	return moraleAffected;
}

// Public Static Functions

void MoraleAffected::CrewMemberDeath(
	const PlayerInfo &player,
	const shared_ptr<Ship> &ship,
	const int64_t deathCount
)
{
	if(!ship->IsDestroyed())
		AffectShipMorale(
			ship,
			GetMoraleAffected("death on ship"),
			deathCount
		);

	return AffectFleetMorale(
		player,
		GetMoraleAffected("death in fleet"),
		deathCount
	);
}



void MoraleAffected::EnemyShipDisabled(
	const PlayerInfo &player,
	const shared_ptr<Ship> &enemyShip
)
{
	return AffectFleetMorale(
		player,
		GetMoraleAffected("enemy ship disabled"),
		enemyShip->Cost()
	);
}



void MoraleAffected::FleetShipDestroyed(
	const PlayerInfo &player,
	const shared_ptr<Ship> &ship
)
{
	return AffectFleetMorale(
		player,
		GetMoraleAffected("fleet ship destroyed"),
		ship->Cost()
	);
}



void MoraleAffected::FleetShipDisabled(
	const PlayerInfo &player,
	const shared_ptr<Ship> &ship
)
{
	AffectShipMorale(
		ship,
		GetMoraleAffected("ship disabled"),
		ship->Cost()
	);

	return AffectFleetMorale(
		player,
		GetMoraleAffected("fleet ship disabled"),
		ship->Cost()
	);
}



double MoraleAffected::ProfitShared(
	const shared_ptr<Ship> &ship,
	const int64_t sharedProfit
)
{
	return AffectShipMorale(
		ship,
		GetMoraleAffected("profit shared"),
		sharedProfit
	);
}



double MoraleAffected::ProfitSharingDebtFailure(
	const std::shared_ptr<Ship> &ship,
	const int64_t missedPaymentAmount
)
{
	return AffectShipMorale(
		ship,
		GetMoraleAffected("profit sharing debt failure"),
		missedPaymentAmount
	);
}


double MoraleAffected::ProfitSharingDebtOwed(
	const std::shared_ptr<Ship> &ship,
	const int64_t debtAmount
)
{
	return AffectShipMorale(
		ship,
		GetMoraleAffected("profit sharing debt owed"),
		debtAmount
	);
}



double MoraleAffected::ProfitSharingDebtPayment(
	const std::shared_ptr<Ship> &ship,
	const int64_t paymentAmount
)
{
	return AffectShipMorale(
		ship,
		GetMoraleAffected("profit sharing debt payment"),
		paymentAmount
	);
}




void MoraleAffected::SalaryFailure(const PlayerInfo &player)
{
	const MoraleAffected * moraleAffected = GetMoraleAffected("salary failure");
	if(!moraleAffected) return;
	
	for(const shared_ptr<Ship> &ship : player.Ships())
	{
		AffectShipMorale(
			ship,
			moraleAffected,
			// Perhaps we should call `Crew::SalaryFailure` instead, and have that call this?
			Crew::SalariesForShip(
				ship,
				ship.get() == player.Flagship()
			)
		);
	}
}



void MoraleAffected::SalaryPayment(const PlayerInfo &player)
{
	const MoraleAffected * moraleAffected = GetMoraleAffected("salary payment");
		if(!moraleAffected) return;

	for(const shared_ptr<Ship> &ship : player.Ships())
	{
		AffectShipMorale(
			ship,
			moraleAffected,
			// Perhaps we should call `Crew::SalaryPayment` instead, and have that call this?
			Crew::SalariesForShip(
				ship,
				ship.get() == player.Flagship()
			)
		);
	}
}



// Private Static Functions

void MoraleAffected::AffectFleetMorale(
	const PlayerInfo &player,
	const MoraleAffected * moraleAffected,
	double multiplier
)
{
	if(
		moraleAffected->AmountDividedByCrewMembers() == 0. &&
		moraleAffected->AmountPerCrewMember() == 0. &&
		moraleAffected->FlatAmount() != 0.
	) {
		for(auto ship = player.Ships().begin(); ship != player.Ships().end(); ++ship)
			(*ship)->ChangeMorale(moraleAffected->FlatAmount());
		return;
	}

	for(auto ship = player.Ships().begin(); ship != player.Ships().end(); ++ship)
		AffectShipMorale(*ship, moraleAffected, multiplier);
}



double MoraleAffected::AffectShipMorale(
	const shared_ptr<Ship> &ship,
	const MoraleAffected * moraleAffected,
	double multiplier
)
{
	if(!moraleAffected || ship->Crew() <= 0) return 0.;

	double amount = 0.;

	amount += multiplier * moraleAffected->AmountDividedByCrewMembers() / ship->Crew();
	amount += multiplier * moraleAffected->AmountPerCrewMember() * ship->Crew();

	if(ship->IsParked())
		amount *= moraleAffected->ParkedMultiplier();
		
	amount += moraleAffected->FlatAmount();

	ship->ChangeMorale(amount);
	return amount;
}



// Public Instance Methods


const double &MoraleAffected::ParkedMultiplier() const
{
	return parkedMultiplier;
}



const double &MoraleAffected::AmountDividedByCrewMembers() const
{
	return amountDividedByCrewMembers;
}



const double &MoraleAffected::AmountPerCrewMember() const
{
	return amountPerCrewMember;
}



const double &MoraleAffected::FlatAmount() const
{
	return flatAmount;
}



const string &MoraleAffected::Id() const
{
	return id;
}



const string &MoraleAffected::Message() const
{
	return message;
}
