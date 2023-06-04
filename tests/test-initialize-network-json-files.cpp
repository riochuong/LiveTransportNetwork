
#include <gtest/gtest.h>
#include <network-monitor/transport-network.h>
#include <network-monitor/file-downloader.h>
#include <logging.h>

#ifndef TEST_FOLDER 
#define TEST_FOLDER "json-test-files"
#endif

namespace NetworkMonitor
{
    TEST(TestTransportNetwork, BasicNetworkLayoutWithJson){
        auto networJsonFile = std::filesystem::path(TEST_FOLDER) / "full-network-layout.json";
        logger::info("Json file path {}", networJsonFile.c_str());
        ASSERT_TRUE(std::filesystem::exists(networJsonFile));
        nlohmann::json data = NetworkMonitor::ParseJsonFile(networJsonFile);
        TransportNetwork tnw {};
        ASSERT_TRUE(tnw.FromJson(std::move(data)));
        // randomly checking some data
        std::vector<Id> routes = tnw.GetRouteServingAtStation("station_136");
        ASSERT_TRUE(std::find(routes.begin(),
                              routes.end(), 
                              "route_015") == routes.end()); 
        routes = tnw.GetRouteServingAtStation("station_086");
        ASSERT_TRUE(std::find(routes.begin(),
                              routes.end(), 
                              "route_018") == routes.end()); 
        
        ASSERT_EQ(tnw.GetTravelTime("line_001", "route_002", "station_044", "station_045"), 1);
        ASSERT_EQ(tnw.GetTravelTime("line_001", "route_006", "station_069", "station_070"), 3);
        ASSERT_EQ(tnw.GetTravelTime("line_002", "route_012", "station_095", "station_096"), 1);
    }

    // TEST(TestTransportNetwork, MissingTravelTime){
    //     auto networJsonFile = std::filesystem::path(TEST_FOLDER) / "missing-travel-time-layout.json";
    //     logger::info("Json file path {}", networJsonFile.c_str());
    //     ASSERT_TRUE(std::filesystem::exists(networJsonFile));
    //     nlohmann::json data = NetworkMonitor::ParseJsonFile(networJsonFile);
    //     TransportNetwork tnw {};
    //     ASSERT_FALSE(tnw.FromJson(std::move(data))); // missing data 
    // }
}

