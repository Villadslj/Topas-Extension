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
	fNtuple->RegisterColumnS(&pAlive, "pAlive");

}


NuclearReactionScorer::~NuclearReactionScorer() {;}


G4bool NuclearReactionScorer::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
    if (!fIsActive) {
        fSkippedWhileInactive++;
        return false;
    }
    ResolveSolid(aStep);

	ClearParameters();

	auto endPoint = aStep->GetPostStepPoint();
	auto startPoint = aStep->GetPreStepPoint();
	auto process2 = const_cast<G4VProcess*>(endPoint->GetProcessDefinedStep());
  	auto process = const_cast<G4VProcess*>(endPoint->GetProcessDefinedStep());
	G4String nuclearChannel = "";
	// G4cout << process2->GetProcessName() << G4endl;
	nuclearChannel = process->GetProcessName() + " : ";
	processname = process->GetProcessName();
	auto hproc = const_cast<G4HadronicProcess*>(static_cast<const G4HadronicProcess *>(process));
	auto hproc2 = const_cast<G4HadronicProcess*>(static_cast<const G4HadronicProcess *>(process2));
	const G4Isotope* target = NULL;
	if (hproc && process->GetProcessType() == 4) {
		target = hproc->GetTargetIsotope();
	}
	// else if (hproc && process->GetProcessType() == 2)
	
	targetName = "XXXX";

	if (target) {
		// G4cout << hproc << G4endl;
		targetName = target->GetName();
	}
	projectile = aStep->GetTrack()->GetDefinition()->GetParticleName();
	if (projectile == "e+" || projectile == "e-" || processname == "ScoringZYBox" || processname == "ScoringZBox") return false;
	if (processname == "RadioactiveDecayBase"){
		targetName = "";
	}
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
		if(name == "e-") continue;
		
		if (first == 0){
			nuclearChannel += name;
			secondaries += name.substr(0,4);
			first += 1;
		} 
		else {
			nuclearChannel += " + " + name;
			secondaries +=  "+" + name.substr(0,4);
			first += 1;
		}
	}

	if (aStep->GetTrack()->GetTrackStatus() == fAlive) {
	nuclearChannel += " + " + aStep->GetTrack()->GetDefinition()->GetParticleName();
	G4String particleAlive = aStep->GetTrack()->GetDefinition()->GetParticleName();
	pAlive +=  particleAlive.substr(0,4);
	}
	
	// G4cout << nuclearChannel << G4endl;
	if (targetName == "B11" || targetName == "B10"){
		FillEmptyParm();
		fNtuple->Fill();
		return true;	
	}
	return false;

}

void NuclearReactionScorer::ClearParameters(){
	projectile = "";
    processname = "";
    targetName = "";
    secondaries = "";
    pAlive = "";

}

void NuclearReactionScorer::FillEmptyParm(){
	if (projectile == "") projectile = "empty";
	if (processname == "") processname = "empty";
	if (targetName == "") targetName = "empty";
	if (secondaries == "") secondaries = "empty";
	if (pAlive == "") pAlive = "empty";


}

