

MAIN_DIR=/home/villads/tools/topas_src

export Geant4_DIR=$MAIN_DIR/geant4.10.07.p03-install
export TOPAS_G4_DATA_DIR=$MAIN_DIR/G4Data


cd $MAIN_DIR/topas-build
# cp -r $HOME/tools/Topas-Extension/topas_extensions .
extensions=/home/villads/tools/Topas-Extension/topas_extensions
GDCM_DIR=../gdcm-install/ cmake ../topas_3_9 -DCMAKE_INSTALL_PREFIX=$MAIN_DIR/topas-install -DTOPAS_EXTENSIONS_DIR=$extensions
make CFLAGS="-Wno-error=format-truncation" -j
make install
