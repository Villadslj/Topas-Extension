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

	fProtonDefinition = G4ParticleTable::GetParticleTable()->FindParticle("proton");
	fElectronDefinition = G4ParticleTable::GetParticleTable()->FindParticle("e-");

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

	includeAll = false;
	// Check which particles to include in TOPAS
	G4String* includeparticles;
	if (fPm->ParameterExists(GetFullParmName("includeparticles"))){
		includeparticles = fPm->GetStringVector(GetFullParmName("includeparticles"));
		G4int length = fPm->GetVectorLength(GetFullParmName("includeparticles"));
		for (G4int i = 0; i < length; i++) {
			G4ParticleDefinition* pdef = G4ParticleTable::GetParticleTable()->FindParticle(includeparticles[i]);
			p.push_back(pdef);
		}

	}
	else {
		G4cout << "All particles are included in Qeff scoring";
		includeAll = true;
	}

	// Upper cutoff to dE/dx (used to fix spikes). Neglected if zero/negative.
	fMaxScoredQeff = 0;
	if (fPm->ParameterExists(GetFullParmName("MaxScoredQeff")))
		fMaxScoredQeff = fPm->GetDoubleParameter(GetFullParmName("MaxScoredQeff"), "force per density");

	// Neglect secondary electrons in low density materials where the mean path length between discrete processes is
	// longer than the voxel width. Otherwise the Qeff is too high.
	fNeglectSecondariesBelowDensity = 0;
	if (fPm->ParameterExists(GetFullParmName("NeglectSecondariesBelowDensity")))
		fNeglectSecondariesBelowDensity = fPm->GetDoubleParameter(GetFullParmName("NeglectSecondariesBelowDensity"), "Volumic Mass");

	// Use fluence-weighted Qeff for low density materials in order to avoid rare spikes (a.k.a. pretty plot mode)
	// Disabled by default
	fUseFluenceWeightedBelowDensity = 0;
	if (fPm->ParameterExists(GetFullParmName("UseFluenceWeightedBelowDensity")))
		fUseFluenceWeightedBelowDensity = fPm->GetDoubleParameter(GetFullParmName("UseFluenceWeightedBelowDensity"), "Volumic Mass");
}


Qeff_Denominator::~Qeff_Denominator()
{;}


G4bool Qeff_Denominator::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}
	// Checks if particle in scoring volume is part of the lists of desired particles to score Qeff
	const G4ParticleDefinition* PDef = aStep->GetTrack()->GetParticleDefinition();
	if (includeAll == false){
		if (std::find(p.begin(), p.end(), PDef) != p.end()){
		}
		else {
			return false;
		}

	}

		G4double stepLength = aStep->GetStepLength();
		if (stepLength <= 0.)
			return false;

		G4double eDep = aStep->GetTotalEnergyDeposit();
		G4double density = aStep->GetPreStepPoint()->GetMaterial()->GetDensity();

		// If step is fluence-weighted, we can avoid a lot of logic
		G4bool isStepFluenceWeighted = !fDoseWeighted || density < fUseFluenceWeightedBelowDensity;
		if (isStepFluenceWeighted) {
			AccumulateHit(aStep, stepLength/mm);
			return true;
		}
		// otherwise we just have to calculate everything again



		// Add energy deposited by secondary electrons (unless NeglectSecondariesBelowDensity is used)
		if (density > fNeglectSecondariesBelowDensity) {
			const G4TrackVector* secondary = aStep->GetSecondary();
			if (!secondary) {
				secondary = aStep->GetTrack()->GetStep()->GetSecondary();  // parallel worlds
			}
			if (secondary) {
				G4int diff;
				if (fStepCount == 0) diff = 0;
				else diff = secondary->size() - fStepCount;

				fStepCount = (*secondary).size();

				for (unsigned int i=(*secondary).size()-diff; i<(*secondary).size(); i++)
					if ((*secondary)[i]->GetParticleDefinition() == fElectronDefinition)
						eDep += (*secondary)[i]->GetKineticEnergy();
			}
		}

		// Compute Qeff
		auto aTrack = aStep->GetTrack();
		G4double Energy = eDep / MeV;
		G4double Mass = aTrack->GetParticleDefinition()->GetPDGMass() / MeV; //aTrack->GetParticleDefinition()->GetPDGMass()
		G4double beta = sqrt(1.0 - 1.0 / ( ((Energy / Mass) + 1) * ((Energy / Mass) + 1) ));
		G4int z = aTrack->GetParticleDefinition()->GetAtomicNumber();; //aTrack->GetParticleDefinition()->GetAtomicNumber();
		G4double Zeff = z * (1.0 - exp(-125.0 * beta * pow(abs(z), -2.0/3.0)));
		G4double Qeff = Zeff*Zeff/(beta*beta);

		// If dose-weighted and not using PreStepLookup, only score Qeff if below MaxScoredQeff
	if (isStepFluenceWeighted || fMaxScoredQeff <= 0 || Qeff / density < fMaxScoredQeff) {
		AccumulateHit(aStep, eDep/MeV);
		return true;
	}
	
	return false;

}
