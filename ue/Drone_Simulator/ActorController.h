// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "UDP_Com.h"
#include "ActorController.generated.h"

UCLASS()
class DRONE_SIMULATOR_API AActorController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AActorController();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Initialize the communication stuff
	virtual void PreInitializeComponents() override;

	// Return a copy of new data received
	void ReturnNewData(FCustomData* NewData);

private:

	// Communication Component
	UPROPERTY(EditAnywhere, Category = "Remote Address")
	class UUdp_Com* OurCommunicationComponent;

	FCustomData ReceivedData;
	
};
