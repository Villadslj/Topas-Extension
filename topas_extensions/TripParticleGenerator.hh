//
// ********************************************************************
// *                                                                  *
// *                                                                  *
// * Created by Villads J. 2022                                       *
// * For including source files from trip                             *
// *                                                                  *
// ********************************************************************
//
#ifndef TripParticleGenerator_hh
#define TripParticleGenerator_hh

#include "TsVGenerator.hh"
#include <G4ParticleGun.hh>
#include <fstream>
#include <CLHEP/Random/RandFlat.h>
#include <random>

class TripParticleGenerator : public TsVGenerator
{
public:
	TripParticleGenerator(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* pgM, G4String sourceName);
	~TripParticleGenerator();

	void ResolveParameters();
	bool isBothSpace(char const &lhs, char const &rhs);

	void GeneratePrimaries(G4Event* );

	#define MyRandNGenerator() CLHEP::HepRandom::getTheEngine()->flat()
	
private:

	std::vector<std::vector<std::string>> parsedCsv;
	std::default_random_engine generator;
	G4double BeamPositionZ;
	G4String ParticleSourceFile;
	G4int linemax;
	
	
};
#endif