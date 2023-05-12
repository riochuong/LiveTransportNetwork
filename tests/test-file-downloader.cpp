#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

#include "network-monitor/file-downloader.h"

TEST(TestDownloadFileSuite, TestDownloadNetworkLayout){
    ASSERT_TRUE(NetworkMonitor::DownloadFile("https://ltnm.learncppthroughprojects.com/network-layout.json",
                                 std::filesystem::temp_directory_path() / "network-layout.json",
                                 std::filesystem::path(TEST_CACERT_PEM)
                                 ));

    std::filesystem::path output_json_file = std::filesystem::temp_directory_path() / "network-layout.json"; 

    ASSERT_TRUE(std::filesystem::exists(output_json_file));
    // Check the content of the file.

    // We cannot check the whole file content as it changes over time, but we
    // can at least check some expected file properties.

    const std::string expectedString{"\"stations\": ["};
    auto filepath = std::filesystem::temp_directory_path() / "network-layout.json";
    std::ifstream file{filepath};
    std::string line{};
    bool foundExpectedString{false};
    while (std::getline(file, line))
    {
        if (line.find(expectedString) != std::string::npos)
        {
            foundExpectedString = true;
            break;
        }
    }
    ASSERT_TRUE(foundExpectedString);
}

TEST(TestDownloadFileSuite, TestParseJsonNetworkLayout){
    std::filesystem::path network_layout_file{TEST_NETWORK_LAYOUT_JSON};
    nlohmann::json data = NetworkMonitor::ParseJsonFile(network_layout_file);
    ASSERT_TRUE(data.size() > 0);
    ASSERT_TRUE(data.is_object());
    ASSERT_TRUE(data.contains("lines"));
    ASSERT_TRUE(data.at("lines").size() > 0);
    ASSERT_TRUE(data.contains("stations"));
    ASSERT_TRUE(data.at("stations").size() > 0);
    ASSERT_TRUE(data.contains("travel_times"));
    ASSERT_TRUE(data.at("travel_times").size() > 0);
}