/**
* Copyright 2014 Social Robotics Lab, University of Freiburg
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*    # Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*    # Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*    # Neither the name of the University of Freiburg nor the names of its
*       contributors may be used to endorse or promote products derived from
*       this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
* \author Billy Okal <okal@cs.uni-freiburg.de>
* \author Sven Wehner <mail@svenwehner.de>
*/

#include <pedsim_simulator/scenarioreader.h>
#include <pedsim_simulator/element/agent.h>
#include <pedsim_simulator/element/agentcluster.h>
#include <pedsim_simulator/element/obstacle.h>
#include <pedsim_simulator/element/areawaypoint.h>
#include <pedsim_simulator/element/waitingqueue.h>
#include <pedsim_simulator/element/attractionarea.h>
#include <QFile>
#include <iostream>

#include <ros/ros.h>


ScenarioReader::ScenarioReader()
{
    // initialize values
    currentAgents = nullptr;
}


bool ScenarioReader::readFromFile ( const QString& filename )
{
// 	ROS_DEBUG("Loading scenario file '%1'.", filename);

    // open file
    QFile file ( filename );
    if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
    {
		ROS_DEBUG("Couldn't open scenario file!");
        return false;
    }

    // read input
    xmlReader.setDevice ( &file );

    while ( !xmlReader.atEnd() )
    {
        xmlReader.readNext();
        processData();
    }

    // check for errors
    if ( xmlReader.hasError() )
    {
// 		ROS_DEBUG("Error while reading scenario file: %s (line: %s)", xmlReader.errorString().toStdString().c_str(),
// xmlReader.lineNumber().toStdString().c_str());
        return false;
    }

    // report success
    return true;
}


void ScenarioReader::processData()
{
    if ( xmlReader.isStartElement() )
    {
        const QString elementName = xmlReader.name().toString();
        const QXmlStreamAttributes elementAttributes = xmlReader.attributes();

        if ( ( elementName == "scenario" )
                || ( elementName == "welcome" ) )
        {
            // nothing to do
        }
        else if ( elementName == "obstacle" )
        {
            double x1 = elementAttributes.value ( "x1" ).toString().toDouble();
            double y1 = elementAttributes.value ( "y1" ).toString().toDouble();
            double x2 = elementAttributes.value ( "x2" ).toString().toDouble();
            double y2 = elementAttributes.value ( "y2" ).toString().toDouble();
            Obstacle* obs = new Obstacle ( x1, y1, x2, y2 );
            SCENE.addObstacle ( obs );
            SCENE.drawObstacles ( x1, y1, x2, y2 );
        }
        else if ( elementName == "waypoint" )
        {
            QString id = elementAttributes.value ( "id" ).toString();
            double x = elementAttributes.value ( "x" ).toString().toDouble();
            double y = elementAttributes.value ( "y" ).toString().toDouble();
            double r = elementAttributes.value ( "r" ).toString().toDouble();
            AreaWaypoint* w = new AreaWaypoint ( id, x, y, r );
            SCENE.addWaypoint ( w );
        }
        else if ( elementName == "queue" )
        {
            QString id = elementAttributes.value ( "id" ).toString();
            double x = elementAttributes.value ( "x" ).toString().toDouble();
            double y = elementAttributes.value ( "y" ).toString().toDouble();
            double directionValue = elementAttributes.value ( "direction" ).toString().toDouble();

            Ped::Tvector position ( x, y );
            Ped::Tangle direction = Ped::Tangle::fromDegree ( directionValue );

            WaitingQueue* queue = new WaitingQueue ( id, position, direction );
            SCENE.addWaitingQueue ( queue );
        }
        else if ( elementName == "attraction" )
        {
            QString id = elementAttributes.value ( "id" ).toString();
            double x = elementAttributes.value ( "x" ).toString().toDouble();
            double y = elementAttributes.value ( "y" ).toString().toDouble();
            double width = elementAttributes.value ( "width" ).toString().toDouble();
            double height = elementAttributes.value ( "height" ).toString().toDouble();
            double strength = elementAttributes.value ( "strength" ).toString().toDouble();

            AttractionArea* attraction = new AttractionArea ( id );
            attraction->setPosition ( x, y );
            attraction->setSize ( width, height );
            attraction->setStrength ( strength );
            SCENE.addAttraction ( attraction );
        }
        else if ( elementName == "agent" )
        {
            double x = elementAttributes.value ( "x" ).toString().toDouble();
            double y = elementAttributes.value ( "y" ).toString().toDouble();
            int n = elementAttributes.value ( "n" ).toString().toInt();
            double dx = elementAttributes.value ( "dx" ).toString().toDouble();
            double dy = elementAttributes.value ( "dy" ).toString().toDouble();
            int type = elementAttributes.value ( "type" ).toString().toInt();
            AgentCluster* agentCluster = new AgentCluster ( x, y, n );
            agentCluster->setDistribution ( dx, dy );
            agentCluster->setType ( static_cast<Ped::Tagent::AgentType> ( type ) );
            SCENE.addAgentCluster ( agentCluster );
            currentAgents = agentCluster;

            std::cout << "Added agent cluster size " << n << std::endl;
        }
        // → agent's inner elements
        else if ( elementName == "addwaypoint" )
        {
            if ( currentAgents == nullptr )
            {
				ROS_DEBUG("Invalid <addwaypoint> element outside of agent element!");
                return;
            }

            // add waypoints to current <agent> element
            QString id = elementAttributes.value ( "id" ).toString();
            currentAgents->addWaypoint ( SCENE.getWaypointByName ( id ) );
        }
        else if ( elementName == "addqueue" )
        {
            if ( currentAgents == nullptr )
            {
				ROS_DEBUG("Invalid <addqueue> element outside of agent element!");
                return;
            }

            // add waiting queue to current <agent> element
            QString id = elementAttributes.value ( "id" ).toString();
            currentAgents->addWaitingQueue ( SCENE.getWaitingQueueByName ( id ) );
        }
        else
        {
            // inform the user about invalid elements
// 			ROS_DEBUG("Unknown element: <%s>", elementName.toStdString());
        }
    }
    else if ( xmlReader.isEndElement() )
    {
        const QString elementName = xmlReader.name().toString();

        if ( elementName == "agent" )
        {
            currentAgents = nullptr;
        }
    }
}