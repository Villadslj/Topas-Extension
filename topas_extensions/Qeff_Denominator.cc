// Scorer for Qeff_Denominator
//
// ********************************************************************
// *                                                                  *
// *                                                                  *
// * This file was obtained from Topas MC Inc under the license       *
// * agreement set forth at http://www.topasmc.org/registration       *
// * Any use of this file constitutes full acceptance of              *
// * this TOPAS MC license agreement.                                 *
// *                                                                  *
// ********************************************************************
//

#include "Qeff_Denominator.hh"

#include "TsParameterManager.hh"

#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"

Qeff_Denominator::Qeff_Denominator(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
														   G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
	:TsVBinnedScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
	SetUnit("");

	fStepCount = 0;

	// Dose-averaged or fluence-averaged Qeff definition
	G4String weightType = "dose";
	if (fPm->ParameterExists(GetFullParmName("WeightBy")))
		weightType = fPm->GetStringParameter(GetFullParmName("WeightBy"));
	weightType.toLower();

	if (weightType == "dose") {
		fDoseWeighted = true;
	} else if (weightType == "fluence" || weightType == "track") {
		fDoseWeighted = false;
	} else {
		G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
		G4cerr << GetFullParmName("WeightBy") << " refers to an unknown weighting: " << weightType << G4endl;
		exit(1);
	}


	// Upper cutoff to Qeff (used to fix spikes). Neglected if zero/negative.
	fMaxScoredQeff = 1e8;
	if (fPm->ParameterExists(GetFullParmName("MaxScoredQeff")))
		fMaxScoredQeff = fPm->GetDoubleParameter(GetFullParmName("MaxScoredQeff"), "");

}


Qeff_Denominator::~Qeff_Denominator()
{;}


G4bool Qeff_Denominator::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}

	G4int z = aStep->GetTrack()->GetParticleDefinition()->GetAtomicNumber();
	if (z < 1 || z == NULL){
		return false;

	}

	G4double stepLength = aStep->GetStepLength();
	if (stepLength <= 0.)
		return false;
	// Checks if particle is an ion.
	

	G4double eKin = aStep->GetTrack()->GetKineticEnergy();
	G4double density = aStep->GetPreStepPoint()->GetMaterial()->GetDensity();

	// If step is fluence-weighted, we can avoid a lot of logic
	G4bool isStepFluenceWeighted = !fDoseWeighted;
	if (isStepFluenceWeighted) {
		AccumulateHit(aStep, stepLength/mm);
		return true;
	}
	// otherwise we just have to calculate everything again


	// Compute Qeff
	auto aTrack = aStep->GetTrack();
	G4double Energy = eKin / MeV;
	G4double Mass = aTrack->GetParticleDefinition()->GetPDGMass() / MeV; //aTrack->GetParticleDefinition()->GetPDGMass()
	G4double beta = sqrt(1.0 - 1.0 / ( ((Energy / Mass) + 1) * ((Energy / Mass) + 1) ));
	G4double Zeff = z * (1.0 - exp(-125.0 * beta * pow(abs(z), -2.0/3.0)));
	G4double Qeff = Zeff*Zeff/(beta*beta);

		
		// If dose-weighted and not using PreStepLookup, only score Qeff if below MaxScoredQeff
	if (!isStepFluenceWeighted  || Qeff < fMaxScoredQeff) {
		AccumulateHit(aStep, eKin/MeV);
		return true;
	}
	
	return false;

}
