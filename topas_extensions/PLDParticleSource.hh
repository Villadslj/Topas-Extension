//
// ********************************************************************
// *                                                                  *
// *                                                                  *
// * Created by Villads J. 2023                                       *
// * For including source files from ROOT                             *
// *                                                                  *
// ********************************************************************
//
#ifndef PLDParticleSource_hh
#define PLDParticleSource_hh

#include "TsVGenerator.hh"
#include <fstream>
#include <random>
#include <vector>
#include <string.h>

struct SpotElement {
    double positionX;
    double positionY;
    double meterSetWeight;
    double meterSetRate;
};

struct Layer {
    std::string spotID;
    double energy;
    double cumulativeMeterSetWeight;
	double SumMeterSetWeight;
    int numLayers;
	int numElements;
    int numPaintings;
    std::vector<SpotElement> elements;
};

class PLDParticleSource : public TsVGenerator
{
	
public:
	PLDParticleSource(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* pgM, G4String sourceName);
	~PLDParticleSource();

	void ResolveParameters();
	bool isBothSpace(char const &lhs, char const &rhs);
	void LoadPLDFile(const char*);

	void GeneratePrimaries(G4Event* );

	#define MyRandNGenerator() CLHEP::HepRandom::getTheEngine()->flat()
	
private:

	std::vector<std::vector<std::string>> parsedCsv;
	std::default_random_engine generator;
	G4double BeamPositionZ;
	G4double SpotFWHM, EnergySpread;
	G4String ParticleSourceFile;
	G4int linemax;

	std::vector<Layer> layers;
	double totalMU;
	double cumulativeMeterSetWeight;
	int numberOfLayers;

		
};
#endif