// Fill out your copyright notice in the Description page of Project Settings.

#include "Drone_Simulator.h"
#include "Udp_Com.h"


// Sets default values for this component's properties
UUdp_Com::UUdp_Com() : SourceIP_Address(FString("0.0.0.0")), PortIn(8000), DestIP_Address(FString("127.0.0.1")), PortOut(8001)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	bAutoActivate = false;

	ListenSocket = NULL;
	SendSocket = NULL;

	// ...
}


// Called when the game starts
void UUdp_Com::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void UUdp_Com::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...
}

void UUdp_Com::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	//ScreenMsg("Closing Sockets...");
	if (UDPReceiver != nullptr)
	{
		UDPReceiver->Stop();
		delete UDPReceiver;
		UDPReceiver = nullptr;
	}

	//Clear all sockets!
	if (ListenSocket)
	{
		ListenSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenSocket);
	}

	if (SendSocket)
	{
		SendSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SendSocket);
	}
}

void UUdp_Com::Recv(const FArrayReaderPtr & ArrayReaderPtr, const FIPv4Endpoint & EndPt)
{
	*ArrayReaderPtr << Data;
	NewPacketRecvd = true;
}

bool UUdp_Com::StartUDPComm(const FString & YourChosenSocketName)
{
	//ScreenMsg("SOCKETS INIT");
	//ScreenMsg("Source IP = ", SourceIP_Address);
	//ScreenMsg("Source Port = ", PortIn);
	//ScreenMsg("Destination IP = ", DestIP_Address);
	//ScreenMsg("Destination Port = ", PortOut);

	FIPv4Address SourceAddr, DestAddr;

	// Parse the IP addresses
	FIPv4Address::Parse(SourceIP_Address, SourceAddr);
	FIPv4Address::Parse(DestIP_Address, DestAddr);

	// Create the endpoint object
	FIPv4Endpoint InputEndpoint(SourceAddr, PortIn);
	FIPv4Endpoint OutputEndpoint(DestAddr, PortOut);

	bool valid = false;
	RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	RemoteAddr->SetIp(*DestIP_Address, valid);
	RemoteAddr->SetPort(PortOut);

	if (!valid)
	{
		ScreenMsg("Problem with the remote Address!");
		return false;
	}

	//BUFFER SIZE
	int32 BufferSize = 2 * 1024 * 1024;

	//Create Listener Socket
	ListenSocket = FUdpSocketBuilder(*YourChosenSocketName)
		.AsNonBlocking()
		.AsReusable()
		.BoundToEndpoint(InputEndpoint)
		.WithReceiveBufferSize(BufferSize);

	//Create Sender Socket
	SendSocket = FUdpSocketBuilder(*YourChosenSocketName)
		.AsReusable()
		.WithSendBufferSize(BufferSize)
		.WithBroadcast();

	// Spaw a receiving thread
	FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(4);
	UDPReceiver = new FUdpSocketReceiver(ListenSocket, ThreadWaitTime, TEXT("UDP RECEIVER"));
	UDPReceiver->OnDataReceived().BindUObject(this, &UUdp_Com::Recv);
	//ScreenMsg("Starting Listening Thread...");
	UDPReceiver->Start();

	return true;
}

void UUdp_Com::GetData(FCustomData * RetData)
{
	NewPacketRecvd = false;
	*RetData = Data;
}

void UUdp_Com::SendData(FCustomOutputData * OutData)
{
	FArrayWriter OutputRaw;
	int32 byteSent = 0;
	OutputRaw << *OutData;

	SendSocket->SendTo(OutputRaw.GetData(), OutputRaw.Num(), byteSent, *RemoteAddr);
}

bool UUdp_Com::GetNewPacketRecvd()
{
	return NewPacketRecvd;
}

