#pragma once

#include "CustomData.generated.h"
#define SPACEDIM 3
#define NROTOR 4

USTRUCT()
struct FCustomData
{
	GENERATED_USTRUCT_BODY()

	FVector dronePosition;
	FVector droneRotation;
	FVector ballPosition;

	FCustomData() {}
};



USTRUCT()
struct FCustomOutputData
{
	GENERATED_USTRUCT_BODY()

	// Object ID
	int Id = 0;

	// Collision Direction
	float Nx = 0;
	float Ny = 0;
	float Nz = 0;

	// Penetration [m]
	float Penetration = 0.0f;

	FCustomOutputData() {}
};


FORCEINLINE FArchive& operator<<(FArchive &Ar, FCustomData& TheStruct)
{
	Ar << TheStruct.dronePosition;
	Ar << TheStruct.droneRotation;
	Ar << TheStruct.ballPosition;

	return Ar;
}


FORCEINLINE FArchive& operator<<(FArchive &Ar, FCustomOutputData& TheStruct)
{
	Ar << TheStruct.Id;
	Ar << TheStruct.Nx;
	Ar << TheStruct.Ny;
	Ar << TheStruct.Nz;
	Ar << TheStruct.Penetration;

	return Ar;
}
