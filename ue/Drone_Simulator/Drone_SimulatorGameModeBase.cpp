// Fill out your copyright notice in the Description page of Project Settings.

#include "Drone_Simulator.h"
#include "Drone_SimulatorGameModeBase.h"
#include "Drone_Body.h"


ADrone_SimulatorGameModeBase::ADrone_SimulatorGameModeBase() {
	DefaultPawnClass = ADrone_Body::StaticClass();
}

