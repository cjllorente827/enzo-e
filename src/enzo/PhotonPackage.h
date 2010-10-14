/***********************************************************************
/
/  PHOTON PACKAGE CLASS
/
/  written by: Tom Abel & Greg Bryan
/  date:       August, 2003
/  modified1:  February, 2010 by JHW
/                Converted into a poor man's class with everything 
/                public.  I need a constructor/destructor to use the
/                MemoryPool to avoid memory fragmentation.
/
/  PURPOSE: Constructs a Linked List of Photon Packages including data
/
************************************************************************/
#include "RadiationSource.h"
#include "RadiativeTransferHealpixRoutines.h"

class PhotonPackageEntry
{
public:
  PhotonPackageEntry *NextPackage; // Next Link
  PhotonPackageEntry *PreviousPackage; // Previous Link
  SuperSourceEntry *CurrentSource;  // Currently used (super)source  
  float Photons;                // number of photons in package
  int   Type;                   // 0 = HI, 1=HeI, 2=HeII, 3=H2I_LW, 4=Xray
  float Energy;                 // Mean energy of photons in this package [eV]
  double CrossSection;          // Cross-section of absorber [cm^2]
  FLOAT EmissionTimeInterval;   // Time over which package was emitted
  FLOAT EmissionTime;           // Time when package was emitted
  FLOAT CurrentTime;            // Current Time
  FLOAT Radius;                 // Distance travelled
  float ColumnDensity;           // Column Density (for shielding functions)
  long  ipix;                   // pixel in HEALPIX terminology
  int   level;                  // level in HEALPIX terminology
  FLOAT SourcePosition[3];      // Position where package was emitted
  float SourcePositionDiff;     // Radius at which it was radiated (0 = pt src)

  /* CONSTRUCTOR AND DESTRUCTOR */

  PhotonPackageEntry(void);

  virtual ~PhotonPackageEntry(void) {};

  /* Overloaded new/delete to use the memory pool, if requested */

#ifdef MEMORY_POOL
  void* operator new(size_t nobjects);
  void operator delete(void* object);
#endif

  void PrintInfo(void) {
    FLOAT r[3];
    FLOAT u[3];
    pix2vec_nest((long)(1 << level), ipix, u);
    for (int dim = 0; dim < 3; dim++) 
      r[dim] = SourcePosition[dim] + u[dim] * Radius;
    printf("Photons = %g, Type = %d, Radius = %"PSYM"\n", Photons, Type, Radius);
    printf("ipix = %ld, level = %d\n", ipix, level);
    printf("normal = %lf %lf %lf\n", u[0], u[1], u[2]);
    printf("SourcePosition = %"PSYM" %"PSYM" %"PSYM"\n",
	   SourcePosition[0], SourcePosition[1], SourcePosition[2]);
    printf("RayPosition = %"PSYM" %"PSYM" %"PSYM"\n", r[0], r[1], r[2]);
    return;
  };

};