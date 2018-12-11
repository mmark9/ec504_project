/*
 * EC504 Final Project
 * Author: Michael Graziano
 * Author: Miguel Mark
 */

#include <map>
#include <vector>
#include <string>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <algorithm>
#include <time.h>
#include "logging.h"
#include "rapidxml.hpp"
#include "maze_api.hpp"

clock_t clock(void);


struct Node {
    
    // Empty Initialization
    Node() {
        priority = UINT32_MAX;
        start_dis = UINT32_MAX;
        end_dis = 0;
        previous = 0;
        queue_pos = 0;
        dead_end = false;
    }

    // Used for setting the node for pathfinding algorithm    
    void Setup(uint32_t distance) {
        priority = UINT32_MAX;
        start_dis = UINT32_MAX;
        end_dis = distance;
        previous = 0;
        queue_pos = 0;
        dead_end = false;
    }

    uint32_t priority;
    uint32_t start_dis;
    uint32_t end_dis;
    uint32_t previous;
    uint32_t queue_pos;
    bool dead_end;
};

// Global Variables for Pathfinding Algorithms
struct Node* node_list;  // Keeps track of nodes
uint32_t* minHeap;       // Priority heap for organizing
uint32_t heapSize = 0;   // Used to maintain the size of the priority heap
uint32_t maxnodes;       // Holds the size of the node_list, minHeap


bool line_segment_exists(const Line& line_seg, std::vector<Line*> line_list) {
	for (std::vector<Line*>::const_iterator line = line_list.begin();
			line != line_list.end(); line++) {
		if (line_seg.x1 >= (*line)->x1 && line_seg.x2 <= (*line)->x2
				&& line_seg.y1 >= (*line)->y1 && line_seg.y2 <= (*line)->y2)
			return true;
	}
	return false;
}

void write_solution_to_file(const std::string& path, const MazeMatrix& maze,
		const std::vector<Line*>& line_list, uint32_t img_width,
		uint32_t img_height) {
	FILE* out_fh = fopen(path.c_str(), "w");
	fprintf(out_fh, "<?xml version=\"1.0\" standalone=\"no\"?>\n");
	fprintf(out_fh, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" "
			"\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
	fprintf(out_fh, "<svg width=\"%u\" height=\"%u\" "
			"version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">\n",
			img_width, img_height);
	fprintf(out_fh, "\t<title>Maze Solution</title>\n"
			"\t<desc>None</desc>\n");
	for (std::vector<Line*>::const_iterator it = line_list.begin();
			it != line_list.end(); it++) {
		fprintf(out_fh, "\t<line "
				"x1=\"%u\" "
				"y1=\"%u\" "
				"x2=\"%u\" "
				"y2=\"%u\" "
				"fill=\"none\" "
				"stroke=\"%s\" "
				"stroke-width=\"%u\" "
				"stroke-linecap=\"%s\"/>\n", (*it)->x1, (*it)->y1, (*it)->x2,
				(*it)->y2, COLOR_BLACK, LINE_WIDTH, STROKE_LINE_CAP_SQUARE);
	}
	Line temp_line;
	MazeCell* maze_cell = NULL;
	for (int row = 0; row < maze.size(); row++) {
		for (int col = 0; col < maze.size(); col++) {
			maze_cell = maze[row][col];
			if (maze_cell->LeftStrokeOn()) {
				temp_line = maze_cell->GetLeftStroke();
				fprintf(out_fh, "\t<line "
						"x1=\"%u\" "
						"y1=\"%u\" "
						"x2=\"%u\" "
						"y2=\"%u\" "
						"fill=\"none\" "
						"stroke=\"%s\" "
						"stroke-width=\"%u\" "
						"stroke-linecap=\"%s\"/>\n", temp_line.x1, temp_line.y1,
						temp_line.x2, temp_line.y2,
						maze_cell->GetLeftStrokeColor(), LINE_WIDTH,
						STROKE_LINE_CAP_ROUND);
			}
			if (maze_cell->UpStrokeIsOn()) {
				temp_line = maze_cell->GetUpStroke();
				fprintf(out_fh, "\t<line "
						"x1=\"%u\" "
						"y1=\"%u\" "
						"x2=\"%u\" "
						"y2=\"%u\" "
						"fill=\"none\" "
						"stroke=\"%s\" "
						"stroke-width=\"%u\" "
						"stroke-linecap=\"%s\"/>\n", temp_line.x1, temp_line.y1,
						temp_line.x2, temp_line.y2,
						maze_cell->GetUpStrokeColor(), LINE_WIDTH,
						STROKE_LINE_CAP_ROUND);
			}
			if (maze_cell->RightStrokeOn()) {
				temp_line = maze_cell->GetRightStroke();
				fprintf(out_fh, "\t<line "
						"x1=\"%u\" "
						"y1=\"%u\" "
						"x2=\"%u\" "
						"y2=\"%u\" "
						"fill=\"none\" "
						"stroke=\"%s\" "
						"stroke-width=\"%u\" "
						"stroke-linecap=\"%s\"/>\n", temp_line.x1, temp_line.y1,
						temp_line.x2, temp_line.y2,
						maze_cell->GetRightStrokeColor(), LINE_WIDTH,
						STROKE_LINE_CAP_ROUND);
			}
			if (maze_cell->DownStrokeOn()) {
				temp_line = maze_cell->GetDownStroke();
				fprintf(out_fh, "\t<line "
						"x1=\"%u\" "
						"y1=\"%u\" "
						"x2=\"%u\" "
						"y2=\"%u\" "
						"fill=\"none\" "
						"stroke=\"%s\" "
						"stroke-width=\"%u\" "
						"stroke-linecap=\"%s\"/>\n", temp_line.x1, temp_line.y1,
						temp_line.x2, temp_line.y2,
						maze_cell->GetDownStrokeColor(), LINE_WIDTH,
						STROKE_LINE_CAP_ROUND);
			}
		}
	}
	fprintf(out_fh, "</svg>\n");
	fclose(out_fh);
}

void upHeap(uint32_t new_pos) {
    /*
     * Code is imported from Homework_05 coding assignment
     * Performs the upHeap operation for the priority heap
     */

    // Variable Declaration:
    uint32_t temp;                                          // Used to hold data during swaps.
    uint32_t target = minHeap[new_pos];                     // Holds the node being upheaped.
    uint32_t parent_pos = new_pos != 0 ? (new_pos-1)/2 : 0; // Holds the parent index of new_pos.
    uint32_t parent_node = minHeap[parent_pos];             // Holds the node being compared.

    // Only perform the swap if the element is not the root and the parent is
    // further away.
    if (new_pos != 0 && node_list[target].priority < node_list[parent_node].priority) {
        temp = target;
        minHeap[new_pos] = parent_node;
        node_list[parent_node].queue_pos = new_pos;
        minHeap[parent_pos] = temp;
        node_list[temp].queue_pos = parent_pos;
        upHeap(parent_pos);
    }
    
    return;
}

void downHeap(uint32_t root) {

    /*
     * Code is imported from Homework_05 coding assignment
     * Perform the downHeap operation for the priority heap
     */

    // Variable Declaration:
    uint32_t left, right, smallest; // Holds the position of the  left child,
                                    // right child, and the smallest value
    uint32_t temp;                  // Holds data during swaps.

    // Initialization:
    left = 2 * root + 1;
    right = 2 * root + 2;
    smallest = root;

    // Check 1: See if the closest element is farther than the left child.
    if (left <= heapSize && 
        node_list[minHeap[left]].priority < node_list[minHeap[smallest]].priority) {
        smallest = left;
    }

    // Check 2: See if the closest element is farther than the right child.
    if (right <= heapSize &&
        node_list[minHeap[right]].priority < node_list[minHeap[smallest]].priority) {
        smallest = right;
    }

    // Check 3: See if the closest element is the root element.
    // If not, then the elements must be swapped and a new downHeap is performed.
    if (smallest != root) {
        temp = minHeap[root];
        minHeap[root] = minHeap[smallest];
        node_list[minHeap[root]].queue_pos = root;
        minHeap[smallest] = temp;
        node_list[temp].queue_pos = smallest;
        downHeap(smallest);
    }

    return;
}

void insert(uint32_t node_label) {
    
    /*
     * Code is imported from Homework_05 coding assignment.
     * Performs the insert operation for the priority heap.
     */

    // Variable Declaration:
    uint32_t i; // Index of the element being added.
    
    // Check 1: If the heapSize is equal to maxnodes, then skip insert.
    if (heapSize == maxnodes) { return; }
    else {
        heapSize++;
        i = heapSize - 1;
        minHeap[i] = node_label;
        node_list[node_label].queue_pos = i;
        upHeap(i);
    }

    return;
}

uint32_t remove() {
    
    /*
     * Code is imported from Homework_05 coding assignment.
     * Performs the remove operation for the priority heap.
     */

    // Variable Declaration:
    uint32_t ret_node; // Holds the node to be returned from the heap

    // Case 1: If there is nothing to remove, return -1
    if (heapSize == 0) { return -1; }

    // Case 2: There is only one element in the heap
    else if (heapSize == 1) {
        ret_node = minHeap[0];
        heapSize--;
    }

    // Case 3: There are more elements that need to be adjusted
    else {
        ret_node = minHeap[0];
        minHeap[0] = minHeap[heapSize - 1];
        node_list[minHeap[0]].queue_pos = 0;
        heapSize--;
        downHeap(0);
    }
    
    return ret_node;
}

bool Relax_Dijkstra(uint32_t b_node, uint32_t e_node, uint32_t weight) {
    
    /*
     * Code is imported from Homework_05 coding assignment.
     * Performs the Relaxation step of Dijkstra algorithm
     */

    if (node_list[e_node].priority > node_list[b_node].priority + weight) {
        node_list[e_node].priority = node_list[b_node].priority + weight;
        node_list[e_node].previous = b_node;
        return true;
    }
    
    return false;
}

double Dijkstra(uint32_t start_node, uint32_t end_node,
                AdjacencyList adjacency_list) {
    
    /*
     * Performs the Dijkstra Pathfinding Algorithm and returns operation time.
     */

    // Variable Declaration:
    uint32_t target;                              // Holds the target node for initialization.
    std::vector<AdjacenyEntry*>::iterator cursor; // Holds the pointer to the removed node.
    clock_t start_time, end_time;                 // Used to time the algorithm.
    bool dead_end;                                // Used to indicate if a dead end was discoverd.

    // Begin the timer
    start_time = clock();

    // Step 1: Initialize the distance from the start node to 0, then create the 
    // heap structure.
    node_list[start_node].priority = 0;
    for (target = 0; target < maxnodes; target++) {
        insert(target);
    }

    // Step 2: Remove elements from the heap and relax the appropriate edges.
    while (heapSize != 0) {
        dead_end = true;
        target = remove();
        if (target == end_node) { break; }
        for(cursor = adjacency_list[target].begin(); cursor != adjacency_list[target].end();
            cursor++) {
            if (Relax_Dijkstra(target, (*cursor)->node_index, (*cursor)->edge_value)) {
                dead_end = false;
                upHeap(node_list[(*cursor)->node_index].queue_pos);
            } 
        }
        node_list[target].dead_end = dead_end;
    }
    
    end_time = clock();
    
    return (double)(end_time-start_time)/CLOCKS_PER_SEC;
}

bool Relax_Astar(uint32_t b_node, uint32_t e_node, uint32_t weight) {
    
    /*
     * Code is imported from Homework_05 coding assignment.
     * Performs the Relaxation step of Dijkstra algorithm
     */

    if (node_list[e_node].start_dis > node_list[b_node].start_dis + weight) {
        node_list[e_node].start_dis = node_list[b_node].start_dis + weight;
        node_list[e_node].priority = node_list[e_node].start_dis + node_list[e_node].end_dis;
        node_list[e_node].previous = b_node;
        return true;
    }
    
    return false;
}

double Astar(uint32_t start_node, uint32_t end_node,
                AdjacencyList adjacency_list) {
    
    /*
     * Performs the Dijkstra Pathfinding Algorithm and returns operation time.
     */

    // Variable Declaration:
    uint32_t target;                              // Holds the target node for initialization.
    std::vector<AdjacenyEntry*>::iterator cursor; // Holds the pointer to the removed node.
    clock_t start_time, end_time;                 // Used to time the algorithm.
    bool dead_end;                                // Used to indicate if a dead end was discoverd.

    // Begin the timer
    start_time = clock();

    // Step 1: Initialize the distance from the start node to 0, then create the 
    // heap structure.
    node_list[start_node].start_dis = 0;
    node_list[start_node].priority = node_list[start_node].start_dis + node_list[start_node].end_dis;
    for (target = 0; target < maxnodes; target++) {
        insert(target);
    }

    // Step 2: Remove elements from the heap and relax the appropriate edges.
    while (heapSize != 0) {
        dead_end = true;
        target = remove();
        if (target == end_node) { break; }
        for(cursor = adjacency_list[target].begin(); cursor != adjacency_list[target].end();
            cursor++) {
            if (Relax_Astar(target, (*cursor)->node_index, (*cursor)->edge_value)) {
                dead_end = false;
                upHeap(node_list[(*cursor)->node_index].queue_pos);
            } 
        }
        node_list[target].dead_end = dead_end;
    }
    
    end_time = clock();
    
    return (double)(end_time-start_time)/CLOCKS_PER_SEC;
}

bool Discover_GreedyBest(uint32_t b_node, uint32_t e_node) {
    
    /*
     * Performs the Discovery step of Greedy Best First Search algorithm
     */

    if (node_list[e_node].priority == UINT32_MAX ) {
        node_list[e_node].priority = node_list[e_node].end_dis;
        node_list[e_node].previous = b_node;
        return true;
    }
    
    return false;
}

double GreedyBest(uint32_t start_node, uint32_t end_node,
                  AdjacencyList adjacency_list) {
    
    /*
     * Performs the Dijkstra Pathfinding Algorithm and returns operation time.
     */

    // Variable Declaration:
    uint32_t target;                              // Holds the current target node for initialization.
    std::vector<AdjacenyEntry*>::iterator cursor; // Holds the pointer to the removed node.
    clock_t start_time, end_time;                 // Used to time the algorithm.
    bool dead_end;                                // Used to indicate if a dead end was discoverd.

    // Begin the timer
    start_time = clock();

    // Step 1: Initialize the distance from the start node to 0, then create the 
    // heap structure.
    node_list[start_node].priority = node_list[start_node].end_dis;
    for (target = 0; target < maxnodes; target++) {
        insert(target);
    }

    // Step 2: Remove elements from the heap and relax the appropriate edges.
    while (heapSize != 0) {
        dead_end = true;
        target = remove();
        if (target == end_node) { break; }
        for(cursor = adjacency_list[target].begin(); cursor != adjacency_list[target].end();
            cursor++) {
            if (Discover_GreedyBest(target, (*cursor)->node_index)) {
                dead_end = false;
                upHeap(node_list[(*cursor)->node_index].queue_pos);
            } 
        }
        node_list[target].dead_end = dead_end;
    }
    
    end_time = clock();
    
    return (double)(end_time-start_time)/CLOCKS_PER_SEC;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stdout, "usage: %s maze_svg_file\n", argv[0]);
        exit(0);
    }
    std::vector<Line*> line_list;
    FILE* xml_file = fopen(argv[1], "r");
    fseek(xml_file, 0, SEEK_END);
    size_t file_size = ftell(xml_file);
    fseek(xml_file, 0, 0);
    void* file_buffer = calloc(file_size, sizeof(char));
    int byte_count = fread(file_buffer, sizeof(char), file_size, xml_file);
    uint32_t line_count = 0;
    Line* new_line = 0;
    uint32_t x1 = 0;
    uint32_t x2 = 0;
    uint32_t y1 = 0;
    uint32_t y2 = 0;
    uint32_t max_x2 = 0;
    uint32_t max_y2 = 0;
    rapidxml::xml_document<> xml_doc;
    xml_doc.parse<0>((char*) file_buffer);
    rapidxml::xml_node<>* root_node = xml_doc.first_node();
    rapidxml::xml_node<>* g_node = root_node->first_node("g");
    rapidxml::xml_attribute<>* attr;
    fprintf(stdout, "Starting import of maze %s ---\n", argv[1]);
    for (rapidxml::xml_node<>* line_node = g_node->first_node("line");
            line_node; line_node = line_node->next_sibling("line")) {
        line_count += 1;
        if (VERBOSE) {
            fprintf(stdout, "line [%u]: ", line_count);
            for (rapidxml::xml_attribute<>* attr = line_node->first_attribute();
                    attr != NULL; attr = attr->next_attribute()) {
                fprintf(stdout, "%s : %s ", attr->name(), attr->value());
            }
            fprintf(stdout, "\n");
        }
        x1 = atoi(line_node->first_attribute("x1")->value());
        y1 = atoi(line_node->first_attribute("y1")->value());
        x2 = atoi(line_node->first_attribute("x2")->value());
        y2 = atoi(line_node->first_attribute("y2")->value());
        new_line = new Line(x1, y1, x2, y2);
        line_list.push_back(new_line);
        max_x2 = x2 > max_x2 ? x2 : max_x2;
        max_y2 = y2 > max_y2 ? y2 : max_y2;
    }
    fprintf(stdout, "--- Import of maze %s complete.\n\n", argv[1]);
    uint32_t row_count = (max_y2 - LINE_WIDTH) / CELL_HEIGHT;
    uint32_t col_count = (max_x2 - LINE_WIDTH) / CELL_WIDTH;
    // assert row_count == col_count
    Line* left_line = new Line(LINE_WIDTH, LINE_WIDTH, LINE_WIDTH,
            LINE_WIDTH + CELL_HEIGHT);
    Line* right_line = new Line(left_line->x1 + CELL_WIDTH, LINE_WIDTH,
            left_line->x1 + CELL_WIDTH, LINE_WIDTH + CELL_HEIGHT);
    Line* top_line = new Line(LINE_WIDTH, LINE_WIDTH, LINE_WIDTH + CELL_WIDTH,
            LINE_WIDTH);
    Line* bottom_line = new Line(LINE_WIDTH, top_line->y1 + CELL_HEIGHT,
            LINE_WIDTH + CELL_WIDTH, top_line->y1 + CELL_HEIGHT);
    uint32_t node_index = 0;
    MazeMatrix maze_matrix;

    // build matrix
    bool can_go_left = false;
    bool can_go_right = false;
    bool can_go_up = false;
    bool can_go_down = false;
    bool is_node = false;
    MazeCell* maze_cell = NULL;
    CellWindow* cell_window = new CellWindow(*left_line, *right_line, *top_line,
            *bottom_line);
    fprintf(stdout, "Starting matrix representation of maze ---\n");
    for (int row = 0; row < row_count; row++) {
        maze_matrix.push_back(std::vector<MazeCell*>());
        for (int col = 0; col < col_count; col++) {
            can_go_left = !line_segment_exists(cell_window->get_left_line(),
                    line_list);
            can_go_right = !line_segment_exists(cell_window->get_right_line(),
                    line_list);
            can_go_up = !line_segment_exists(cell_window->get_top_line(),
                    line_list);
            can_go_down = !line_segment_exists(cell_window->get_bottom_line(),
                    line_list);
            is_node = ((can_go_up && !can_go_down)
				|| (can_go_down && !can_go_up)
				|| (can_go_left && !can_go_right)
				|| (can_go_right && !can_go_left)
				|| (can_go_right & can_go_left & can_go_up & can_go_down))
            	|| (can_go_down == true && row == row_count - 1)
				|| (can_go_up == true && row == 0);
            maze_cell = new MazeCell(*cell_window, can_go_left, can_go_right,
                    can_go_up, can_go_down, is_node);
            if (is_node) {
                maze_cell->SetNodeIndex(node_index);
                node_index += 1;
            }
            if (VERBOSE) {
                fprintf(stdout,
                        "cell (%u, %u) left %s | up %s | right %s | down %s\n", row,
                        col, can_go_left ? "True" : "False",
                        can_go_up ? "True" : "False",
                        can_go_right ? "True" : "False",
                        can_go_down ? "True" : "False");
            }
            maze_matrix[row].push_back(maze_cell);
            cell_window->ShiftRight(1);
        }
        cell_window->ResetHorizontalPosition();
        cell_window->ShiftDown(1);
    }
    maxnodes = node_index;
    node_list = new struct Node[maxnodes];
    minHeap = new uint32_t[maxnodes];
    fprintf(stdout, "--- Matrix representation complete.\n\n");

    // build adjacency list
    fprintf(stdout, "Starting graph representation ---\n");
    uint32_t src_col = 0;
    AdjacencyList adjacency_list;
    MazeCell* src_cell = NULL;
    MazeCell* dest_cell = NULL;
    // horizontal sweep
    for (int row = 0; row < row_count; row++) {
        src_col = 0;
        src_cell = maze_matrix[row][0];
        for (int col = 0; col < col_count; col++) {
            dest_cell = maze_matrix[row][col];
            if (src_cell == dest_cell) {
                if (!dest_cell->CanGoRight()) {
                    src_col = col + 1;
                    if (src_col < row_count) {
                        src_cell = maze_matrix[row][src_col];
                    }
                } else {
                    continue;
                }
            } else if (dest_cell->CanGoLeft() && dest_cell->CanGoRight()
                    && !dest_cell->IsNode()) {
                continue;
            } else {
                adjacency_list[src_cell->GetNodeIndex()].push_back(
                        new AdjacenyEntry(dest_cell->GetNodeIndex(), row, col,
                                (col-src_col)));
                adjacency_list[dest_cell->GetNodeIndex()].push_back(
                        new AdjacenyEntry(src_cell->GetNodeIndex(), row,
                                src_col, (col-src_col)));
                if (dest_cell->CanGoRight()) {
                    src_cell = dest_cell;
                    src_col = col;
                } else {
                    src_col = col + 1;
                    if (src_col >= row_count) {
                        continue;
                    } else {
                        src_cell = maze_matrix[row][src_col];
                    }
                }
            }
        }
    }
    // vertical sweep
    uint32_t src_row = 0;
    src_cell = NULL;
    dest_cell = NULL;
    for (int col = 0; col < col_count; col++) {
        src_row = 0;
        src_cell = maze_matrix[0][col];
        for (int row = 0; row < row_count; row++) {
            dest_cell = maze_matrix[row][col];
            if (src_cell == dest_cell) {
                if (!dest_cell->CanGoDown()) {
                    src_row = row + 1;
                    if (src_row < row_count) {
                        src_cell = maze_matrix[src_row][col];
                    } else {
                        continue;
                    }
                }
            } else if (dest_cell->CanGoUp() && dest_cell->CanGoDown()
                    && !dest_cell->IsNode()) {
                continue;
            } else {
                adjacency_list[src_cell->GetNodeIndex()].push_back(
                        new AdjacenyEntry(dest_cell->GetNodeIndex(), row, col,
                                (row-src_row)));
                adjacency_list[dest_cell->GetNodeIndex()].push_back(
                        new AdjacenyEntry(src_cell->GetNodeIndex(), src_row,
                                col, (row-src_row)));
                if (dest_cell->CanGoDown()) {
                    src_cell = dest_cell;
                    src_row = row;
                } else {
                    src_row = row + 1;
                    if (src_row >= row_count) {
                        continue;
                    } else {
                        src_cell = maze_matrix[src_row][col];
                    }
                }
            }
        }
    }
    if (VERBOSE) {
        fprintf(stdout, "Maze Matrix to Graph Mapping\n");
        for (int row = 0; row < row_count; row++) {
            for (int col = 0; col < col_count; col++) {
                fprintf(stdout, "(%u, %u) is Node ", row, col);
                if (maze_matrix[row][col]->IsNode()) {
                    fprintf(stdout, "yes #%u\n",
                            maze_matrix[row][col]->GetNodeIndex());
                } else {
                    fprintf(stdout, "no\n");
                }
            }
        }
    
        fprintf(stdout, "\nAdjacency List\n");
        for (AdjacencyList::iterator u = adjacency_list.begin();
                u != adjacency_list.end(); u++) {
            fprintf(stdout, "%u: ", u->first);
            for (std::vector<AdjacenyEntry*>::iterator v = u->second.begin();
                    v != u->second.end(); v++) {
                fprintf(stdout, "--> %u(%u)", (*v)->node_index, (*v)->edge_value);
            }
            fprintf(stdout, "\n");
        }
    }
    fprintf(stdout, "--- Graph representation complete.\n\n");
    
    std::string output_path(argv[1]);
    size_t sub_pos = output_path.find(".svg");
    if (sub_pos != output_path.npos) {
        output_path = output_path.substr(0, sub_pos);
    }
    std::string answer_file[3] = { output_path + "_Dijkstra_solution.svg",
                                   output_path + "_GreedyBest_solution.svg",
                                   output_path + "_Astar_solution.svg" };
    MazeTraveler* mt = new MazeTraveler(maze_matrix, adjacency_list);
    // call these functions to get starts and end points
    uint32_t start_node = mt->GetStartNode();
    uint32_t end_node = mt->GetEndNode();
    fprintf(stdout, "Start node is %u | End node %u\n", start_node, end_node);
    NodeIndexToCellMap node_check = mt->GetNodeMap();
    Path solution_path;
    uint32_t path_ptr;
    uint32_t end_distance;
    double algorithm_time;
    for (int pathfinder = 0; pathfinder < 3; pathfinder++) {
        // Setup node_list & Reset Maze Traveler and heapSize;
        for (uint32_t iter = 0; iter < maxnodes; iter++) {
            end_distance = abs((int)(node_check[end_node].maze_row - node_check[iter].maze_row)) +
                           abs((int)(node_check[end_node].maze_col - node_check[iter].maze_col));
            node_list[iter].Setup(end_distance);
        }
        mt->ClearPath();
        heapSize = 0;
        // Perform pathfinding algorithm
        switch (pathfinder) {
            case 0: fprintf(stdout, "Starting Dijkstra Algorithm --- ");
                    algorithm_time = Dijkstra(start_node, end_node, adjacency_list);
                    fprintf(stdout, "completed. Time = %f seconds\n", algorithm_time);
                    break;
            case 1: fprintf(stdout, "Starting Greedy Best Algorithm --- ");
                    algorithm_time = GreedyBest(start_node, end_node, adjacency_list);
                    fprintf(stdout, "completed. Time = %f seconds\n", algorithm_time);
                    break;
            case 2: fprintf(stdout, "Starting A* Algorithm --- ");
                    algorithm_time = Astar(start_node, end_node, adjacency_list);
                    fprintf(stdout, "completed. Time = %f seconds\n", algorithm_time);
                    break;
        }
        // create path
        fprintf(stdout, "Saving path to %s --- \n", answer_file[pathfinder].c_str());
        for (uint32_t iter = 0; iter < maxnodes; iter++) {
            if (node_list[iter].dead_end && iter != end_node) {
                solution_path.clear();
                path_ptr = iter;
                solution_path.push_back(path_ptr);
                while (path_ptr != start_node) {
                    solution_path.push_back(node_list[path_ptr].previous);
                    path_ptr = node_list[path_ptr].previous;
                }
                std::reverse(solution_path.begin(), solution_path.end());
                // call this to draw the start line
                mt->StartTravel(start_node, COLOR_BLUE);
                // draw solution path*/
                mt->DrawPath(solution_path, COLOR_BLUE);
                write_solution_to_file(answer_file[pathfinder], mt->GetMazeMatrix(), line_list,
                        max_x2 + LINE_WIDTH, max_x2 + LINE_WIDTH);
            }
        }
        solution_path.clear();
        path_ptr = end_node;
        solution_path.push_back(path_ptr);
        while (path_ptr != start_node) {
            solution_path.push_back(node_list[path_ptr].previous);
            path_ptr = node_list[path_ptr].previous;
        }
        std::reverse(solution_path.begin(), solution_path.end());
        mt->StartTravel(start_node, COLOR_RED);
        mt->DrawPath(solution_path, COLOR_RED);
        mt->FinishTravel(COLOR_RED);
        write_solution_to_file(answer_file[pathfinder], mt->GetMazeMatrix(), line_list,
                max_x2 + LINE_WIDTH, max_x2 + LINE_WIDTH);
        fprintf(stdout, "--- Saving of path to %s completed.\n\n", answer_file[pathfinder].c_str());
    }
    return 0;
}
