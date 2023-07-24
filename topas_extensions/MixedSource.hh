//
// ********************************************************************
// *                                                                  *
// *                                                                  *
// * Created by Villads J. 2022                                       *
// * For including source files from trip                             *
// *                                                                  *
// ********************************************************************
//
#ifndef MixedSource_hh
#define MixedSource_hh

#include "TsVGenerator.hh"
#include <G4ParticleGun.hh>
#include <fstream>
#include <CLHEP/Random/RandFlat.h>
#include <random>

class MixedSource : public TsVGenerator
{
public:
	MixedSource(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* pgM, G4String sourceName);
	~MixedSource();

	void ResolveParameters();
	bool isBothSpace(char const &lhs, char const &rhs);

	void GeneratePrimaries(G4Event* );

	#define MyRandNGenerator() CLHEP::HepRandom::getTheEngine()->flat()
	
private:

	std::vector<std::vector<std::string>> parsedCsv;
	std::default_random_engine generator;
	G4double BeamPositionZ, SpotSize;
	G4String ParticleSourceFile;
	G4int linemax;
	std::vector<G4String> DSecondaries;
	std::vector<G4double> DMixingFraction, DMonoEnergies;
	
	
};
#endif