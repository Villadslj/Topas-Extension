//
// ********************************************************************
// *                                                                  *
// *                                                                  *
// * Created by Villads J. 2022                                       *
// * For scoring nuclear reaction						              *
// *                                                                  *
// ********************************************************************
//

#ifndef NuclearReactionScorer_hh
#define NuclearReactionScorer_hh

#include "TsVNtupleScorer.hh"

class NuclearReactionScorer : public TsVNtupleScorer
{
public:
    NuclearReactionScorer(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
                G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer);
    
    virtual ~NuclearReactionScorer();

    G4bool ProcessHits(G4Step*,G4TouchableHistory*);

private:
	// Output variables	
	G4String projectile;
    G4String processname;
    G4String targetName;
    G4String secondaries;
    G4int processType;
};
#endif
