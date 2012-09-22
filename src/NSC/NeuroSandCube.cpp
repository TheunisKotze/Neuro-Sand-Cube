#include "NeuroSandCube.h"

#include "ConfigReader.h"
#include "MessageDispatchController.h"
#include "TCPDispatcher.h"

#include <sstream>
#include <boost/foreach.hpp>


NeuroSandCube::NeuroSandCube(void)
{
}


NeuroSandCube::~NeuroSandCube(void)
{
}


void NeuroSandCube::Initialize(fpsent* player)
{
	this->player = player;

	std::string configFile = "data/NSC/nsc_config.json";

	distributor.AddSharedState(boost::variant<int&,float&, bool&>(player->newpos.x),"player_x");
	distributor.AddSharedState(boost::variant<int&,float&, bool&>(player->newpos.y),"player_y");
	distributor.AddSharedState(boost::variant<int&,float&, bool&>(player->attacking),"player_left_click");
	distributor.AddSharedState(boost::variant<int&, float&, bool&>(player->respawned),"level_restart");

	SharedStateConfigReader configReader;
	

	configReader.ReadConfig(configFile);
	int i=0;
	boost::shared_ptr<SharedStateAttributes> attributes = configReader.Get(i);

	MessageDispatchController& dispatchController = MessageDispatchController::GetInstance();
	while (attributes.get() != NULL)
	{
		distributor.AddDistribution(*attributes);

		// for each consumer

		// to clean up, create TCPDispatcher via factory or other proxy
		for (unsigned int j=0; j < attributes->consumers.size(); ++j)
		{
			auto consumer = std::string(attributes->consumers[j]);
			std::istringstream oss(consumer);
			std::string ip, port;
			getline(oss , ip, ':'); 
			getline(oss , port, ':'); 

			if (!dispatchController.HasDispatcher(consumer))
			{
				auto dispatcher = boost::shared_ptr<TCPDispatcher>(new TCPDispatcher(ip,port));
				dispatchController.AddDispatcher(consumer,dispatcher);
			}
		}
		
		attributes = configReader.Get(++i);

	}


}

void NeuroSandCube::Update()
{
	distributor.Distribute();
	TCPClient::Update();
}