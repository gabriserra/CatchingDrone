// Fill out your copyright notice in the Description page of Project Settings.

#include "Drone_Simulator.h"
#include "Messages.h"
#include "CustomData.h"
#include "ActorController.h"
#include "EngineUtils.h"
#include "Drone_Body.h"


// Sets default values
ADrone_Body::ADrone_Body()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetActorLocation(FVector(0.0f, 0.0f, 10.0f));

	// Instantiate the StaticMesh Component and configure it as root component
	OurVisibleComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OurVisibleComponent"));
	OurVisibleComponent->SetupAttachment(RootComponent);
	RootComponent = OurVisibleComponent;

	// Instantiate Spring Arm and the Camera component
	CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
	OutBoardCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("OutBoardCamera"));

	// Attach the Spring Arm to the root component
	CameraSpringArm->SetupAttachment(RootComponent);
	CameraSpringArm->SetRelativeLocation(FVector(-100.0f, 0.0f, 50.0f));
	CameraSpringArm->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	CameraSpringArm->TargetArmLength = 400.0f;
	
	// Decouple the some rotationale movement of the Pawn from the movement of the camera
	CameraSpringArm->bInheritRoll = false;
	CameraSpringArm->bInheritPitch = false;

	// Attach the Camera to the Spring Arm
	OutBoardCamera->SetupAttachment(CameraSpringArm, USpringArmComponent::SocketName);

	// Get actor controller pointer
	for (TObjectIterator<AActorController> Itr; Itr; ++Itr)
		if (Itr->IsA(AActorController::StaticClass()))
			ActorController = *Itr;
}

// Called when the game starts or when spawned
void ADrone_Body::BeginPlay()
{
	Super::BeginPlay();
	SetActorLocation(FVector(0.0f, 0.0f, 500.0f));
}

// Called every frame
void ADrone_Body::Tick( float DeltaTime )
{
	FCustomData NewData;

	Super::Tick(DeltaTime);
	ActorController->ReturnNewData(&NewData);
	SetPose(&NewData);
}

// Called to bind functionality to input
void ADrone_Body::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);
	
	//Hook up events for camera zoom
	InputComponent->BindAction("CameraZoomIn", IE_Pressed, this, &ADrone_Body::CameraZoomIn);
	InputComponent->BindAction("CameraZoomOut", IE_Pressed, this, &ADrone_Body::CameraZoomOut);

	//Hook up every-frame handling for our camera rotation
	InputComponent->BindAxis("CameraPitch", this, &ADrone_Body::CameraPitch);
	InputComponent->BindAxis("CameraYaw", this, &ADrone_Body::CameraYaw);
	
}

void ADrone_Body::SetPose(FCustomData* ReceivedData)
{
	FVector Position;
	FRotator Rotation;
	FVector Attuale;
	bool done = false;



	// Take the measures in Meters and convert them in centimeters
	Position.X = 100 * ReceivedData->dronePosition.X;
	Position.Y = 100 * ReceivedData->dronePosition.Y;
	Position.Z = 100 * ReceivedData->dronePosition.Z;

	// Take the measures in Radiants and convert them into degree 
	Rotation.Roll = ReceivedData->droneRotation.X * 180 / PI;
	Rotation.Pitch = ReceivedData->droneRotation.Y * 180 / PI;
	Rotation.Yaw = ReceivedData->droneRotation.Z * 180 / PI;

	// Set Position	
	SetActorLocation(Position);

	// Set relative rotation in space
	SetActorRelativeRotation(Rotation.Quaternion());
}

void ADrone_Body::CameraPitch(float AxisValue)
{
	CameraInput.Y += AxisValue;
	CameraSpringArm->SetRelativeRotation(FRotator(CameraInput.Y, CameraInput.X, 0.0f));

}

void ADrone_Body::CameraYaw(float AxisValue)
{
	CameraInput.X += AxisValue;
	CameraSpringArm->SetRelativeRotation(FRotator(CameraInput.Y, CameraInput.X, 0.0f));
}

void ADrone_Body::CameraZoomIn()
{
	CameraSpringArm->TargetArmLength += 50;
}

void ADrone_Body::CameraZoomOut()
{
	CameraSpringArm->TargetArmLength -= 50;
}

