// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "UDP_Com.h"
#include "ActorController.h"
#include "Ball.generated.h"

UCLASS()
class DRONE_SIMULATOR_API ABall : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABall();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	// Set the Pose from the Data received and return it in the correspondend vectors
	// Position is in [cm]
	// Rotation is in [degrees]
	void SetPose(FCustomData* ReceivedData);

private:

	UPROPERTY(EditAnywhere, Category = "Geometry")
	USceneComponent* BallVisibleComponent;

	//ActorController pointer
	AActorController* ActorController;
};
