/* MoraleAffected.cpp
Copyright (c) 2021 by Luke Arndt

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "MoraleAffected.h"
#include "DataNode.h"
#include "Files.h"
#include "Format.h"
#include "GameData.h"
#include "Messages.h"

using namespace std;

const bool MoraleAffected::SHOW_DEBUG_MESSAGES = true;

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
			else if(child.Token(0) == "fleet message")
				fleetMessage = child.Token(1);
			else if(child.Token(0) == "parked multiplier")
				parkedMultiplier = child.Value(1);
			else if(child.Token(0) == "ship message")
				shipMessage = child.Token(1);
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
	const vector< shared_ptr<Ship> > &fleet,
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
		fleet,
		GetMoraleAffected("death in fleet"),
		deathCount
	);
}



void MoraleAffected::EnemyShipDisabled(
	const vector< shared_ptr<Ship> > &fleet,
	const shared_ptr<Ship> &enemyShip
)
{
	return AffectFleetMorale(
		fleet,
		GetMoraleAffected("enemy ship disabled"),
		enemyShip->Cost()
	);
}



void MoraleAffected::FleetShipDestroyed(
	const vector< shared_ptr<Ship> > &fleet,
	const shared_ptr<Ship> &ship
)
{
	return AffectFleetMorale(
		fleet,
		GetMoraleAffected("fleet ship destroyed"),
		ship->Cost()
	);
}



void MoraleAffected::FleetShipDisabled(
	const vector< shared_ptr<Ship> > &fleet,
	const shared_ptr<Ship> &ship
)
{
	AffectShipMorale(
		ship,
		GetMoraleAffected("ship disabled"),
		1
	);

	return AffectFleetMorale(
		fleet,
		GetMoraleAffected("fleet ship disabled"),
		ship->Crew()
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




double MoraleAffected::SalaryFailure(
	const shared_ptr<Ship> &ship,
	const int64_t missingCredits
)
{
	return AffectShipMorale(
		ship,
		GetMoraleAffected("salary failure"),
		missingCredits
	);
}



double MoraleAffected::SalaryPayment(
	const shared_ptr<Ship> &ship,
	const int64_t credits
)
{
	return AffectShipMorale(
		ship,
		GetMoraleAffected("salary payment"),
		credits
	);
}



// Private Static Functions

void MoraleAffected::AffectFleetMorale(
	const vector< shared_ptr<Ship> > &fleet,
	const MoraleAffected * moraleAffected,
	const double multiplier
)
{
	// If the morale affected record only specifies a flat amount, we can short-circuit the process.
	// This improves performance and allows us to provide a more precise message to the fleet.
	if(
		moraleAffected->AmountDividedByCrewMembers() == 0. &&
		moraleAffected->AmountPerCrewMember() == 0. &&
		moraleAffected->FlatAmount() != 0.
	)
	{
		double amount = moraleAffected->FlatAmount() * multiplier;

		for(auto shipIt = fleet.begin(); shipIt != fleet.end(); ++shipIt)
			(*shipIt)->ChangeMorale(amount);
		
		return;
	}

	// If the morale change includes non-flat amounts, we need to process it for each ship.
	double totalMoraleChange = 0.;

	for(auto shipIt = fleet.begin(); shipIt != fleet.end(); ++shipIt)
		totalMoraleChange += AffectShipMorale(*shipIt, moraleAffected, multiplier);

	string changeDescription = totalMoraleChange > 0 ? "increased" : "decreased";

	if(moraleAffected->FleetMessage().length() > 0)
	{
		string joiningWord = " your";

		if(moraleAffected->FleetMessage()[moraleAffected->FleetMessage().length() - 1] == '.')
			joiningWord = " Your";
		
		double averageAmount = abs(totalMoraleChange / fleet.size());
		
		if(SHOW_DEBUG_MESSAGES)
			Messages::Add(
				moraleAffected->FleetMessage() +
				joiningWord +
				" fleet's morale has "
				+ changeDescription +
				" by an average of " +
				to_string(averageAmount) +
				"."
			);
	}
}



double MoraleAffected::AffectShipMorale(
	const shared_ptr<Ship> &ship,
	const MoraleAffected * moraleAffected,
	const double multiplier,
	const bool silenceMessage
)
{
	if(!moraleAffected || ship->Crew() <= 0) return 0.;

	double amount = 0.;

	amount += moraleAffected->AmountDividedByCrewMembers() / ship->Crew();
	amount += moraleAffected->AmountPerCrewMember() * ship->Crew();
	amount += moraleAffected->FlatAmount();

	amount *= multiplier;

	if(ship->IsParked())
		amount *= moraleAffected->ParkedMultiplier();

	const string previousDescription = ship->MoraleDescription();

	ship->ChangeMorale(amount);

	const string description = ship->MoraleDescription();

	if(description != previousDescription)
	{
		Messages::Add(
			BuildShipMessage(
				moraleAffected,
				ship,
				description
			)
		);
	}

	if(SHOW_DEBUG_MESSAGES && !silenceMessage && moraleAffected->ShipMessage().length() > 0)
	{
		string joiningWord = " the";

		if(moraleAffected->ShipMessage()[moraleAffected->ShipMessage().length() - 1] == '.')
			joiningWord = " The";
			
		string changeDescription = amount > 0 ? "increased" : "decreased";

		Messages::Add(
			moraleAffected->ShipMessage() +
			joiningWord +
			" morale of the " +
			ship->Name() +
			" has " + changeDescription +
			" by " +
			to_string(abs(amount)) +
			"."
		);
	}

	return amount;
}



string MoraleAffected::BuildShipMessage(
	const MoraleAffected * moraleAffected,
	const shared_ptr<Ship> &ship,
	const string moraleDescription
)
{
	return (
		moraleAffected->ShipMessage() +
		GetLinkingWord(moraleAffected->ShipMessage()) +
		ship->Name() +
		" is now " +
		moraleDescription +
		"."
	);
}



string MoraleAffected::GetLinkingWord(
	const string message
)
{
	string linkingWord = "";

	if(message.length() > 0)
	{
		linkingWord += " ";

		if(message[message.length() - 1] == '.')
			linkingWord += "The ";
		else
			linkingWord += "the ";
	} else
		linkingWord += "The ";

	return linkingWord;
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




const string &MoraleAffected::FleetMessage() const
{
	return fleetMessage;
}



const string &MoraleAffected::Id() const
{
	return id;
}



const string &MoraleAffected::ShipMessage() const
{
	return shipMessage;
}
