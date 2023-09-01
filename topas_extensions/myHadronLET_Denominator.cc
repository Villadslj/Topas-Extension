// Scorer for myHadronLET_Denominator
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

#include "myHadronLET_Denominator.hh"

#include "TsParameterManager.hh"

#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"

myHadronLET_Denominator::myHadronLET_Denominator(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
														   G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
	:TsVBinnedScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
	SetUnit("");

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
}


myHadronLET_Denominator::~myHadronLET_Denominator()
{;}


G4bool myHadronLET_Denominator::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}

	
	G4Track * theTrack = aStep  ->  GetTrack();
	G4ParticleDefinition *particleDef = theTrack -> GetDefinition();

	// atomic number
    G4int Z = particleDef -> GetAtomicNumber();
    if (Z<1) return false; // calculate only protons and ions

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

	G4double total_energy_loss = EnergySecondary + energyDeposit; 
	// G4double total_energy_loss = energyDeposit; 

    // // Total LET calculation...
    // totalLetD[voxel]  += (DE + DEEletrons) * Lsn; // total dose Let Numerator, including secondary electrons energy deposit
    // DtotalLetD[voxel] += DE + DEEletrons;         // total dose Let Denominator, including secondary electrons energy deposit
    // totalLetT[voxel]  += DX * Lsn;                // total track Let Numerator
    // DtotalLetT[voxel] += DX;                      // total track Let Denominator

	G4double weight = 1.0;
	if (fDoseWeighted){
		// G4cout << " dose weighted " << G4endl;		
		weight *= total_energy_loss;
	}
	else{
		// G4cout << " track weighted " << G4endl;		
		weight *= DX;
	}

	AccumulateHit(aStep, weight);
	return true;

}
