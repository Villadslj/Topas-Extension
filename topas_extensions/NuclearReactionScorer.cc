// Scorer for NuclearReaction
//
// ********************************************************************
// *                                                                  *
// *
// * Created by Villads J. 2022                                       *
// * For scoring nuclear reaction						              *
// *                                                                  *
// ********************************************************************
//

#include "NuclearReactionScorer.hh"

#include "TsTrackInformation.hh"
#include "G4PSDirectionFlag.hh"
#include "G4VProcess.hh"
#include "G4HadronicProcess.hh"
#include "G4ParticleTypes.hh"
#include "G4RunManager.hh"


NuclearReactionScorer::NuclearReactionScorer(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
                          G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
                         : TsVNtupleScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
	pM->SetNeedsSteppingAction();
	pM->SetNeedsTrackingAction();

	fNtuple->RegisterColumnS(&processname, "processname");
	fNtuple->RegisterColumnS(&projectile, "projectile");
	fNtuple->RegisterColumnS(&targetName, "targetName");
	fNtuple->RegisterColumnS(&secondaries, "secondaries");

}


NuclearReactionScorer::~NuclearReactionScorer() {;}


G4bool NuclearReactionScorer::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
    if (!fIsActive) {
        fSkippedWhileInactive++;
        return false;
    }
    ResolveSolid(aStep);

	auto endPoint = aStep->GetPostStepPoint();
  	auto process = const_cast<G4VProcess*>(endPoint->GetProcessDefinedStep());
	G4String nuclearChannel = "";
	// G4cout << process->GetProcessName() << " " << process->GetProcessType() << G4endl;
	if( process->GetProcessName() == "protonInelastic" || process->GetProcessName() == "neutronInelastic"){ // || (process->GetProcessType() == 6)
		nuclearChannel = process->GetProcessName() + " : ";
		processname = process->GetProcessName();
		auto hproc = const_cast<G4HadronicProcess*>(static_cast<const G4HadronicProcess *>(process));
		const G4Isotope* target = NULL;
		if (hproc) {
			target = hproc->GetTargetIsotope();
		}
		targetName = "XXXX";

		if (target) {
			// G4cout << target << G4endl;
			targetName = target->GetName();
		}
		projectile = aStep->GetTrack()->GetDefinition()->GetParticleName();
		nuclearChannel += aStep->GetTrack()->GetDefinition()->GetParticleName() + " + " + targetName + " -->";
		// secondaries
		
		auto secondary = aStep->GetSecondaryInCurrentStep();
		auto vec_name = std::vector<G4String>();
		for (auto  t : *secondary) {
		vec_name.push_back(t->GetDefinition()->GetParticleName());
		}
		std::sort(std::begin(vec_name ), std::end(vec_name ));
		int first = 0;
		for(auto  name : vec_name) {
			secondaries +=  "+" + name;
			if (first == 0) nuclearChannel += name;
			else nuclearChannel += " + " + name;
			first += 1;
		}

		if (aStep->GetTrack()->GetTrackStatus() == fAlive) {
		nuclearChannel += " + " + aStep->GetTrack()->GetDefinition()->GetParticleName();
		secondaries +=  "+" + aStep->GetTrack()->GetDefinition()->GetParticleName();
		}
		
		G4cout << nuclearChannel << G4endl;
		fNtuple->Fill();
		return true;
	}

    return false;   
}
