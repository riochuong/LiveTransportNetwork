
#include <gtest/gtest.h>
#include <network-monitor/transport-network.h>
#include <logging.h>

namespace NetworkMonitor
{
    TEST(TestTransportNetwork, Basic){
        TransportNetwork tn{};
        Station station {
            "station_000",
            "Station name"
        };
        ASSERT_TRUE(tn.AddStation(station));
    }


    TEST(TestTransportNetwork, DuplicateStationId){
        TransportNetwork tn{};
        Station station {
            "station_000",
            "Station name"
        };
        Station station1 {
            "station_000",
            "Station name"
        };

        ASSERT_TRUE(tn.AddStation(station));
        ASSERT_FALSE(tn.AddStation(station1));
    }

    TEST(TestTransportNetwork, BasicRouteLine){
        TransportNetwork nw{};
        bool ok{false};

        // Add a line with 1 route.
        // route0: 0 ---> 1

        // First, add the stations.
        Station station0{
            "station_000",
            "Station Name 0",
        };
        Station station1{
            "station_001",
            "Station Name 1",
        };
        ok = true;
        ok &= nw.AddStation(station0);
        ok &= nw.AddStation(station1);
        ASSERT_TRUE(ok);

        // Then, add the line, with the two routes.
        Route route0{
            "route_000",
            "inbound",
            "line_000",
            "station_000",
            "station_001",
            {"station_000", "station_001"},
        };
        Line line{
            "line_000",
            "Line Name",
            {route0},
        };
        ASSERT_TRUE(nw.AddLine(line));
        ASSERT_FALSE(nw.AddLine(line));
    }

    TEST(TestTransportNetwork, TestRoutesWithSharedStations){
        TransportNetwork tnw {};
        // Define a line with 2 routes going through some shared stations.
        // route0: 0 ---> 1 ---> 2
        // route1: 3 ---> 1 ---> 2
        Station station1 {
            "station1",
            "station1"
        };
        
        Station station2 {
            "station2",
            "station2"
        };
        
        Station station0 {
            "station0",
            "station0"
        };
        
        Station station3 {
            "station3",
            "station3"
        };


        Route route0{
            "route_000",
            "inbound",
            "line_000",
            "station0",
            "station2",
            {"station0", "station1", "station2"}
        };
        Route route1{
            "route_001",
            "inbound",
            "line_000",
            "station3",
            "station2",
            {"station3", "station1", "station2"}
        };

        Line line0{
            "line0",
            "Line 0",
            {route0, route1}
        };        
        ASSERT_TRUE(tnw.AddStation(station0));
        ASSERT_TRUE(tnw.AddStation(station1));
        ASSERT_TRUE(tnw.AddStation(station2));
        ASSERT_TRUE(tnw.AddStation(station3));
        ASSERT_TRUE(tnw.AddLine(line0));
    }

    TEST(TestTransportNetwork, TestRecordingPassengerEventBasic){
        TransportNetwork tnw {};
        // Define a line with 2 routes going through some shared stations.
        // route0: 0 ---> 1 ---> 2
        // route1: 3 ---> 1 ---> 2
        Station station1 {
            "station1",
            "station1"
        };
        
        Station station2 {
            "station2",
            "station2"
        };
        
        Station station0 {
            "station0",
            "station0"
        };
        
        Station station3 {
            "station3",
            "station3"
        };

        ASSERT_TRUE(tnw.AddStation(station0));
        ASSERT_TRUE(tnw.RecordPassengerEvent({"station0", PassengerEvent::Type::In}));
        // failed as station is not added yet
        ASSERT_FALSE(tnw.RecordPassengerEvent({"station1", PassengerEvent::Type::In}));


    }
    
    TEST(TestTransportNetwork, TestRecordingPassengerEventInOut){
        TransportNetwork tnw {};
        // Define a line with 2 routes going through some shared stations.
        // route0: 0 ---> 1 ---> 2
        // route1: 3 ---> 1 ---> 2
        Station station1 {
            "station1",
            "station1"
        };
       
        Station station0 {
            "station0",
            "station0"
        };
        
        ASSERT_TRUE(tnw.AddStation(station0));
        ASSERT_TRUE(tnw.AddStation(station1));
        const uint32_t num_passenger_in = 100;
        const uint32_t num_passenger_out = 50;
        for (int i = 0; i < num_passenger_in; i++){
            tnw.RecordPassengerEvent({"station0", PassengerEvent::Type::In});
        }
        for (int i = 0; i < num_passenger_out; i++){
            tnw.RecordPassengerEvent({"station0", PassengerEvent::Type::Out});
        }
        ASSERT_TRUE(tnw.GetPassengerCount("station0") == (num_passenger_in - num_passenger_out));
        // negative customer count is valid
        for (int i = 0; i < num_passenger_out; i++){
            tnw.RecordPassengerEvent({"station1", PassengerEvent::Type::Out});
        }
        ASSERT_TRUE(tnw.GetPassengerCount("station1") == (-1) * (int64_t)(num_passenger_out));
    }

    TEST(TestTransportNetwork, TestGetRouteServingAtStation){
        TransportNetwork tnw {};
        Station station1 {
            "station1",
            "station1"
        };
       
        Station station0 {
            "station0",
            "station0"
        };
        
        Station station2 {
            "station2",
            "station2"
        };
        
        Station station3 {
            "station3",
            "station3"
        };
        
        Route route0{
            "route_000",
            "inbound",
            "line_000",
            "station0",
            "station2",
            {"station0", "station1", "station2"}
        };
        
        Route route1{
            "route_001",
            "inbound",
            "line_000",
            "station3",
            "station2",
            {"station3", "station1", "station2"}
        };

        Line line0{
            "line0",
            "Line 0",
            {route0, route1}
        }; 
        ASSERT_TRUE(tnw.AddStation(station0));
        ASSERT_TRUE(tnw.AddStation(station1));
        ASSERT_TRUE(tnw.AddStation(station2));
        ASSERT_TRUE(tnw.AddStation(station3));
        ASSERT_TRUE(tnw.AddLine(line0));

        ASSERT_EQ(tnw.GetRouteServingAtStation("station1").size(), 2);
        ASSERT_EQ(tnw.GetRouteServingAtStation("station2").size(), 2);
        ASSERT_EQ(tnw.GetRouteServingAtStation("station0").size(), 1);
        ASSERT_EQ(tnw.GetRouteServingAtStation("station3").size(), 1);
        ASSERT_THROW(tnw.GetRouteServingAtStation("station4"), std::runtime_error);
    }

    TEST(TestTransportNetwork, TestSetTravelTime){
        TransportNetwork tnw {};
        Station station1 {
            "station1",
            "station1"
        };
       
        Station station0 {
            "station0",
            "station0"
        };
        
        Station station2 {
            "station2",
            "station2"
        };
        
        Station station3 {
            "station3",
            "station3"
        };

 
        Station station4 {
            "station4",
            "station4"
        };
        
        Route route0{
            "route_000",
            "inbound",
            "line_000",
            "station0",
            "station2",
            {"station0", "station1", "station2"}
        };
        
        Route route1{
            "route_001",
            "inbound",
            "line_000",
            "station3",
            "station4",
            {"station3", "station1", "station2", "station4"}
        };

        Line line0{
            "line0",
            "Line 0",
            {route0, route1}
        }; 
        ASSERT_TRUE(tnw.AddStation(station0));
        ASSERT_TRUE(tnw.AddStation(station1));
        ASSERT_TRUE(tnw.AddStation(station2));
        ASSERT_TRUE(tnw.AddStation(station3));
        ASSERT_TRUE(tnw.AddStation(station4));
        ASSERT_TRUE(tnw.AddLine(line0));
        
        // different travel time between two adjacent stations in different directions
        ASSERT_TRUE(tnw.SetTravelTime("station3", "station1", 10));
        ASSERT_EQ(tnw.GetTravelTimeBetweenAdjStations("station3", "station1"), 10);

        // non-adjacent stations
        ASSERT_FALSE(tnw.SetTravelTime("station3", "station2", 10));
        ASSERT_FALSE(tnw.SetTravelTime("station1", "station4", 10));
        ASSERT_FALSE(tnw.SetTravelTime("station4", "station1", 10));

        // Station that's not in the network
        ASSERT_FALSE(tnw.SetTravelTime("station5", "station1", 10));
    }

    TEST(TestTransportNetwork, TestGetTravelTime){
        TransportNetwork tnw {};
        Station station1 {
            "station1",
            "station1"
        };
       
        Station station0 {
            "station0",
            "station0"
        };
        
        Station station2 {
            "station2",
            "station2"
        };
        
        Station station3 {
            "station3",
            "station3"
        };

 
        Station station4 {
            "station4",
            "station4"
        };
        
        Station station5 {
            "station5",
            "station5"
        };
        
        Station station6 {
            "station6",
            "station6"
        };


        Route route0{
            "route0",
            "inbound",
            "line_000",
            "station0",
            "station2",
            {"station0", "station1", "station2"}
        };
        
        Route route1{
            "route1",
            "inbound",
            "line_000",
            "station3",
            "station4",
            {"station3", "station1", "station2", "station4"}
        };
        
        Route route2{
            "route2",
            "inbound",
            "line_001",
            "station4",
            "station0",
            {"station4", "station5", "station1", "station0", "station6"}
        };

        Line line1{
            "line1",
            "Line 1",
            {route2}
        };

        Line line0{
            "line0",
            "Line 0",
            {route0, route1}
        };

        ASSERT_TRUE(tnw.AddStation(station0));
        ASSERT_TRUE(tnw.AddStation(station1));
        ASSERT_TRUE(tnw.AddStation(station2));
        ASSERT_TRUE(tnw.AddStation(station3));
        ASSERT_TRUE(tnw.AddStation(station4));
        ASSERT_TRUE(tnw.AddStation(station5));
        ASSERT_TRUE(tnw.AddStation(station6));
        ASSERT_TRUE(tnw.AddLine(line0));
        ASSERT_TRUE(tnw.AddLine(line1));

        ASSERT_TRUE(tnw.SetTravelTime("station0", "station1", 2));
        ASSERT_TRUE(tnw.SetTravelTime("station1", "station2", 3));
        ASSERT_TRUE(tnw.SetTravelTime("station3", "station1", 1));
        ASSERT_TRUE(tnw.SetTravelTime("station1", "station2", 3));
        ASSERT_TRUE(tnw.SetTravelTime("station2", "station4", 3));
        ASSERT_TRUE(tnw.SetTravelTime("station4", "station5", 3));
        ASSERT_TRUE(tnw.SetTravelTime("station5", "station1", 2));
        ASSERT_TRUE(tnw.SetTravelTime("station1", "station0", 2));
        ASSERT_TRUE(tnw.SetTravelTime("station0", "station6", 10));
        
        // different travel time between two adjacent stations in different directions
        // Line 0 Route0: station0 ---2---> station1 ---3---> station2
        // Line 0 Route1: station3 ---1---> station1 ---3---> station2 ---3--->station4
        // Line 1 Route2: station4 ---3---> station5 ---2---> station1 ---2--->station0---10-->station6
        ASSERT_EQ(tnw.GetTravelTime("line0", "route0", "station0", "station2"), 5);
        ASSERT_EQ(tnw.GetTravelTime("line0", "route0", "station1", "station2"), 3);
        ASSERT_EQ(tnw.GetTravelTime("line0", "route1", "station1", "station4"), 6);
        ASSERT_EQ(tnw.GetTravelTime("line0", "route1", "station1", "station2"), 3);
        ASSERT_EQ(tnw.GetTravelTime("line1", "route2", "station5", "station0"), 4);
        ASSERT_EQ(tnw.GetTravelTime("line1", "route2", "station5", "station1"), 2);
        ASSERT_EQ(tnw.GetTravelTime("line1", "route2", "station4", "station1"), 5);
        ASSERT_EQ(tnw.GetTravelTime("line1", "route2", "station4", "station0"), 7);
        ASSERT_EQ(tnw.GetTravelTime("line1", "route2", "station4", "station6"), 17);

    }
}
