// Scorer for Qeff
//
// ********************************************************************
// *                                                                  *
// * Created by Villads J. 2022                                       *
// * For scoring proton dosis over an LET threshold
// *                                                                  *
// ********************************************************************
//

#include "Qeff.hh"
#include "TsParameterManager.hh"

#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"

Qeff::Qeff(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
														   G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
	:TsVBinnedScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
	SetUnit("");



}


Qeff::~Qeff()
{;}


G4bool Qeff::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}

	

	G4double stepLength = aStep->GetStepLength();
	if (stepLength <= 0.)
		return false;


	auto aTrack = aStep->GetTrack();
	G4double Energy = aTrack->GetKineticEnergy() / MeV;
	G4double Mass = aTrack->GetParticleDefinition()->GetPDGMass() / MeV;
	G4double beta = sqrt(1.0 - 1.0 / ( ((Energy / Mass) + 1) * ((Energy / Mass) + 1) ));
	G4int z = aTrack->GetParticleDefinition()->GetAtomicNumber();
	G4double Zeff = z * (1.0 - exp(-125.0 * beta * pow(abs(z), -2.0/3.0)));
	G4double Qeff = Zeff*Zeff/(beta*beta);

	// G4cout << z << G4endl;
	// G4cout << beta << G4endl;
	// G4cout << Mass << G4endl;
	
	if (Zeff != 0 && beta != 0){	
		AccumulateHit(aStep, Qeff);
		return true;		
	}
	return false;
}
