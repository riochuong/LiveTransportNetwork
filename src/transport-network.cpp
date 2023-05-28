#include <stdexcept>
#include "network-monitor/transport-network.h"
#include "logging.h"

namespace NetworkMonitor
{
    TransportNetwork::TransportNetwork(){};
    TransportNetwork::TransportNetwork(const TransportNetwork &copy)
    {

        // add all nodes
        for (auto it = copy.nodeDict_.begin(); it != copy.nodeDict_.end(); it++)
        {
            this->nodeDict_[it->first] = Node();
            this->nodeDict_[it->first].station_id = it->first;
        }

        for (auto it = copy.lineDict_.begin(); it != copy.lineDict_.end(); it++)
        {
            this->lineDict_[it->first] = LineInternal();
            this->lineDict_[it->first].name = it->second.name;
            this->lineDict_[it->first].id = it->second.id;
            for (auto routeIt = it->second.routeMaps.begin(); routeIt != it->second.routeMaps.end(); routeIt++)
            {
                auto &currentLine = this->lineDict_[it->first];
                currentLine.routeMaps[routeIt->first].id = routeIt->second.id;
                currentLine.routeMaps[routeIt->first].line = std::make_shared<LineInternal>(currentLine);
                for (auto &stop : routeIt->second.stops)
                {
                    currentLine.routeMaps[routeIt->first].stops.push_back(std::make_shared<Node>(this->nodeDict_[stop->station_id]));
                }
            }
        }

        // add all edges
        for (auto it = copy.nodeDict_.begin(); it != copy.nodeDict_.end(); it++)
        {
            for (auto &edge : it->second.edges)
            {
                this->nodeDict_[it->first].edges.emplace_back();
                auto &lastEdge = this->nodeDict_[it->first].edges[this->nodeDict_.size() - 1];
                lastEdge->nextStop = std::make_shared<Node>(this->nodeDict_[edge->nextStop->station_id]);
                lastEdge->lineId = edge->lineId;
                lastEdge->routeId = edge->routeId;
            }
        }
    }

    bool TransportNetwork::AddLine(const Line &line)
    {
        if (this->lineDict_.find(line.id) != this->lineDict_.end())
        {
            logger::error("Line id {} is already part of the network !", line.id);
            return false;
        }
        this->lineDict_.emplace(line.id, LineInternal());
        this->lineDict_[line.id].id = line.id;
        this->lineDict_[line.id].name = line.name;
        for (auto &route : line.routes)
        {
            if (this->nodeDict_.find(route.startStationId) != this->nodeDict_.end())
            {
                logger::error("Station {} is not already part of the Network !!!");
                return false;
            }
            if (this->nodeDict_.find(route.endStationId) != this->nodeDict_.end())
            {

                logger::error("Station {} is not already part of the Network !!!");
                return false;
            }
            // add new edge to the network also
            auto edge_ptr = std::make_shared<Edge>();
            edge_ptr->lineId = line.id;
            edge_ptr->routeId = route.id;
            edge_ptr->travelTime = 0;
            edge_ptr->nextStop = std::make_shared<Node>(this->nodeDict_[route.endStationId]);
            this->nodeDict_[route.startStationId].edges.push_back(edge_ptr);
            this->lineDict_[line.id].routeMaps.emplace(route.id, RouteInternal());
            auto &routeInt = this->lineDict_[line.id].routeMaps[route.id];
            routeInt.id = route.id;
            routeInt.line = std::make_shared<LineInternal>(this->lineDict_[line.id]);
            for (auto &stopId : route.stops)
            {
                routeInt.stops.push_back(std::make_shared<Node>(this->nodeDict_[stopId]));
            }
        }
        return true;
    }

    bool TransportNetwork::AddStation(const Station &station)
    {
        if (this->nodeDict_.find(station.id) != this->nodeDict_.end())
        {
            logger::error("Station {} is already part of the network !", station.id);
            return false;
        }
        assert(this->nodeDict_.find(station.id) == this->nodeDict_.end());
        this->nodeDict_.emplace(station.id, Node());
        this->nodeDict_[station.id].station_id = station.id;
        this->nodeDict_[station.id].name = station.name;
        return true;
    }

    bool TransportNetwork::RecordPassengerEvent(const PassengerEvent &event)
    {
        if (this->nodeDict_.find(event.stationId) == this->nodeDict_.end())
        {
            throw std::runtime_error("Station: " + event.stationId + " is not part of the network");
        }

        this->nodeDict_[event.stationId].passengerCount++;
        return true;
    }

    int64_t TransportNetwork::GetPassengerCount(const Id &station)
    {
        if (this->nodeDict_.find(station) == this->nodeDict_.end())
        {
            throw std::runtime_error("Station: " + station + " is not part of the network");
        }
        return this->nodeDict_[station].passengerCount;
    }

    std::vector<Id> TransportNetwork::GetRouteServingAtStation(const Id &station)
    {
        if (this->nodeDict_.find(station) == this->nodeDict_.end())
        {
            throw std::runtime_error("Station: " + station + " is not part of the network");
        }
        std::vector<Id> routeList;
        for (auto it = this->nodeDict_.begin(); it != this->nodeDict_.end(); it++)
        {
            if (it->first == station)
            {
                routeList.push_back(it->first);
            }
        }
        return routeList;
    }

    bool TransportNetwork::SetTravelTime(const Id &stationA, const Id &stationB, const unsigned int travelTime)
    {
        if (this->nodeDict_.find(stationA) != this->nodeDict_.end())
        {
            logger::error("Station {} is not part of the network", stationA);
            return false;
        }
        if (this->nodeDict_.find(stationB) != this->nodeDict_.end())
        {

            logger::error("Station {} is not part of the network", stationB);
            return false;
        }

        for (auto &edge : nodeDict_[stationA].edges)
        {
            if (edge->nextStop->station_id == stationB)
            {
                edge->travelTime = travelTime;
                return true;
            }
        }
        logger::error("Station {} is not adjacent to station {}", stationB, stationA);
        return false;
    }

    unsigned int TransportNetwork::GetTravelTimeBetweenAdjStations(const Id &stationA, const Id &stationB)
    {
        if (this->nodeDict_.find(stationA) != this->nodeDict_.end())
        {
            logger::error("Station {} is not part of the network", stationA);
            return 0;
        }
        if (this->nodeDict_.find(stationB) != this->nodeDict_.end())
        {

            logger::error("Station {} is not part of the network", stationB);
            return 0;
        }

        for (auto &edge : nodeDict_[stationA].edges)
        {
            if (edge->nextStop->station_id == stationB)
            {
                return edge->travelTime;
            }
        }
        logger::error("Station {} is not adjacent to station {}", stationB, stationA);
        return 0;
    }

    unsigned int TransportNetwork::GetTravelTime(const Id &line, const Id &route, const Id &stationA, const Id &stationB)
    {
        if (this->lineDict_.find(line) == this->lineDict_.end())
        {
            logger::error("Line {} is not part of the Network ", line);
            return 0;
        }
        if (this->lineDict_[line].routeMaps.find(route) == this->lineDict_[line].routeMaps.end())
        {
            logger::error("Route {} cannot be found from line {}", route, line);
            return 0;
        }
        std::shared_ptr<Node> prevStop = nullptr;
        for (auto &stop : lineDict_[line].routeMaps[route].stops)
        {
            bool foundAStop = false;
            bool foundBStop = false;
            uint32_t travelTime = 0;
            if (stop->station_id == stationA)
            {
                foundAStop = true;
                prevStop = stop;
                continue;
            }
            if (stop->station_id == stationB)
            {
                foundBStop = true;
            }

            if (foundAStop)
            {
                int edgeTime = GetTravelTimeBetweenAdjStations(prevStop->station_id, stop->station_id);
                assert(edgeTime > 0);
                travelTime += edgeTime;
            }
            if (foundBStop)
            {
                return travelTime;
            }
            prevStop = stop;
        }
        logger::error("Failed to get travel time between stations {} {}", stationA, stationB);
        return 0;
    }

    std::vector<std::shared_ptr<TransportNetwork::Edge>>::const_iterator
    TransportNetwork::Node::FindEdegesForRoute(const Id &route) const
    {
        std::vector<std::shared_ptr<TransportNetwork::Edge>> returnEdges;
        for (auto &edge : this->edges)
        {
            if (edge->routeId == route)
            {
                returnEdges.push_back(edge);
            }
        }
        return returnEdges.begin();
    }

    bool operator==(const Route& r1, const Route& r2) {return r1.id == r2.id;}

    bool operator==(const Line& r1, const Line& r2) {return r1.id == r2.id;}


}
