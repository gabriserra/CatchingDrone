// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "UDP_Com.h"
#include "ActorController.h"
#include "Drone_Body.generated.h"

UCLASS()
class DRONE_SIMULATOR_API ADrone_Body : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ADrone_Body();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	// Get the Pose from the Data received and return it in the correspondend vectors
	// Position is in [cm]
	// Rotation is in [degrees]
	void SetPose(FCustomData* ReceivedData);

private:
	// Camera Component
	UPROPERTY(EditAnywhere, Category = "Camera")
	UCameraComponent* OutBoardCamera;

	// Visual Component
	UPROPERTY(EditAnywhere, Category = "SpringArm")
	USpringArmComponent* CameraSpringArm;

	UPROPERTY(EditAnywhere, Category = "Geometry")
	USceneComponent* OurVisibleComponent;

	//ActorController pointer
	AActorController* ActorController;

	//Input variables
	FVector2D CameraInput;

	//Input functions
	void CameraPitch(float AxisValue);
	void CameraYaw(float AxisValue);
	void CameraZoomIn();
	void CameraZoomOut();
};
