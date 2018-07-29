// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Networking.h"
#include "Components/ActorComponent.h"
#include "CustomData.h"
#include "Messages.h"
#include "Udp_Com.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DRONE_SIMULATOR_API UUdp_Com : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Remote Address")
	FString SourceIP_Address;

	UPROPERTY(EditAnywhere, Category = "Remote Address")
	int32 PortIn;

	UPROPERTY(EditAnywhere, Category = "Remote Address")
	FString DestIP_Address;

	UPROPERTY(EditAnywhere, Category = "Remote Address")
	int32 PortOut;

public:	
	// Sets default values for this component's properties
	UUdp_Com();

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	// Called whenever this actor is being removed from a level
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Parsing the Received Data
	void Recv(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt);

	// Start the communication
	bool StartUDPComm(const FString& YourChosenSocketName);

	// Retrieve Data
	void GetData(FCustomData* RetData);

	// Send Data
	void SendData(FCustomOutputData* OutData);

	// Get first packet received value
	bool GetNewPacketRecvd();

private:

	// ----------------
	// UDP Socket stuff
	FSocket* ListenSocket;   // Input Socket
	FSocket* SendSocket;	 // Output Socket

	ISocketSubsystem* SocketSubsystem;

	FUdpSocketReceiver* UDPReceiver = nullptr;

	// Address of the remote machine
	TSharedPtr<FInternetAddr>	RemoteAddr;

	// Received Data
	FCustomData Data;

	// Output Data
	FCustomOutputData DataOut;

	bool NewPacketRecvd = false;	
};
