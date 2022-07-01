// Scorer for CrossScorer
//
// ********************************************************************
// *                                                                  *
// *
// * Created by Villads J. 2022                                       *
// * For scoring nuclear reaction						              *
// *                                                                  *
// ********************************************************************
//

#include "CrossScorer.hh"

#include "TsTrackInformation.hh"
#include "G4PSDirectionFlag.hh"
#include "G4VProcess.hh"
#include "G4HadronicProcess.hh"
#include "G4ParticleTypes.hh"
#include "G4RunManager.hh"
#include "G4SteppingManager.hh"





CrossScorer::CrossScorer(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
                          G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
                         : TsVNtupleScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
	// pM->SetNeedsSteppingAction();
	// pM->SetNeedsTrackingAction();
	
	SetSurfaceScorer();
	fNtuple->RegisterColumnS(&processname, "processname");
	fNtuple->RegisterColumnS(&projectile, "projectile");
	fNtuple->RegisterColumnS(&targetName, "targetName");
	fNtuple->RegisterColumnS(&secondaries, "secondaries");
	fNtuple->RegisterColumnS(&pAlive, "pAlive");

	if (!fPm->ParameterExists(GetFullParmName("Target"))){
		DTarget = "";
	} else {
		DTarget = fPm->GetStringParameter(GetFullParmName("Target"));
	}

	if (!fPm->ParameterExists(GetFullParmName("Projectile"))){
		DProjectile = "";
	} else {
		DProjectile = fPm->GetStringParameter(GetFullParmName("Projectile"));
	}
	if (!fPm->ParameterExists(GetFullParmName("Secondaries"))){


	}
	if (!fPm->ParameterExists(GetFullParmName("Secondaries"))){
		slength=1;
		DSecondaries.push_back("Emptyspace");
	}
	if (fPm->ParameterExists(GetFullParmName("Secondaries"))){

		auto G4Secondaries = fPm->GetStringVector(GetFullParmName("Secondaries"));
		slength = fPm->GetVectorLength(GetFullParmName("Secondaries"));
		for (int i=0;i<slength;i++){
			DSecondaries.push_back(G4Secondaries[i]);
		}


	}


	if (!fPm->ParameterExists(GetFullParmName("Secondaries2"))){
		slength2=1;
		DSecondaries2.push_back("Emptyspace");
	}
	if (fPm->ParameterExists(GetFullParmName("Secondaries2"))){

		auto G4Secondaries2 = fPm->GetStringVector(GetFullParmName("Secondaries2"));
		slength2 = fPm->GetVectorLength(GetFullParmName("Secondaries2"));
		for (int i=0;i<slength;i++){
			DSecondaries2.push_back(G4Secondaries2[i]);
		}


	}

}


CrossScorer::~CrossScorer() {;}


G4bool CrossScorer::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{	

	ResolveSolid(aStep);
	ClearParameters();
    if (!fIsActive) {
        fSkippedWhileInactive++;
        return false;
    }

	// const G4TrackVector* secondary = fpSteppingManager->GetSecondary();
	auto endPoint = aStep->GetPostStepPoint();
	const G4VProcess* process=endPoint->GetProcessDefinedStep();
	G4String nuclearChannel = "";
	nuclearChannel = process->GetProcessName() + " : ";
	processname = process->GetProcessName();
	auto hproc = const_cast<G4HadronicProcess*>(static_cast<const G4HadronicProcess *>(process));
	const G4Isotope* target = NULL;
	if (hproc && process->GetProcessType() == 4) {
		target = hproc->GetTargetIsotope();
	}
	
	targetName = "XXXX";

	if (target) {
		// G4cout << hproc << G4endl;
		targetName = target->GetName();
	}
	projectile = aStep->GetTrack()->GetDefinition()->GetParticleName();
	// if (processname == "ScoringZYBox" || processname == "ScoringZBox") return false;
	if (processname == "RadioactiveDecayBase"){
		targetName = "";
	}
	nuclearChannel += aStep->GetTrack()->GetDefinition()->GetParticleName() + " + " + targetName + " -->";
	
	
	// secondaries
	auto secondary = aStep->GetSecondaryInCurrentStep();
	// auto secondary = aStep->GetfSecondary();
	G4String test = "";
	size_t size_secondary = (*secondary).size();
	if (!size_secondary)return false;

	auto vec_name = std::vector<G4String>();
	vec_name.clear();
	// check if neutron is produced:
	bool NeutronIsHere = false;
	for (auto  t : *secondary) {
	vec_name.push_back(t->GetDefinition()->GetParticleName());
	}
	
	std::sort(std::begin(vec_name ), std::end(vec_name ));
	int first = 0;
	for(auto  name : vec_name) {
		// if(name == "e-") continue;
		if(name == "neutron") NeutronIsHere = true;
		if (first == 0){
			nuclearChannel += name;
			test += name;
			secondaries += name.substr(0,4);
			first += 1;
		} 
		else {
			nuclearChannel += " + " + name;
			test += " + " + name;
			secondaries +=  "+" + name.substr(0,4);
		}
	}

	if (aStep->GetTrack()->GetTrackStatus() == fAlive) {
	nuclearChannel += " + " + aStep->GetTrack()->GetDefinition()->GetParticleName();
	G4String particleAlive = aStep->GetTrack()->GetDefinition()->GetParticleName();
	pAlive +=  particleAlive.substr(0,4);
	}
	



	// Filter reactions:

	if (targetName != DTarget && DTarget != "")return false;
	if (projectile != DProjectile && DProjectile!="")return false;
	if(!DSecondaries.empty() || !DSecondaries2.empty()){
		
		bool SecondariesTest = CheckSecondaries(DSecondaries,vec_name);
		bool SecondariesTest2 = CheckSecondaries(DSecondaries2,vec_name);
		if (SecondariesTest == false && SecondariesTest2 == false)return false;

	}
	// G4cout << nuclearChannel << G4endl;
	// G4cout <<"test:		" << aStep->GetNumberOfSecondariesInCurrentStep() << G4endl;
	// if (aStep->GetPostStepPoint()->GetStepStatus() == fGeomBoundary) G4cout<< "yes" << G4endl;
	// aStep->GetTrack()->DumpInfo();
	FillEmptyParm();
	fNtuple->Fill();
	return true;

}

void CrossScorer::ClearParameters(){
	projectile = "";
    processname = "";
    targetName = "";
    secondaries = "";
    pAlive = "";

}

void CrossScorer::FillEmptyParm(){
	if (projectile == "") projectile = "empty";
	if (processname == "") processname = "empty";
	if (targetName == "") targetName = "empty";
	if (secondaries == "") secondaries = "empty";
	if (pAlive == "") pAlive = "empty";


}

bool CrossScorer::CheckSecondaries(std::vector<G4String>& Test, std::vector<G4String>& real){
	
	std::sort(Test.begin(), Test.end());
    std::sort(real.begin(), real.end());
	bool goodtest = true;
	for (int i=0;i<slength;i++){
		if (!std::count(real.begin(), real.end(), Test[i])) {
        goodtest = false;
    	}
	}

    return goodtest;

}

