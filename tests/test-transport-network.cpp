
#include <gtest/gtest.h>
#include <network-monitor/transport-network.h>

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
        ok = nw.AddLine(line);
        ASSERT_TRUE(ok);
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


        tnw.AddStation(station0);
        tnw.AddStation(station1);
        tnw.AddStation(station2);

        Route route0 {}

    }
}
