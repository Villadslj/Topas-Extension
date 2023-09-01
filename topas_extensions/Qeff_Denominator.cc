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

#include "G4EmCalculator.hh"

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

	// Checks if particle in scoring volume is part of the lists of desired particles to score Qeff
	const G4ParticleDefinition* PDef = aStep->GetTrack()->GetParticleDefinition();
	if (includeAll == false){
		if (std::find(p.begin(), p.end(), PDef) != p.end()){
		}
		else {
			return false;
		}

	}
	
	G4Track * theTrack = aStep  ->  GetTrack();
	G4ParticleDefinition *particleDef = theTrack -> GetDefinition();

	// atomic number
    G4int Z = particleDef -> GetAtomicNumber();
    if (Z < 1) return false; // calculate only protons and ions

	G4String particleName =  particleDef -> GetParticleName();
	G4double energyDeposit = aStep -> GetTotalEnergyDeposit();
	// G4double stepLength = aStep->GetStepLength();
	G4double DX = aStep -> GetStepLength();
	if (DX <= 0.)
		return false;
	
	// get the secondary paticles
	G4Step fstep = *theTrack -> GetStep();
	// store all the secondary partilce in current step
	const std::vector<const G4Track*> * secondary = fstep.GetSecondaryInCurrentStep();
	
	size_t SecondarySize = (*secondary).size();
	G4double EnergySecondary = 0.;
	
	// get secondary electrons energy deposited
	if (SecondarySize) // calculate only secondary particles
	{
		for (size_t numsec = 0; numsec< SecondarySize ; numsec ++)
		{
			//Get the PDG code of every secondaty particles in current step
			G4int PDGSecondary=(*secondary)[numsec]->GetDefinition()->GetPDGEncoding();
			
			if(PDGSecondary == 11) // calculate only secondary electrons
			{
				// calculate the energy deposit of secondary electrons in current step
				EnergySecondary += (*secondary)[numsec]->GetKineticEnergy();
			}
		}		
	}

	//indsat af Maria 
	// Get the pre-step kinetic energy
	G4double eKinPre = aStep -> GetPreStepPoint() -> GetKineticEnergy();
	// Get the post-step kinetic energy
	G4double eKinPost = aStep -> GetPostStepPoint() -> GetKineticEnergy();
	// Get the step average kinetic energy
	G4double eKinMean = (eKinPre + eKinPost) * 0.5;
	// get the material
	const G4Material * materialStep = aStep -> GetPreStepPoint() -> GetMaterial();
	 // ICRU stopping power calculation
    G4EmCalculator emCal;
	G4double dEdx = emCal.ComputeElectronicDEDX(eKinMean, particleDef, materialStep);

	// G4double total_energy_loss = EnergySecondary + energyDeposit; 
	G4double total_energy_loss = energyDeposit; 

	G4double weight = 1.0;
	if (fDoseWeighted){
		// G4cout << " dose weighted " << G4endl;		
		
		weight *= total_energy_loss;
		//G4cout << "weight nævner" << weight << G4endl;
	}
	else{
		// G4cout << " track weighted " << G4endl;		
		weight *= DX;
	}

	
	if (dEdx > 0){
		AccumulateHit(aStep, weight);
		return true;
	}

	return false;

}