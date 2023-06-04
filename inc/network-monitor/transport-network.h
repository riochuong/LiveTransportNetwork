#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include <nlohmann/json.hpp>
namespace NetworkMonitor{

    /*! \brief A station, line, or routeID is represented by a string
    */
    using Id = std::string;

    struct Station {
        Id id {};
        std::string name {};

        /*! \brief Comparing two stations to be equals if they have same id
        */
        bool operator==(const Station& other) const;
    };

    /*! \brief Network Route
     *  Each line might have on or more routes. A rout represent a single possible journeys through a
     *  set of stops in a specified direction.
     *  There maybe or maynot be a corresponding route in the opposite direction of travel
     * 
    */
    struct Route {
        Id id {};
        std::string direction {};
        Id lineId {};
        Id startStationId;
        Id endStationId;
        std::vector<Id> stops;

        /*! \brief Two routes are equal if they have the same id
        */
        bool operator==(const Route& other) const;
    };

    /*! \brief Line 
     * Represent a collection of routes acrocss multiple stastions 
     *
    */
    struct Line {
        Id id {};
        std::string name {};
        std::vector<Route> routes;
    };

    struct PassengerEvent {
        enum Type {
            In,
            Out
        };
        Id stationId {};
        Type type {Type::In};
    };

    /*! \brief Underground network representation 
     *
    */
    class TransportNetwork {    
    public:

        /*! \brief Default constructor
        */
        TransportNetwork();

        /*! \brief Destructor */
        ~TransportNetwork(){};

        TransportNetwork(const TransportNetwork& copy);

        TransportNetwork(TransportNetwork&& moved);

        TransportNetwork& operator=(const TransportNetwork& copy);

        TransportNetwork& operator=(TransportNetwork&& moved);

        /*! \brief Add a station to the network
         *  \returns false if failed to add new station to the network.
         *  
         *  This function assumes that the Station is well-formed.
         *  The station cannot already be in the network. 
        */
        bool AddStation(const Station& station);

        /*! \brief Add a line to the network
         *   \returns false if there was an error while adding new line the network
         * 
         * This function assumes that the Line object is well-formed
         * All stations served by this line must already be in the network. The line cannot already be in the 
         * network. 
        */
        bool AddLine(const Line& line);

        /*! \brief Record a passenger event at a station.
         *
         * \returns false if the station is not in the network or if the passenger is not recognized  
        */
        bool RecordPassengerEvent(const PassengerEvent& event);

        /*! \brief Get total number of passsenger at a station
         *  
         *  The returned number can be negative: this happens if we  start recording in the middle of 
         *  the day and we record more exiting than entering passengers. 
         * 
         * \throws std::runtime_error if the station is not in the network
        */
        int64_t GetPassengerCount(const Id& station);
        
        /*! \brief Get routes that are served at this station
         *
         * \returns An empty error if there was error getting the list of routes serving or if the station
         * has legitimately no routes serving it.
        */
        std::vector<Id> GetRouteServingAtStation(const Id& station);

        /*! \brief Set travel time between two adjacent stations
        *   \returns false if one of the station is not already added to the network. 
        *   
        *   Travel time is same for all routes connecting the two stations directly.
        *   
        *   The two stations must be adjacent in at least one line route. The two stations must be already 
        *   in the network.
        */
        bool SetTravelTime(const Id& stationA, const Id& stationB, const unsigned int travelTime);


        /*! \brief Get the travel time between two adjacent stations
         * \returns 0 if the function cannot find the travel time between two adjacent stations.
         *
         *  The two stations must be adjacent in at least one line route. The two stations must already be in the network
        */
        unsigned int GetTravelTimeBetweenAdjStations(const Id& stationA, const Id& stationB);

        /*! \brief Get the total travel time between any 2 stations on a specific route
         *    
         *   The total travel time is the cumulative sum of the travel times between all stations betwen stationA and stationB
         * 
         * \returns 0 if the function could not find the travel time between the two stations of if station A and B are the same.
         * 
         *   The two stations mubse be both served byt the given `route`. The two stations must be in the network. And given route must 
         *   be part of the line.
         * 
        */
        unsigned int GetTravelTime(const Id& line, const Id& route, const Id& stationA, const Id& stationB);

        /*! \brief Populate the newtwork from JSON object
         *  \param src ownership of the source json file is moved into this method
         * 
         *  \return false if stations and lines where parsed successfully, but not the travel times.
         * 
         *  \throws std::runtime_error this mthod throws if the JSON items were parsed correctly 
         *                      but there was an issue adding new stations or lines to the network
         * 
         *  \throws nlohmann::json::exception If there was a problem parsing the JSON object 
         * 
         * 
         */
        bool FromJson(nlohmann::json&& src);
    private:
        // Graph Adjacency list representations 
        struct Node;
        struct Edge;
        struct RouteInternal;
        struct LineInternal;
        

        struct Edge {
            Id routeId {};
            Id lineId {};
            unsigned int travelTime {};
            std::shared_ptr<Node> nextStop {nullptr};
        };

       
        struct RouteInternal {
            Id id {};
            std::shared_ptr<LineInternal> line {nullptr};
            std::vector<std::shared_ptr<Node>> stops {};
        };
        
        struct LineInternal{
            Id id{};
            std::string name{};
            std::unordered_map<Id, RouteInternal> routeMaps{};
        };

        struct Node
        {
            Id station_id{};
            std::string name{};
            uint64_t passengerCount = 0;
            std::vector<std::shared_ptr<Edge>> edges{};

            // find the edge of next stop  for specific route
            std::vector<std::shared_ptr<Edge>>::const_iterator FindEdegesForRoute(const Id &route) const;
        };

        std::unordered_map<Id, Node> nodeDict_;
        std::unordered_map<Id, LineInternal> lineDict_;
    };

}