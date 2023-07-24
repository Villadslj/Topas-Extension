// Scorer for Qeff
//
// ********************************************************************
// *                                                                  *

// *                                                                  *
// ********************************************************************
//

#include "Qeff.hh"

#include "TsParameterManager.hh"
#include <cmath>
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4EmCalculator.hh"
#include "G4UIcommand.hh"
using namespace std;



Qeff::Qeff(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
								   G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
: TsVBinnedScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
	SetUnit("");


	fElectronDefinition = G4ParticleTable::GetParticleTable()->FindParticle("e-");
	fProtonDefinition = G4ParticleTable::GetParticleTable()->FindParticle("proton");

	fStepCount = 0;
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
		includeAll = true;
	}

	// Dose-averaged or fluence-averaged LET definition
	G4String weightType = "dose";
	if (fPm->ParameterExists(GetFullParmName("WeightBy"))){
		weightType = fPm->GetStringParameter(GetFullParmName("WeightBy"));
		weightType.toLower();
	}
	if (weightType == "dose"){
		fDoseWeighted = true;
	}
	else if (weightType == "fluence" || weightType == "track"){
		fDoseWeighted = false;
	}
	else{
		G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
		G4cerr << GetFullParmName("WeightBy") << " refers to an unknown weighting: " << weightType << G4endl;
		exit(1);
	}
	
	// Instantiate subscorer needed for denominator
	InstantiateSubScorer("Qeff_Denominator", outFileName, "DenominatorQ");}

Qeff::~Qeff() {;}



G4bool Qeff::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
	
	if (!fIsActive)
	{
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
	G4String particleName =  particleDef -> GetParticleName();

	// atomic number
    G4int Z = particleDef -> GetAtomicNumber();
    if (Z<1) return false; // calculate only protons and ions

	G4double energyDeposit = aStep -> GetTotalEnergyDeposit();
	G4double DX = aStep -> GetStepLength();

	if (DX <= 0.)
		return false;

	G4double density = aStep->GetPreStepPoint()->GetMaterial()->GetDensity();
	//G4cout << "density" << density << G4endl;

 	// Get the pre-step kinetic energy
	G4double eKinPre = aStep -> GetPreStepPoint() -> GetKineticEnergy();
	// Get the post-step kinetic energy
	G4double eKinPost = aStep -> GetPostStepPoint() -> GetKineticEnergy();
	// Get the step average kinetic energy
	G4double eKinMean = (eKinPre + eKinPost) * 0.5;
	
	// get the material
	const G4Material * materialStep = aStep -> GetPreStepPoint() -> GetMaterial();
	
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
	
    // ICRU stopping power calculation
    G4EmCalculator emCal;
    // use the mean kinetic energy of ions in a step to calculate ICRU stopping power
    G4double dEdx = emCal.ComputeElectronicDEDX(eKinMean, particleDef, materialStep);
	
	// G4double total_energy_loss = EnergySecondary + energyDeposit; 
	G4double total_energy_loss = energyDeposit; 


	// get mass and energy
	G4double mass_MeV = theTrack -> GetParticleDefinition() -> GetPDGMass() / MeV;
	G4double energy_MeV =  eKinMean / MeV;

	// get gamma / beta
	G4double gamma = 1.0 + energy_MeV / mass_MeV;
	G4double beta = sqrt(1.0 - 1.0 / (gamma * gamma));

	// effective charge following Barkas
	G4int z = theTrack -> GetParticleDefinition() -> GetAtomicNumber();
	G4double z_eff = z * (1.0 - exp(-125.0 * beta * pow(abs(z), - 2.0 / 3.0)));
	G4double Q_eff = (z_eff * z_eff) / (beta * beta);
	



	G4double weight = 1.0;
	if (fDoseWeighted){
		// dose weighted;		
		weight *= total_energy_loss;
	}
	else{
		// track weighted;		
		weight *= DX;
	}

	if (dEdx > 0){
		AccumulateHit(aStep, weight * Q_eff);

		return true;
	}

	return false;
	
}



G4int Qeff::CombineSubScorers()
{
	G4int counter = 0;
	G4double QeffMax = 16000;
	TsVBinnedScorer* denomScorer = dynamic_cast<TsVBinnedScorer*>(GetSubScorer("DenominatorQ"));
	for (G4int index=0; index < fNDivisions; index++) {
		if (denomScorer->fFirstMomentMap[index]==0.) {
			fFirstMomentMap[index] = 0;
			counter++;
		} else {

			fFirstMomentMap[index] = fFirstMomentMap[index] / denomScorer->fFirstMomentMap[index];
			if (fFirstMomentMap[index] > QeffMax){
				G4cout << "fFirstMomentMap" << fFirstMomentMap[index] << G4endl;
			}
		}
	}

	return counter;
}