// Fill out your copyright notice in the Description page of Project Settings.

#include "Drone_Simulator.h"
#include "Messages.h"
#include "CustomData.h"
#include "EngineUtils.h"
#include "ActorController.h"


// Sets default values
AActorController::AActorController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Instantiate the communication component
	OurCommunicationComponent = CreateDefaultSubobject<UUdp_Com>(TEXT("CommunicationComponent"));
}

// Called when the game starts or when spawned
void AActorController::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AActorController::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	
	// Get Data
	if(OurCommunicationComponent->GetNewPacketRecvd()) {
		OurCommunicationComponent->GetData(&ReceivedData);
	}
}

void AActorController::ReturnNewData(FCustomData* NewData) {
	*NewData = ReceivedData;
}

void AActorController::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	OurCommunicationComponent->StartUDPComm("PawnCommunicationComponent");
}

