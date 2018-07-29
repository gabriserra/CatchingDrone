// Fill out your copyright notice in the Description page of Project Settings.

#include "Drone_Simulator.h"
#include "Messages.h"
#include "CustomData.h"
#include "ActorController.h"
#include "Ball.h"


// Sets default values
ABall::ABall()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetActorLocation(FVector(0.0f, 0.0f, 29.0f));

	// Instantiate the StaticMesh Component and configure it as root component
	BallVisibleComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallVisibleComponent"));

	// Get actor controller pointer
	for (TObjectIterator<AActorController> Itr; Itr; ++Itr)
		if (Itr->IsA(AActorController::StaticClass()))
			ActorController = *Itr;
}

// Called when the game starts or when spawned
void ABall::BeginPlay()
{
	Super::BeginPlay();
	SetActorLocation(FVector(0.0f, 0.0f, 0.0f));
}

// Called every frame
void ABall::Tick( float DeltaTime )
{
	FCustomData NewData;

	Super::Tick(DeltaTime);
	ActorController->ReturnNewData(&NewData);
	SetPose(&NewData);
}

void ABall::SetPose(FCustomData* ReceivedData)
{
	FVector Position;

	// Take the measures in Meters and convert them in centimeters
	Position.X = 100 * ReceivedData->ballPosition.X;
	Position.Y = 100 * ReceivedData->ballPosition.Y;
	Position.Z = 100 * ReceivedData->ballPosition.Z;

	// Set Position	
	SetActorLocation(Position);
}

