// Scorer for myHadronLET
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

#include "myHadronLET.hh"

#include "TsParameterManager.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4EmCalculator.hh"
#include "G4UIcommand.hh"

myHadronLET::myHadronLET(TsParameterManager *pM, TsMaterialManager *mM, TsGeometryManager *gM, TsScoringManager *scM, TsExtensionManager *eM,
							   G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
	: TsVBinnedScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
	SetUnit("MeV/mm/(g/cm3)");

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
	InstantiateSubScorer("myHadronLET_Denominator", outFileName, "Denominator");
}

myHadronLET::~myHadronLET() { ; }

G4bool myHadronLET::ProcessHits(G4Step *aStep, G4TouchableHistory *)
{
	if (!fIsActive)
	{
		fSkippedWhileInactive++;
		return false;
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

	if (dEdx > 0){
		AccumulateHit(aStep, weight * dEdx / density);
		return true;
	}

	return false;
}

G4int myHadronLET::CombineSubScorers()
{
	G4int counter = 0;

	TsVBinnedScorer *denomScorer = dynamic_cast<TsVBinnedScorer *>(GetSubScorer("Denominator"));
	for (G4int index = 0; index < fNDivisions; index++)
	{
		if (denomScorer->fFirstMomentMap[index] == 0.)
		{
			fFirstMomentMap[index] = 0;
			counter++;
		}
		else
		{
			fFirstMomentMap[index] = fFirstMomentMap[index] / denomScorer->fFirstMomentMap[index];
		}
	}

	return counter;
}
