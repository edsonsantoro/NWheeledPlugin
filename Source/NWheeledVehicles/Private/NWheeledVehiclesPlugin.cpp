// Copyright 2016 Santoro Studio, All Rights Reserved.

#include "NWheeledVehiclesPluginPrivatePCH.h"




class FNWheeledVehicle : public IModuleInterface
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FNWheeledVehicle, NWheeledVehicle)



void FNWheeledVehicle::StartupModule()
{
	
}


void FNWheeledVehicle::ShutdownModule()
{
	
}



