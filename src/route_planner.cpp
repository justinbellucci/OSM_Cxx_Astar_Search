#include "route_planner.h"
#include <algorithm>

RoutePlanner::RoutePlanner(RouteModel &model, float start_x, float start_y, float end_x, float end_y): m_Model(model) {
    // Convert inputs to percentage:
    start_x *= 0.01;
    start_y *= 0.01;
    end_x *= 0.01;
    end_y *= 0.01;

    // TODO 2: Use the m_Model.FindClosestNode method to find the closest nodes to the starting and ending coordinates.
    // Store the nodes you find in the RoutePlanner's start_node and end_node attributes.
    
    start_node = &m_Model.FindClosestNode(start_x, start_y);
    end_node = &m_Model.FindClosestNode(end_x, end_y);
    
    // just some tests to help understand pointers and dereferance operators
    // std::cout << "start node: " <<  (*start_node).x << "\n";
    // std::cout << "start node: " << (*start_node).visited << "\n";
}


// TODO 3: Implement the CalculateHValue method.
// Tips:
// - You can use the distance to the end_node for the h value.
// - Node objects have a distance method to determine the distance to another node.

float RoutePlanner::CalculateHValue(RouteModel::Node const *node) {
    // return (*node).distance(*end_node); // dereference operator usesd
    return node->distance(*end_node); // same as above but sexier
}


// TODO 4: Complete the AddNeighbors method to expand the current node by adding all unvisited neighbors to the open list.
// Tips:
// - Use the FindNeighbors() method of the current_node to populate current_node.neighbors vector with all the neighbors.
// - For each node in current_node.neighbors, set the parent, the h_value, the g_value. 
// - Use CalculateHValue below to implement the h-Value calculation.
// - For each node in current_node.neighbors, add the neighbor to open_list and set the node's visited attribute to true.

void RoutePlanner::AddNeighbors(RouteModel::Node *current_node) {
    current_node->FindNeighbors(); // dereference current_node and access FindNeighbors() method

    for (auto neighbor : current_node->neighbors) {
        neighbor->parent = current_node;
        neighbor->g_value = current_node->g_value + current_node->distance(*neighbor);
        neighbor->h_value = CalculateHValue(neighbor);

        // add neighbor to open_list
        open_list.emplace_back(neighbor);
        // set the node to visited
        neighbor->visited = true;
        // std::cout << "--- G-Val: " << neighbor->g_value << "\n";
    }
}


// TODO 5: Complete the NextNode method to sort the open list and return the next node.
// Tips:
// - Sort the open_list according to the sum of the h value and g value.
// - Create a pointer to the node in the list with the lowest sum.
// - Remove that node from the open_list.
// - Return the pointer.

// bool compareFval(RouteModel::Node *a, RouteModel::Node *b) {
//         float fval_a = a->g_value + a->h_value;
//         float fval_b = b->g_value + b->h_value;
//         return fval_a < fval_b;
//     }
    
RouteModel::Node *RoutePlanner::NextNode() {
    // std::sort(open_list.begin(), open_list.end(), compareFval);

    // sort function by smallest f_value

    std::sort(open_list.begin(), open_list.end(), [](RouteModel::Node *lhs, RouteModel::Node *rhs) {
        return (lhs->g_value + lhs->h_value < rhs->g_value + rhs->h_value);
    });

    // create pointer to the node with the lowest f_value. 
    // Note: list::front returns a direct referance
    RouteModel::Node *node_low_fval = open_list.front();

    // remove element by position. 
    // Note: list::begin returns an iterator
    open_list.erase(open_list.begin());

    // return the pointer to the node with the lowest f_val
    return node_low_fval;
}


// TODO 6: Complete the ConstructFinalPath method to return the final path found from your A* search.
// Tips:
// - This method should take the current (final) node as an argument and iteratively follow the 
//   chain of parents of nodes until the starting node is found.
// - For each node in the chain, add the distance from the node to its parent to the distance variable.
// - The returned vector should be in the correct order: the start node should be the first element
//   of the vector, the end node should be the last element.

std::vector<RouteModel::Node> RoutePlanner::ConstructFinalPath(RouteModel::Node *current_node) {
    // Create path_found vector
    distance = 0.0f;
    std::vector<RouteModel::Node> path_found{};

    // TODO: Implement your solution here.
    RouteModel::Node parent_node;

    // go from the current node to the start where the parent is a null pointer.
    while (current_node->parent != nullptr) {

        // dereference and add the current node to the end of the path vector
        path_found.push_back(*current_node);
        // dereference the current nodes parent and set equal to parent node
        parent_node = *(current_node->parent);
        // calculate distance from current node to parent
        distance += current_node->distance(parent_node);
        // set parent to current node
        current_node = current_node->parent;

    }
    // need to add the last current node to the path
    path_found.push_back(*current_node);
    // reverse the vector
    std::reverse(path_found.begin(), path_found.end());

    distance *= m_Model.MetricScale(); // Multiply the distance by the scale of the map to get meters.
    return path_found;

}


// TODO 7: Write the A* Search algorithm here.
// Tips:
// - Use the AddNeighbors method to add all of the neighbors of the current node to the open_list.
// - Use the NextNode() method to sort the open_list and return the next node.
// - When the search has reached the end_node, use the ConstructFinalPath method to return the final path that was found.
// - Store the final path in the m_Model.path attribute before the method exits. This path will then be displayed on the map tile.

void RoutePlanner::AStarSearch() {
    RouteModel::Node *current_node = nullptr;
    
    // TODO: Implement your solution here.
    // add start_node to open_list and set to visted
    open_list.push_back(start_node);
    start_node->visited = true;
    // while open_list has elements in it
    while (open_list.size() > 0) {
        // current node == node with lowest f_value 
        current_node = NextNode();
        // if current node is end goal return model path
        if (current_node->distance(*end_node) == 0) {
            m_Model.path = ConstructFinalPath(current_node);
            return;
        } 
        // else add neighbors to open_list
        else {
            AddNeighbors(current_node);
        }
    }
        
}