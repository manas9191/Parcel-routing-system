#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <map>
#include <set>
#include <queue>
#include <stack>
#include <list>
#include <utility>
#include <set>
using namespace std;
class Parcel
{
public:
    int time_tick;
    std::string parcel_id;
    std::string origin;
    std::string destination;
    int priority;
    bool delivered;
    std::string current_location;

    Parcel() = default;
    Parcel(int time_tick, std::string parcel_id, std::string origin, std::string destination, int priority)
    {
        this->time_tick = time_tick;
        this->parcel_id = parcel_id;
        this->origin = origin;
        this->destination = destination;
        this->priority = priority;
        this->delivered = false;
        this->current_location = origin;
    }
};

class MaxHeap
{
public:
    std::vector<Parcel *> heap;

    int parent(int i)
    {
        return (i == 0) ? -1 : (i - 1) / 2;
    }

    int left(int i)
    {
        if (2 * i + 1 >= this->heap.size())
            return -1;
        return 2 * i + 1;
    }

    int right(int i)
    {
        if (2 * i + 2 >= this->heap.size())
            return -1;
        return 2 * i + 2;
    }

    Parcel *get_max()
    {
        return this->heap[0];
    }

    Parcel *extract_max()
    {
        Parcel *max = this->heap[0];
        this->heap[0] = this->heap[this->heap.size() - 1];
        this->heap.pop_back();
        max_heapify(0);
        return max;
    }

    void max_heapify(int i)
    {
        int l = left(i);
        int r = right(i);
        int largest = i;
        if (l < this->heap.size() && this->heap[l]->priority > this->heap[largest]->priority)
        {
            largest = l;
        }
        if (r < this->heap.size() && this->heap[r]->priority > this->heap[largest]->priority)
        {
            largest = r;
        }
        if (largest != i)
        {
            std::swap(this->heap[i], this->heap[largest]);
            max_heapify(largest);
        }
    }

    void insert(Parcel *item)
    {
        this->heap.push_back(item);
        int i = this->heap.size() - 1;
        while (i > 0 && this->heap[parent(i)]->priority < this->heap[i]->priority)
        {
            std::swap(this->heap[i], this->heap[parent(i)]);
            i = parent(i);
        }
    }

    bool is_empty()
    {
        return this->heap.empty();
    }
};

class FreightCar
{
public:
    int max_parcel_capacity;
    std::vector<Parcel *> parcels;
    std::string destination_city;
    std::string next_link;
    std::string current_location;
    std::string prev_link;
    bool sealed;
    std::unordered_map<std::string, bool> cities_visited;
    FreightCar(int max_parcel_capacity)
    {
        this->max_parcel_capacity = max_parcel_capacity;
        // instantiate parcels vector as empty initially
        this->parcels = {};
        this->destination_city = "";
        this->next_link = "";
        this->current_location = "";
        this->prev_link = "";
        this->sealed = false;
    }

    void load_parcel(Parcel *parcel)
    {
        if (this->parcels.size() < this->max_parcel_capacity)
        {
            this->parcels.push_back(parcel);
        }
    }

    bool can_move()
    {
        // if length of parcels is equal to max_parcel_capacity return true else return false
        return this->parcels.size() == max_parcel_capacity;
    }

    void move(std::string destination)
    {
        // function that moves freight car to destination (link)
        if (can_move())
            this->current_location = destination;
    }
};

class Vertex
{
public:
    std::string name;
    std::vector<FreightCar *> freight_cars;
    std::vector<Vertex *> neighbors;
    // trains_to_move is dict of destination and freight cars
    std::map<std::string, std::vector<FreightCar *>> trains_to_move;
    // int min_freight_cars_to_move;
    int max_parcel_capacity;
    std::unordered_map<std::string, MaxHeap> parcel_destination_heaps;
    std::vector<FreightCar *> sealed_freight_cars;
    std::vector<Parcel *> all_parcels;
    Vertex() = default;
    Vertex(std::string name, int max_parcel_capacity)
    {
        this->name = name;
        this->freight_cars = {};
        this->neighbors = {};
        this->trains_to_move = {};
        // this->min_freight_cars_to_move = min_freight_cars_to_move;
        this->max_parcel_capacity = max_parcel_capacity;
        this->parcel_destination_heaps = {};
        this->sealed_freight_cars = {};
        this->all_parcels = {};
    }

    void add_neighbor(Vertex *neighbor)
    {
        // add neighbor to neighbors vector
        this->neighbors.push_back(neighbor);
    }

    // add get neighbors function
    std::vector<Vertex *> get_neighbors()
    {
        return this->neighbors;
    }

    std::vector<Parcel *> get_all_current_parcels()
    {
        // get all parcels from all destination heaps
        std::vector<Parcel *> all_parcels;
        for (auto &parcel_pairs : this->parcel_destination_heaps)
        {
            MaxHeap &heap = parcel_pairs.second;
            for (int i = 0; i < heap.heap.size(); i++)
            {
                all_parcels.push_back(heap.heap[i]);
            }
        }
        return all_parcels;
    }

    void clean_unmoved_freight_cars()
    {
        if (this->freight_cars.empty())
            return;

        std::vector<FreightCar *> cars_to_remove;

        for (int i = 0; i < this->freight_cars.size(); ++i)
        {
            FreightCar *freight_car = this->freight_cars[i];
            if (freight_car->parcels.size() < this->max_parcel_capacity)
            {
                for (Parcel *parcel : freight_car->parcels)
                {
                    this->parcel_destination_heaps[parcel->destination].insert(parcel);
                }
                cars_to_remove.push_back(freight_car);
            }
        }

        // Now remove the marked freight cars
        for (FreightCar *car : cars_to_remove)
        {
            auto it = std::find(this->freight_cars.begin(), this->freight_cars.end(), car);
            if (it != this->freight_cars.end())
            {
                this->freight_cars.erase(it);
                delete *it; // Deallocate memory for the removed freight car
            }
        }
    }

    void loadParcel(Parcel *parcel)
    {
        // load parcel into heap
        if (parcel->delivered == false)
        {
            if (this->parcel_destination_heaps.find(parcel->destination) == this->parcel_destination_heaps.end())
            {
                MaxHeap heap;
                heap.insert(parcel);
                this->parcel_destination_heaps[parcel->destination] = heap;
            }
            else
            {
                this->parcel_destination_heaps[parcel->destination].insert(parcel);
            }
            this->all_parcels.push_back(parcel);
        }
    }

    void loadFreightCars()
    {
        // load parcels into freight cars (making sure that each freight car has parcels from only one destination, and # parcels == min_parcel_capacity)
        for (auto &pair : this->parcel_destination_heaps)
        {
            MaxHeap heap = pair.second;
            std::string dest = pair.first;
            int size = heap.heap.size();
            int j = 0, k = 0;
            FreightCar *freight_car = new FreightCar(max_parcel_capacity);
            freight_car->destination_city = dest;
            freight_car->current_location = this->name;
            while (j < size)
            {
                if (k < this->max_parcel_capacity)
                {
                    if (!heap.is_empty())
                    {
                        Parcel *parcel = heap.extract_max();
                        freight_car->load_parcel(parcel);
                        j++;
                        k++;
                    }
                    else
                    {
                        break;
                    }
                }
                else if (k == max_parcel_capacity)
                {
                    this->freight_cars.push_back(freight_car);
                    freight_car = new FreightCar(max_parcel_capacity);
                    freight_car->destination_city = dest;
                    freight_car->current_location = this->name;
                    k = 0;
                }
            }
        }
    }

    void print_parcels_in_freight_cars()
    {
        for (auto &parcel_pair : this->parcel_destination_heaps)
        {
            MaxHeap heap = parcel_pair.second;
            for (auto &parcel : heap.heap)
            {
                std::cout << "Parcel ID: " << parcel->parcel_id << ", Parcel origin: " << parcel->origin << ", Parcel destination: " << parcel->destination << ", Parcel priority: " << parcel->priority << std::endl;
            }
        }
    }
};

class Graph
{

public:
    std::list<Vertex *> vertices;
    std::vector<std::pair<Vertex *, Vertex *>> edges;
    // std::unordered_map<int,std::string> delivered_at_time_tick;
    
    void add_edge(std::string source, std::string destination, int max_parcel_capacity)
    {
        // Check if source and destination vertices exist
        // If they do not exist, create them
        Vertex *src = nullptr;
        Vertex *dest = nullptr;
        for (auto v : vertices)
        {
            if (v->name == source)
            {
                src = v;
            }
            if (v->name == destination)
            {
                dest = v;
            }
        }
        if (src == nullptr)
        {
            src = new Vertex(source, max_parcel_capacity);
            vertices.push_back(src);
        }
        if (dest == nullptr)
        {
            dest = new Vertex(destination, max_parcel_capacity);
            vertices.push_back(dest);
        }
        // Add destination to the neighbor list of source
        src->add_neighbor(dest);
        // Add source to the neighbor list of destination
        dest->add_neighbor(src);
        // Add the edge to the list of edges
        edges.push_back({src, dest});
    }

    void print_graph()
    {
        std::cout << "Printing graph, vertices: " << this->vertices.size() << std::endl;
        for (auto &vertex : this->vertices)
        {
            std::cout << "Vertex: " << vertex->name << std::endl;
            std::cout << "Neighbors: ";
            for (auto &neighbor : vertex->neighbors)
            {
                std::cout << neighbor->name << " ";
            }
            std::cout << std::endl;
        }
    }

    std::vector<std::string> bfs(std::string source, std::string destination)
    {
        // function to return path from source to destination using bfs
        std::vector<std::string> path;
        std::queue<Vertex *> q;
        std::unordered_map<Vertex *, bool> visited;
        std::unordered_map<Vertex *, Vertex *> parent;
        for (auto it : this->vertices)
        {
            visited[it] = false;
            parent[it] = NULL;
        }
        for (auto it : this->vertices)
        {
            if (it->name == source)
            {
                q.push(it);
                visited[it] = true;
                break;
            }
        }
        while (!q.empty())
        {
            Vertex *curr_city = q.front();
            q.pop();
            for (int i = 0; i < curr_city->neighbors.size(); i++)
            {
                if (visited[curr_city->neighbors[i]] == false)
                {
                    visited[curr_city->neighbors[i]] = true;
                    parent[curr_city->neighbors[i]] = curr_city;
                    q.push(curr_city->neighbors[i]);
                    if (curr_city->neighbors[i]->name == destination)
                    {
                        Vertex *node = curr_city->neighbors[i];
                        while (node != NULL)
                        {
                            path.push_back(node->name);
                            node = parent[node];
                        }
                        reverse(path.begin(), path.end());
                        return path;
                    }
                }
            }
        }
        return std::vector<std::string>();
    }

    vector<string> shortest_path(string source, string destination)
    {
        unordered_map<Vertex *, int> distance;
        unordered_map<Vertex *, Vertex *> predecessor;
        set<Vertex *> visited;
        Vertex *src = nullptr;
        Vertex *dest = nullptr;
        for (auto v : this->vertices)
        {
            if (v->name == source)
            {
                src = v;
            }
            if (v->name == destination)
            {
                dest = v;
            }
            distance[v] = numeric_limits<int>::max();
            predecessor[v] = nullptr;
        }

        distance[src] = 0;

        priority_queue<pair<int, Vertex *>, vector<pair<int, Vertex *>>, greater<pair<int, Vertex *>>> pq;
        pq.push({0, src});

        while (!pq.empty())
        {
            Vertex *current = pq.top().second;
            pq.pop();

            if (visited.find(current) != visited.end())
            {
                continue;
            }

            visited.insert(current);

            for (auto neighbor : current->neighbors)
            {

                if (distance[current] + 1 < distance[neighbor])
                {
                    distance[neighbor] = distance[current] + 1;
                    predecessor[neighbor] = current;
                    pq.push({distance[neighbor], neighbor});
                }
            }
        }

        // Reconstruct the shortest path
        vector<string> path;
        while (dest != nullptr)
        {
            path.push_back(dest->name);
            dest = predecessor[dest];
        }

        reverse(path.begin(), path.end());
        return path;
    }

    void set_next_link_of_freight_cars()
    {
        for (auto vertex : this->vertices)
        {
            for (auto freight_car : vertex->freight_cars)
            {
                if (!freight_car->sealed && freight_car->next_link == "")
                {

                    vector<string> path = shortest_path(freight_car->current_location, freight_car->destination_city);
                    freight_car->next_link = path[1];
                }
            }
        }
    }

    void groupFreightCars()
    {
        // for every vertex, group freight cars by links
        this->set_next_link_of_freight_cars();
        for (auto it : this->vertices)
        {
            for (int i = 0; i < it->freight_cars.size(); i++)
            {
                it->trains_to_move[it->freight_cars[i]->next_link].push_back((it->freight_cars[i]));
            }
        }
    }

    void moveTrains(int t)
    {
        // for every vertex, move freight cars to next link
        for (auto it : this->vertices)
        {
            for (auto &it2 : it->trains_to_move)
            {
                string nxt_link = it2.first;
                vector<FreightCar *> freight_cars_on_curr_location = it2.second;
                for (int i = 0; i < freight_cars_on_curr_location.size(); i++)
                {
                    string curr_location = freight_cars_on_curr_location[i]->current_location;
                    freight_cars_on_curr_location[i]->move(nxt_link);
                    for (auto it3 : freight_cars_on_curr_location[i]->parcels)
                    {
                        it3->current_location = nxt_link;
                        cout << "At time tick " << t << " " << it3->parcel_id << " travelled from " << curr_location << " to " << nxt_link << endl;
                        if (it3->current_location == it3->destination)
                        {
                            it3->delivered = true;
                            // this->delivered_at_time_tick[it3->time_tick] = it3->parcel_id;
                        }
                        else{
                            it3->time_tick++;
                        }
                    }
                    if (freight_cars_on_curr_location[i]->destination_city == nxt_link)
                    {
                        freight_cars_on_curr_location[i]->next_link = " ";
                        freight_cars_on_curr_location[i]->prev_link = curr_location;
                        freight_cars_on_curr_location[i]->sealed = true;
                        it->sealed_freight_cars.push_back(freight_cars_on_curr_location[i]);
                        // freight_cars_on_curr_location[i]->parcels.clear();
                        freight_cars_on_curr_location[i]->parcels.clear();
                    }

                    else
                    {
                        vector<string> path = shortest_path(nxt_link, freight_cars_on_curr_location[i]->destination_city);
                        freight_cars_on_curr_location[i]->next_link = path[1];
                        freight_cars_on_curr_location[i]->prev_link = curr_location;
                    }
                }
            }
            // empty the trains to move map after reaching the next link so that rearranging
            // can be done
            it->trains_to_move.erase(it->trains_to_move.begin(), it->trains_to_move.end());
        }
    }
};

class PRC
{
public:
    Graph graph;
    std::vector<FreightCar *> freight_cars;
    std::map<std::string, Parcel *> parcels;
    std::map<int, std::vector<Parcel *>> parcels_with_time_tick;
    int max_parcel_capacity;
    // int min_freight_cars_to_move;
    int time_tick;
    std::map<std::string, std::string> old_state;
    std::map<std::string, std::string> new_state;
    int max_time_tick;

    PRC(int max_parcel_capacity)
    {
        this->max_parcel_capacity = max_parcel_capacity;
        // this->min_freight_cars_to_move = min_freight_cars_to_move;
        this->time_tick = 1;
        this->max_time_tick = 3;
        this->freight_cars = {};
        this->parcels = {};
        this->parcels_with_time_tick = {};
        this->old_state = {};
        this->new_state = {};
    }

    void create_graph(std::string graph_file_name)
    {
        // reading graph.txt file and creating graph
        ifstream graphFile(graph_file_name);
        if (!graphFile.is_open())
        {
            cout << "ERROR opening graph file"
                 << "\n";
        }
        string line;
        while (getline(graphFile, line))
        {
            istringstream iss(line);
            string city1, city2;
            iss >> city1 >> city2;
            this->graph.add_edge(city1, city2, this->max_parcel_capacity);
        }

        graphFile.close();
    }
    // int time_tick, std::string parcel_id, std::string origin, std::string destination, int priority
    void process_parcels(std::string parcels_file_name)
    {
        // reading parcels.txt file and creating parcels
        ifstream parcelFile(parcels_file_name);
        if (!parcelFile.is_open())
        {
            cout << "ERROR opening parcels file"
                 << "\n";
        }
        string line;
        while (getline(parcelFile, line))
        {
            istringstream iss(line);
            int time_tick, priority;
            string src, dest, parcel_id;
            iss >> time_tick >> parcel_id >> src >> dest >> priority;
            // defining the parcel and assigning a pointer to it
            Parcel *pt = new Parcel(time_tick, parcel_id, src, dest, priority);
            this->parcels[parcel_id] = pt;
            this->parcels_with_time_tick[time_tick].push_back(pt);
        }
    }
    // function to return bookings at a particular time tick at a particular vertex
    std::vector<Parcel *> get_bookings(int time_tick, std::string vertex)
    {
        vector<Parcel *> parcels_at_given_vertex;
        for (auto it : this->parcels_with_time_tick[time_tick])
        {
            if (it->origin == vertex)
            {
                parcels_at_given_vertex.push_back(it);
            }
        }
        return parcels_at_given_vertex;
    }

    std::map<std::string, std::string> get_state()
    {
        std::map<std::string, std::string> state;
        // from this.parcels
        // should return the dict of parcel_id and current_location
        for (auto &it : this->parcels)
        {
            string parcel_id = it.first;
            state[parcel_id] = it.second->current_location;
        }
        return state;
    }

    // run simulation function, takes run_till_time_tick as argument
    // if run_till_time_tick is not provided then run simulation till max_time_tick

    void run_simulation(int run_till_time_tick = -1)
    {
        // function to run simulation
        // this->graph.print_graph();
        if (run_till_time_tick == -1)
        {
            run_till_time_tick = this->max_time_tick;
        }

        for (int time_tick = 1; time_tick <= run_till_time_tick; time_tick++)
        {
            cout << "Time Tick: " << time_tick << " " << endl;
            // int i = 0;
            for (auto vertex : this->graph.vertices)
            {
                vector<Parcel *> parcels_at_given_vertex = get_bookings(time_tick, vertex->name);
                for (auto parcel : parcels_at_given_vertex)
                {
                    // cout << i++ ;
                    vertex->loadParcel(parcel);
                }
                vertex->loadFreightCars();
                vertex->clean_unmoved_freight_cars();
                vertex->print_parcels_in_freight_cars();
            }
            this->graph.groupFreightCars();
            this->graph.moveTrains(time_tick);
            for (auto vertex : this->graph.vertices)
            {
                // Use remove_if with a lambda function to identify elements to remove
                vertex->freight_cars.erase(
                    std::remove_if(
                        vertex->freight_cars.begin(),
                        vertex->freight_cars.end(),
                        [&](FreightCar *freight_car)
                        {
                            if (freight_car->prev_link == vertex->name)
                            {
                                for (auto next_vertex : this->graph.vertices)
                                {
                                    if (freight_car->current_location == next_vertex->name)
                                    {
                                        next_vertex->freight_cars.push_back(freight_car);
                                        return true; // Indicate that the freight car should be removed
                                    }
                                }
                            }
                            return false; // Keep the freight car
                        }),
                    vertex->freight_cars.end());
            }
            for (auto vertex : this->graph.vertices)
            {
                // Use remove_if with a lambda function to identify elements to remove
                vertex->all_parcels.erase(
                    std::remove_if(
                        vertex->all_parcels.begin(),
                        vertex->all_parcels.end(),
                        [&](Parcel *parcel)
                        {
                            if(parcel->current_location == parcel->destination) {
                                return true;
                            }
                            else if (parcel->current_location != vertex->name)
                            {
                                // Move the parcel to the vertex with the same name as its current location
                                for (auto next_vertex : this->graph.vertices)
                                {
                                    if (parcel->current_location == next_vertex->name)
                                    {
                                        // Move the parcel to the next vertex
                                        next_vertex->all_parcels.push_back(parcel);
                                        return true;
                                    }
                                }
                            }
                            return false;
                        }),
                    vertex->all_parcels.end());

            }
            for(auto& it: parcels_with_time_tick){
                vector<Parcel*> parcels = it.second;
                parcels.erase(
                    remove_if(
                        parcels.begin(),
                        parcels.end(),
                        [&](Parcel *parcel)
                        {
                            if(parcel->delivered == true){
                                return true;
                            }
                            return false;
                        }),
                        parcels.end());
            }
            bool all_parcels_delivered = true;
            for (auto vertex : this->graph.vertices)
            {
                for(auto it: vertex->all_parcels){
                    if(!it->delivered){
                        all_parcels_delivered = false;
                    }
                }
            }
            if (all_parcels_delivered)
            {
                break;
            }
        }
    }

    // bool convergence_check()
    // {
    //     // function to check convergence between old_state and new_state
    // }

    bool all_parcels_delivered()
    {
        for (auto &parcel_pair : this->parcels)
        {
            Parcel *parcel = parcel_pair.second;
            if (!parcel->delivered)
            {
                return false;
            }
        }
        return true;
    }

    std::vector<std::string> get_stranded_parcels()
    {
        std::vector<std::string> stranded_parcels;
        for (auto &parcel_pair : this->parcels)
        {
            Parcel &parcel = *parcel_pair.second;
            if (!parcel.delivered && parcel.current_location != parcel.destination)
            {
                stranded_parcels.push_back(parcel.parcel_id);
            }
        }
        return stranded_parcels;
    }

    std::vector<std::string> get_delivered_parcels()
    {
        // return parcel_ids of delivered parcels
        std::vector<std::string> delivered_parcels;
        for (auto &parcel_pair : this->parcels)
        {
            Parcel &parcel = *parcel_pair.second;
            if (parcel.delivered)
            {
                delivered_parcels.push_back(parcel.parcel_id);
            }
        }
        return delivered_parcels;
    }

    // get status of all parcels at time tick t (status = id, current location, delivered/not delivered)
    std::vector<std::string> get_status_of_all_parcels(int t)
    {
        //  std::string status = parcel.parcel_id + " " + parcel.current_location + " " + std::to_string(parcel.delivered) + "\n" ;
        // return the status of all parcels at time tick t as a vector of strings (each string formatted as above)
        std::vector<std::string> status;
        for (auto &parcel_pair : this->parcels)
        {
            Parcel* parcel = parcel_pair.second;
            if (true)
            {
                std::string status_str = parcel->parcel_id + " " + parcel->current_location + " " + std::to_string(parcel->delivered) + "\n";
                status.push_back(status_str);
            }
        }
        return status;
    }

    // get status of parcel_id at time tick t (status = id, current location, delivered/not delivered)
    std::string get_status_of_parcel(std::string parcel_id, int t)
    {
        // status = parcel.parcel_id + " " + parcel.current_location + " " + std::to_string(parcel.delivered);
        // return status of one parcel at time tick t as a string (formatted as above)
        Parcel *parcel = this->parcels[parcel_id];
        if (parcel->time_tick == t)
        {
            std::string status_str = parcel->parcel_id + " " + parcel->current_location + " " + std::to_string(parcel->delivered);
            return status_str;
        }
        else
        {
            // Return an empty string if the parcel's time tick doesn't match
            return "";
        }
    }

    // get parcels delivered up to time tick t
    // std::vector<std::string> get_parcels_delivered_up_to_time_tick(int t)
    // {
    //     // return list of parcel_ids of parcels delivered up to time tick t
    //     std::vector<std::string> delivered_parcels;
    //     for(auto it: this->graph.vertices){
    //         for(auto it2 : it->all_parcels){
    //             if(it2->time_tick <= t && it2->delivered == true){
    //                 delivered_parcels.push_back(it2->parcel_id);
    //             }
    //         }
    //     }
    //     return delivered_parcels;
    // }
};

int main()
{

    PRC prc = PRC(4);
    prc.create_graph("C:/Users/manas/OneDrive/Documents/dsa/samples/3/graph.txt");
    prc.process_parcels("C:/Users/manas/OneDrive/Documents/dsa/samples/3/bookings.txt");
    prc.run_simulation(4);

    std::cout << "All parcels delivered: " << (prc.all_parcels_delivered() ? "Yes" : "No") << "\n";
    std::cout << "Stranded parcels: ";
    for (const auto &parcel_id : prc.get_stranded_parcels())
    {
        std::cout << parcel_id << " ";
    }
    std::cout << "\nDelivered parcels: ";
    for (const auto &parcel_id : prc.get_delivered_parcels())
    {
        std::cout << parcel_id << " ";
    }
    std::cout << "\n";

    std::cout << "Status of all parcels at time tick 3:\n";
    for (const auto &status_str : prc.get_status_of_all_parcels(5))
    {
        std::cout << status_str;
    }
    std::cout << "\n";
}
