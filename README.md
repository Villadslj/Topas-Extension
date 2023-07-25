# Topas-Extension

Welcome to this Topas-Extension repository, which contains various extensions developed over time for use with TOPAS - a Monte Carlo simulation software for particle therapy. Below, you will find descriptions of each extension and how to include them in your local TOPAS version. We encourage you to contribute your own cool extensions to this project.

## How to include extensions in your TOPAS version

To include these extensions (or others) in your local TOPAS software, follow the steps below, as detailed on the [TOPAS website](https://sites.google.com/a/topasmc.org/home/user-guides/installation):

1. Clone or download the TOPAS repository to your local machine.
2. Navigate to the TOPAS directory.
3. Compile the extensions by executing the following commands in your terminal (for Debian based systems):

```bash
cd <path to TOPAS>
cmake -DTOPAS_EXTENSIONS_DIR=<path to this directory>
make
```

We recommend creating your own bash script for compilation, as you need to recompile whenever you make changes or add new extensions.

## Dirty Dose

The Dirty Dose extension calculates the proton dose for protons with LET values above a certain threshold. To use it, follow the example configuration provided below:

```ini
s:Sc/DirtyDose/Quantity                       = "DirtyDose"
d:Sc/DirtyDose/LET_Max                        =  3000 MeV/mm/(g/cm3) # (optional)
d:Sc/DirtyDose/LET_Min                        =  2 MeV/mm/(g/cm3)
s:Sc/DirtyDose/Component                      = "SomeVolume" 
b:Sc/DirtyDose/OutputToConsole                = "FALSE" # (optional)
s:Sc/DirtyDose/IfOutputFileAlreadyExists      = "Overwrite" # (optional)
b:Sc/DirtyDose/PropagateToChildren             = "True" # (optional)
s:Sc/DirtyDose/OutputFile                     = "Path to output"
```

## Trip Particle Generator

The Trip Particle Generator extension generates particles from a .csv source file created with TRiP98 (a research treatment planning software, not for clinical use). However, any .csv file with the columns listed below should work:

* ENERGY(GeV) X(CM) Y(CM) FWHM(cm) WEIGHT

Example configuration:

```ini
s:So/MySource/Type                         = "TripParticleGenerator"
s:So/MySource/Component                    = "World"
i:So/MySource/NumberOfHistoriesInRun       = 10000
s:So/MySource/ParticleSourceFile           = "PathToSourceFile"
s:So/MySource/BeamParticle                 = "proton"
```

## Nuclear Reaction Scorer

The Nuclear Reaction Scorer extension tracks nuclear interactions that occur in a volume and outputs .csv files with reaction strings in the form "A + B --> C + D," e.g.:

protonInelastic: proton + B11 --> Li7 + alpha + proton

Example configuration:

```ini
s:Sc/Ntuple/Quantity                       = "NuclearReaction"
s:Sc/Ntuple/OutputType                     = "ASCII"
s:Sc/Ntuple/Component                      = "SomeVolume"
b:Sc/Ntuple/OutputToConsole                = "TRUE" # (optional)
s:Sc/Ntuple/IfOutputFileAlreadyExists      = "Overwrite" # (optional)
sv:Sc/Ntuple/Secondaries                   =  2 "neutron", "alpha"
s:Sc/Ntuple/Target                         = "oxygen"
s:Sc/Ntuple/Projectile                     = "proton"
b:Sc/Ntuple/PropagateToChildren            = "True" # (optional)
s:Sc/Ntuple/OutputFile                     = "Path to output"
```

## Qeff Scoring

The Qeff Scorer extension calculates the Qeff parameter, an alternative RBE modeling parameter (instead of LETd). The calculation is done as follows:

```cpp
G4double Energy = aTrack->GetKineticEnergy() / MeV;
G4double Mass = aTrack->GetParticleDefinition()->GetPDGMass() / MeV;
G4double beta = sqrt(1.0 - 1.0 / ( ((Energy / Mass) + 1) * ((Energy / Mass) + 1) ));
G4int z = aTrack->GetParticleDefinition()->GetAtomicNumber();
G4double Zeff = z * (1.0 - exp(-125.0 * beta * pow(abs(z), -2.0/3.0)));
G4double Qeff = Zeff*Zeff/(beta*beta);
```

Example configuration:

```ini
s:Sc/Qeff/Quantity                         = "Qeff"
s:Sc/Qeff/Component                        = "SomeVolume"
s:Sc/Qeff/WeightBy                         = "fluence" # or dose
s:Sc/Qeff/OutputFile                      = "Qeff"
```

## Hadron LET Scorer

The Hadron LET Scorer extension scores LET for all hadrons, not only protons.

```ini
s:Sc/HadronTLET/Quantity                   = "myHadronLET"
s:Sc/HadronTLET/Component                  = "SomeVolume"
s:Sc/HadronTLET/WeightBy                   = "fluence" # or dose
s:Sc/HadronTLET/IfOutputFileAlreadyExists  = "Overwrite"
```

## Response Function Weighted Ambient Dose Equivalence Scorer

The Response Function Weighted Ambient Dose Equivalence Scorer extension calculates ambient dose equivalence while weighing the results with a specified detector response function. Example configuration:

```ini
# Score ambient dose equivalent for neutrons
sv:Sc/scorerAmbDosePerSource/OnlyIncludeParticlesNamed = 1 "neutron"
s:Sc/scorerAmbDosePerSource/Quantity                   = "MyAmbientDoseEquivalent"
s:Sc/scorerAmbDosePerSource/Component                  = "WaterPhantom"
s:Sc/scorerAmbDosePerSource/OutputFile                 = "AmbientDoseNeutronPerSourceNeutron"
b:Sc/scorerAmbDosePerSource/OutputToConsole            = "True"
s:Sc/scorerAmbDosePerSource/IfOutputFileAlreadyExists  = "Overwrite"
s:Sc/scorerAmbDosePerSource/ImportRespons              = "Wendi2_Respons.csv"
sv:Sc/scorerAmbDosePerSource/Report                    = 2 "Sum" "Standard_Deviation"
# Set the fluence-to-dose conversion factors.
s:Sc/scorerAmbDosePerSource/GetAmbientDoseEquivalentForParticleNamed = "neutron"
dv:Sc/scorerAmbDosePerSource/FluenceToDoseConversionEnergies = 58
2.5314e-08 7.71605e-08 2.35195e-07 6.33404e-07 1.70582e-06 4.05885e-06 1.02746e-05 2.44475e-05 6.18866e-05 

0.000142765
0.000309568 0.000611723 0.00100388 0.00150131 0.00217678 0.00305995 0.00430144 0.00604662 0.00849986  0.0119484
0.0157877  0.0221931  0.0293242  0.0399651  0.0511969  0.0676476  0.0866593   0.101168     0.1296   0.171243
0.233382   0.289858    0.37132   0.490632   0.590784   0.711379          1    1.85741    2.95521    4.95083
...
****
```

## Mixed Beam Source

The Mixed Beam Source extension simulates beams consisting of multiple particle types.

```ini
s:So/MySource/Type                         = "MixedSource"
s:So/MySource/Component                    = "BeamPosition"
s:So/MySource/BeamParticle                 = "proton"
sv:So/MySource/Particles                   = 2 "proton" "neutron" # what particles to simulate
i:So/MySource/NumberOfHistoriesInRun       = 100000
uv:So/MySource/MixingFraction              = 2 0.9 0.1 # mixing fraction. This gives 90 % of the No. of histories as protons and 10 % as neutrons
dv:So/MySource/MonoEnergies                 = 2 250 1 MeV # Energy of the particles. In this case, protons have 250 MeV and neutrons have 1 MeV
d:So/MySource/BeamPositionZ                = Ge/YourComponent/HLZ + 1.5 cm
```

Feel free to use these extensions, contribute your own, and have fun simulating with TOPAS!
