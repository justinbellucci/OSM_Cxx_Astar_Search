#include "route_planner.h"
#include <algorithm>

RoutePlanner::RoutePlanner(RouteModel &model, float start_x, float start_y, float end_x, float end_y): m_Model(model) {
    // Convert inputs to percentage:
    start_x *= 0.01;
    start_y *= 0.01;
    end_x *= 0.01;
    end_y *= 0.01;

    start_node = &m_Model.FindClosestNode(start_x, start_y);
    end_node = &m_Model.FindClosestNode(end_x, end_y);
}

/* Calculate the Heuristic using the Euclildian distance method for a Node  */
float RoutePlanner::CalculateHValue(RouteModel::Node const *node) {
    return node->distance(*end_node); 
}

/* Add neighbor nodes to the open_list while updating the nodes g, h, and visted values */
void RoutePlanner::AddNeighbors(RouteModel::Node *current_node) {
    current_node->FindNeighbors(); 

    for (auto &neighbor_node : current_node->neighbors) {
        neighbor_node->parent = current_node;
        neighbor_node->g_value = current_node->g_value + current_node->distance(*neighbor_node);
        neighbor_node->h_value = CalculateHValue(neighbor_node);

        open_list.emplace_back(neighbor_node);
        neighbor_node->visited = true;
    }
}

/* Sort the open_list from lowest to highest f_val (g_val + h_val).
   Remove the Node with the lowest f_value and return the pointer. */
RouteModel::Node *RoutePlanner::NextNode() {
    std::sort(open_list.begin(), open_list.end(), [](RouteModel::Node *lhs, RouteModel::Node *rhs) {
        return lhs->g_value + lhs->h_value < rhs->g_value + rhs->h_value;
    });

    RouteModel::Node *node_low_fval = open_list.front();
    open_list.erase(open_list.begin());

    return node_low_fval;
}

/* Iteratively follow the chain of parent nodes until the starting node is found. Add
   the nodes to the path_found vector and sum the total distanc. The return vector is 
   reversed so the path can be recreated from start to end. */
std::vector<RouteModel::Node> RoutePlanner::ConstructFinalPath(RouteModel::Node *current_node) {
    distance = 0.0f;
    std::vector<RouteModel::Node> path_found{};
    RouteModel::Node parent_node;

    while (current_node->parent != nullptr) {
        path_found.push_back(*current_node);
        parent_node = *(current_node->parent);
        distance += current_node->distance(parent_node);
        current_node = current_node->parent;
    }

    // add last current node to the path
    path_found.push_back(*current_node);
    std::reverse(path_found.begin(), path_found.end());
    // Multiply the distance by the scale of the map to get meters.
    distance *= m_Model.MetricScale(); 
    
    return path_found;
}

/* AStart search algorithm implemented. Returns the m_Model path vector of nodes. */
void RoutePlanner::AStarSearch() {
    RouteModel::Node *current_node = nullptr;
    open_list.push_back(start_node);
    start_node->visited = true;

    // while the list is not empty
    while (!open_list.empty()) {
        // get node with lowest f_value 
        current_node = NextNode();
        // return if current node equals goal 
        if (current_node->distance(*end_node) == 0) {
            m_Model.path = ConstructFinalPath(current_node);
            return;
        } else {
            // check all neighbors
            AddNeighbors(current_node);
        }
    }
}