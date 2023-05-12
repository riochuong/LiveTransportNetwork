#include <curl/curl.h>
#include <fstream>

#include "logging.h"
#include "network-monitor/file-downloader.h"

bool NetworkMonitor::DownloadFile(
    const std::string& fileUrl,
    const std::filesystem::path& destination,
    const std::filesystem::path& caCertFile)
{
    CURLcode ret;
    bool res = true;
    FILE* output_file_ptr = NULL; 
    CURL *curl = curl_easy_init();
    if (!curl) {
        logger::critical("Failed to initialize libcurl !");
        res = false;
        goto cleanup;
    }       
    output_file_ptr = fopen(destination.c_str(),"w");
    if (!output_file_ptr){
        logger::error("Cannot open file {} for writing", destination.c_str());
        res = false;
        goto cleanup;
    }
    curl_easy_setopt(curl, CURLOPT_URL, fileUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); 
    curl_easy_setopt(curl, CURLOPT_CAINFO, caCertFile.string().c_str());
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L); 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*) output_file_ptr);

    ret = curl_easy_perform(curl);
    if (ret != CURLE_OK) {
        logger::error("CURL hit error {}", ret);
        res = false;
        goto cleanup;
    }
    return true;
cleanup:
    if (curl) {curl_easy_cleanup(curl);}
    if (output_file_ptr){fclose(output_file_ptr);} 
    return res;
}


nlohmann::json NetworkMonitor::ParseJsonFile(const std::filesystem::path& source){
    std::ifstream ifs{source};
    nlohmann::json data = {};
    if (!std::filesystem::exists(source)){
        return data;
    }
    try{
        ifs >> data;
    } catch (const std::exception &exc){
        logger::error("Failed to parse JSON file at {}. Exception: {}", source.c_str(), exc.what());
    }
    ifs.close();
    return data;
}