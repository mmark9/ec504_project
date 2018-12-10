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
#include "rapidxml.hpp"

const uint32_t LINE_WIDTH = 2;
const uint32_t CELL_WIDTH = 16;
const uint32_t CELL_HEIGHT = CELL_WIDTH;
const uint32_t CELL_MID_WIDTH = CELL_WIDTH / 2;
const uint32_t CELL_MID_HEIGHT = CELL_MID_WIDTH;

const char* COLOR_BLACK = "#000000";
const char* COLOR_RED = "#ff3300";
const char* COLOR_BLUE = "#0000ff";

const char* STROKE_LINE_CAP_BUTT = "butt";
const char* STROKE_LINE_CAP_ROUND = "round";
const char* STROKE_LINE_CAP_SQUARE = "square";

clock_t clock(void);

struct Line {
    Line() {
        x1 = 0;
        y1 = 0;
        x2 = 0;
        y2 = 0;
        x1_bak = 0;
        x2_bak = 0;
        y1_bak = 0;
        y2_bak = 0;
    }

    Line(uint32_t x_1, uint32_t y_1, uint32_t x_2, uint32_t y_2) {
        this->x1 = x_1;
        this->y1 = y_1;
        this->x2 = x_2;
        this->y2 = y_2;

        this->x1_bak = x_1;
        this->y1_bak = y_1;
        this->x2_bak = x_2;
        this->y2_bak = y_2;
    }

    void ResetXCoords() {
        x1 = x1_bak;
        x2 = x2_bak;
    }
    uint32_t x1;
    uint32_t y1;
    uint32_t x2;
    uint32_t y2;

    uint32_t x1_bak;
    uint32_t y1_bak;
    uint32_t x2_bak;
    uint32_t y2_bak;
};

class CellWindow {
public:
    CellWindow(const Line& left, const Line& right, const Line& top,
            const Line& bottom) {
        _left_line = left;
        _right_line = right;
        _top_line = top;
        _bottom_line = bottom;
        _cell_height = (bottom.y1 - top.y1);
        _cell_width = (top.x2 - top.x1);
    }

    void ShiftRight(uint32_t num_cells) {
        uint32_t pixel_offset = (_cell_width * num_cells);
        _top_line.x1 += pixel_offset;
        _top_line.x2 = _top_line.x1 + _cell_width;

        _bottom_line.x1 = _top_line.x1;
        _bottom_line.x2 = _top_line.x2;

        _left_line.x1 += pixel_offset;
        _left_line.x2 = _left_line.x1;

        _right_line.x1 += pixel_offset;
        _right_line.x2 = _right_line.x1;
    }

    void ShiftLeft(uint32_t num_cells) {
        uint32_t pixel_offset = (_cell_width * num_cells);
        _top_line.x1 -= pixel_offset;
        _top_line.x2 = _top_line.x1 + _cell_width;

        _bottom_line.x1 = _top_line.x1;
        _bottom_line.x2 = _top_line.x2;

        _left_line.x1 -= pixel_offset;
        _left_line.x2 = _left_line.x1;

        _right_line.x1 -= pixel_offset;
        _right_line.x2 = _left_line.x2;
    }

    void ShiftUp(uint32_t num_cells) {
        uint32_t pixel_offset = (_cell_width * num_cells);
        _left_line.y1 -= pixel_offset;
        _left_line.y2 = _left_line.y1 + _cell_height;

        _right_line.y1 = _left_line.y1;
        _right_line.y2 = _left_line.y2;

        _top_line.y1 -= pixel_offset;
        _top_line.y2 = _top_line.y1;

        _bottom_line.y1 -= pixel_offset;
        _bottom_line.y2 = _bottom_line.y1;
    }

    void ShiftDown(uint32_t num_cells) {
        uint32_t pixel_offset = (_cell_width * num_cells);
        _left_line.y1 += pixel_offset;
        _left_line.y2 = _left_line.y1 + _cell_height;

        _right_line.y1 = _left_line.y1;
        _right_line.y2 = _left_line.y2;

        _top_line.y1 += pixel_offset;
        _top_line.y2 = _top_line.y1;

        _bottom_line.y1 += pixel_offset;
        _bottom_line.y2 = _bottom_line.y1;
    }

    void ResetHorizontalPosition() {
        _left_line.ResetXCoords();
        _right_line.ResetXCoords();
        _top_line.ResetXCoords();
        _bottom_line.ResetXCoords();
    }

    Line get_left_line() const {
        return _left_line;
    }

    Line get_right_line() const {
        return _right_line;
    }

    Line get_top_line() const {
        return _top_line;
    }

    Line get_bottom_line() const {
        return _bottom_line;
    }

private:
    Line _left_line;
    Line _right_line;
    Line _top_line;
    Line _bottom_line;
    uint32_t _cell_width;
    uint32_t _cell_height;
};

class MazeCell {
public:
    MazeCell(const Line& left, const Line& right, const Line& top,
            const Line& bottom, bool can_go_left, bool can_go_right,
            bool can_go_up, bool can_go_down) {
        _can_go_left = can_go_left;
        _can_go_right = can_go_right;
        _can_go_up = can_go_up;
        _can_go_down = can_go_down;
        _is_node = (_can_go_up && !_can_go_down)
                || (_can_go_down && !_can_go_up)
                || (_can_go_left && !_can_go_right)
                || (_can_go_right && !_can_go_left)
                || (_can_go_right & _can_go_left & _can_go_up & _can_go_down);
        int mid_x = right.x1 - CELL_MID_WIDTH;
        int mid_y = bottom.y1 - CELL_MID_HEIGHT;
        _left_stroke = new Line(left.x1, mid_y, mid_x, mid_y);
        _right_stroke = new Line(mid_x, mid_y, mid_x + CELL_MID_WIDTH, mid_y);
        _up_stroke = new Line(mid_x, top.y1, mid_x, mid_y);
        _down_stroke = new Line(mid_x, mid_y, mid_x, bottom.y1);
        _left_stroke_on = false;
        _right_stroke_on = false;
        _up_stroke_on = false;
        _down_stroke_on = false;
        _node_index = 0;
        _left_stroke_color = COLOR_BLACK;
        _right_stroke_color = COLOR_BLACK;
        _up_stroke_color = COLOR_BLACK;
        _down_stroke_color = COLOR_BLACK;
    }

    MazeCell(const CellWindow& cell, bool can_go_left, bool can_go_right,
            bool can_go_up, bool can_go_down) {
        _can_go_left = can_go_left;
        _can_go_right = can_go_right;
        _can_go_up = can_go_up;
        _can_go_down = can_go_down;
        _is_node = (_can_go_up && !_can_go_down)
                || (_can_go_down && !_can_go_up)
                || (_can_go_left && !_can_go_right)
                || (_can_go_right && !_can_go_left)
                || (_can_go_right & _can_go_left & _can_go_up & _can_go_down);
        int mid_x = cell.get_right_line().x1 - CELL_MID_WIDTH;
        int mid_y = cell.get_bottom_line().y1 - CELL_MID_HEIGHT;
        _left_stroke = new Line(cell.get_left_line().x1, mid_y, mid_x, mid_y);
        _right_stroke = new Line(mid_x, mid_y, mid_x + CELL_MID_WIDTH, mid_y);
        _up_stroke = new Line(mid_x, cell.get_top_line().y1, mid_x, mid_y);
        _down_stroke = new Line(mid_x, mid_y, mid_x, cell.get_bottom_line().y1);
        _left_stroke_on = false;
        _right_stroke_on = false;
        _up_stroke_on = false;
        _down_stroke_on = false;
        _node_index = 0;
        _left_stroke_color = COLOR_BLACK;
        _right_stroke_color = COLOR_BLACK;
        _up_stroke_color = COLOR_BLACK;
        _down_stroke_color = COLOR_BLACK;
    }

    Line GetLeftStroke() const {
        return *_left_stroke;
    }

    Line GetRightStroke() const {
        return *_right_stroke;
    }

    Line GetUpStroke() const {
        return *_up_stroke;
    }

    Line GetDownStroke() const {
        return *_down_stroke;
    }

    bool CanGoRight() const {
        return _can_go_right;
    }

    bool CanGoLeft() const {
        return _can_go_left;
    }

    bool CanGoUp() const {
        return _can_go_up;
    }

    bool CanGoDown() const {
        return _can_go_down;
    }

    void ToggleUpStroke() {
        _up_stroke_on = !_up_stroke_on;
    }

    void ToggleDownStroke() {
        _down_stroke_on = !_down_stroke_on;
    }

    void ToggleLeftStroke() {
        _left_stroke_on = !_left_stroke;
    }

    void ToggleRightStroke() {
        _right_stroke_on = !_right_stroke_on;
    }

    void SetLeftStrokeOn(bool on, const char* color) {
        _left_stroke_on = on;
        _left_stroke_color = color;
    }

    void SetRightStrokeOn(bool on, const char* color) {
        _right_stroke_on = on;
        _right_stroke_color = color;
    }

    void SetUpStrokeOn(bool on, const char* color) {
        _up_stroke_on = on;
        _up_stroke_color = color;
    }

    void SetDownStrokeOn(bool on, const char* color) {
        _down_stroke_on = on;
        _down_stroke_color = color;
    }

    void ToggleHorizontalLineStroke() {
        ToggleLeftStroke();
        ToggleRightStroke();
    }

    void ToggleVerticalLineStroke() {
        ToggleUpStroke();
        ToggleDownStroke();
    }

    bool UpStrokeIsOn() const {
        return _up_stroke_on;
    }

    bool DownStrokeOn() const {
        return _down_stroke_on;
    }

    bool LeftStrokeOn() const {
        return _left_stroke_on;
    }

    bool RightStrokeOn() const {
        return _right_stroke_on;
    }

    void ClearStrokes() {
        _left_stroke_on = false;
        _right_stroke_on = false;
        _up_stroke_on = false;
        _down_stroke_on = false;
    }

    void ClearStrokeColors() {
        _left_stroke_color = COLOR_BLACK;
        _right_stroke_color = COLOR_BLACK;
        _up_stroke_color = COLOR_BLACK;
        _down_stroke_color = COLOR_BLACK;
    }

    bool IsNode() const {
        return _is_node;
    }

    void SetNodeIndex(uint32_t index) {
        _node_index = index;
    }

    uint32_t GetNodeIndex() const {
        return _node_index;
    }

    const char* GetLeftStrokeColor() const {
        return _left_stroke_color;
    }

    const char* GetRightStrokeColor() const {
        return _right_stroke_color;
    }

    const char* GetUpStrokeColor() const {
        return _up_stroke_color;
    }

    const char* GetDownStrokeColor() const {
        return _down_stroke_color;
    }

private:
    bool _can_go_left;
    bool _can_go_right;
    bool _can_go_up;
    bool _can_go_down;
    bool _is_node;
    Line* _left_stroke;
    Line* _right_stroke;
    Line* _up_stroke;
    Line* _down_stroke;
    bool _left_stroke_on;
    bool _right_stroke_on;
    bool _up_stroke_on;
    bool _down_stroke_on;
    uint32_t _node_index;
    const char* _left_stroke_color;
    const char* _right_stroke_color;
    const char* _up_stroke_color;
    const char* _down_stroke_color;
};

struct AdjacenyEntry {

    AdjacenyEntry() {
        node_index = 0;
        maze_row = 0;
        maze_col = 0;
        edge_value = 0;
        is_valid = false;
    }

    AdjacenyEntry(uint32_t index, uint32_t row, uint32_t col, uint32_t value) {
        node_index = index;
        maze_row = row;
        maze_col = col;
        edge_value = value;
        is_valid = false;
    }

    uint32_t node_index;
    uint32_t maze_row;
    uint32_t maze_col;
    uint32_t edge_value;
    bool is_valid;
};

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

typedef std::map<uint32_t, AdjacenyEntry> NodeIndexToCellMap;
typedef std::vector<std::vector<MazeCell*> > MazeMatrix;
typedef std::map<uint32_t, std::vector<AdjacenyEntry*> > AdjacencyList;
typedef std::vector<std::vector<bool> > AdjacencyMatrix;
typedef std::vector<std::vector<AdjacenyEntry> > AdjacencyTravelMatrix;
typedef std::vector<uint32_t> Path;

enum TravelDirection {
    UP = 0, DOWN = 1, LEFT = 2, RIGHT = 3
};

class MazeTraveler {
public:
    MazeTraveler(MazeMatrix matrix, const AdjacencyList& adj_list) {
        _row_count = matrix.size() * matrix.size();
        _col_count = _row_count;
        _adj_matrix.resize(_row_count);
        for (int c = 0; c < _row_count; c++) {
            _adj_matrix[c].resize(_row_count);
        }
        for (AdjacencyList::const_iterator u = adj_list.begin();
                u != adj_list.end(); u++) {
            for (std::vector<AdjacenyEntry*>::const_iterator v =
                    (*u).second.begin(); v != (*u).second.end(); v++) {
                _adj_matrix[(*u).first][(*v)->node_index] = *(*v);
                _adj_matrix[(*u).first][(*v)->node_index].is_valid = true;
                _index_to_cell_map[(*v)->node_index] = *(*v);
            }
        }
        _origin_row = 0;
        _origin_col = 0;
        _maze_matrix = matrix;
    }

    void StartTravel(uint32_t node_index, const char* stroke_color) {
        if (_index_to_cell_map.find(node_index) == _index_to_cell_map.end())
            return;
        uint32_t row = _index_to_cell_map[node_index].maze_row;
        uint32_t col = _index_to_cell_map[node_index].maze_col;
        if (row <= _maze_matrix.size() && col <= _maze_matrix.size()) {
            _origin_row = row;
            _origin_col = col;
            _maze_matrix[row][col]->ClearStrokes();
            // here we assume entries and exits are always at top or bottom
            if (_maze_matrix[row][col]->CanGoUp()) {
                _maze_matrix[row][col]->SetUpStrokeOn(true, stroke_color);
            } else if (_maze_matrix[row][col]->CanGoDown()) {
                _maze_matrix[row][col]->SetDownStrokeOn(true, stroke_color);
            }
        }
    }

    void ResetOrigin(uint32_t node_index, const char* stroke_color) {
        if (_index_to_cell_map.find(node_index) == _index_to_cell_map.end())
            return;
        uint32_t row = _index_to_cell_map[node_index].maze_row;
        uint32_t col = _index_to_cell_map[node_index].maze_col;
        if (row <= _maze_matrix.size() && col <= _maze_matrix.size()) {
            _origin_row = row;
            _origin_col = col;
        }
    }

    void StartPath(uint32_t node_index, TravelDirection start_direction, const char* stroke_color) {
        // TODO: test this
        if (_index_to_cell_map.find(node_index) == _index_to_cell_map.end())
            return;
        uint32_t row = _index_to_cell_map[node_index].maze_row;
        uint32_t col = _index_to_cell_map[node_index].maze_col;
        if (row <= _maze_matrix.size() && col <= _maze_matrix.size()) {
            _origin_row = row;
            _origin_col = col;
            switch(start_direction) {
            case LEFT:
                _maze_matrix[row][col]->SetLeftStrokeOn(true, stroke_color);
                break;
            case RIGHT:
                _maze_matrix[row][col]->SetRightStrokeOn(true, stroke_color);
                break;
            case UP:
                _maze_matrix[row][col]->SetUpStrokeOn(true, stroke_color);
                break;
            case DOWN:
                _maze_matrix[row][col]->SetDownStrokeOn(true, stroke_color);
                break;
            }
        }
    }

    void FinishTravel(const char* stroke_color) {
        // here we assume entries and exits are always at top or bottom
        if (_maze_matrix[_origin_row][_origin_col]->CanGoUp()) {
            _maze_matrix[_origin_row][_origin_col]->SetUpStrokeOn(true, stroke_color);
        } else if (_maze_matrix[_origin_row][_origin_col]->CanGoDown()) {
            _maze_matrix[_origin_row][_origin_col]->SetDownStrokeOn(true, stroke_color);
        }
    }

    void TravelToNode(uint32_t node_index, const char* stroke_color) {
        // first check if they are adjacent
        uint32_t origin_node_index =
                _maze_matrix[_origin_row][_origin_col]->GetNodeIndex();
        AdjacenyEntry adj_entry =\
 _adj_matrix[origin_node_index][node_index];
        fprintf(stdout, "Attempting to travel from %u to %u..\n",
                origin_node_index, node_index);
        fprintf(stdout, "\tOrigin was (%u, %u)\n", _origin_row, _origin_col);
        if (adj_entry.is_valid) {
            if (_origin_row == adj_entry.maze_row) {
                if (_origin_col > adj_entry.maze_col) {
                    // we are  moving to the left
                    _maze_matrix[_origin_row][_origin_col]->SetLeftStrokeOn(
                            true, stroke_color);
                    for (int col = _origin_col - 1; col > adj_entry.maze_col;
                            col--) {
                        _maze_matrix[_origin_row][col]->SetLeftStrokeOn(true, stroke_color);
                        _maze_matrix[_origin_row][col]->SetRightStrokeOn(true, stroke_color);
                    }
                    _maze_matrix[_origin_row][adj_entry.maze_col]->SetRightStrokeOn(
                            true, stroke_color);
                    _origin_col = adj_entry.maze_col;
                    _origin_row = adj_entry.maze_row;

                } else if (_origin_col < adj_entry.maze_col) {
                    // we are going to the right
                    _maze_matrix[_origin_row][_origin_col]->SetRightStrokeOn(
                            true, stroke_color);
                    for (int col = _origin_col + 1; col < adj_entry.maze_col;
                            col++) {
                        _maze_matrix[_origin_row][col]->SetLeftStrokeOn(true, stroke_color);
                        _maze_matrix[_origin_row][col]->SetRightStrokeOn(true, stroke_color);
                    }
                    _maze_matrix[_origin_row][adj_entry.maze_col]->SetLeftStrokeOn(
                            true, stroke_color);
                    _origin_col = adj_entry.maze_col;
                    _origin_row = adj_entry.maze_row;
                }
            } else if (_origin_col == adj_entry.maze_col) {
                if (_origin_row > adj_entry.maze_row) {
                    // we are going up
                    _maze_matrix[_origin_row][_origin_col]->SetUpStrokeOn(true, stroke_color);
                    for (int row = _origin_row - 1; row > adj_entry.maze_row;
                            row--) {
                        _maze_matrix[row][_origin_col]->SetUpStrokeOn(true, stroke_color);
                        _maze_matrix[row][_origin_col]->SetDownStrokeOn(true, stroke_color);
                    }
                    _maze_matrix[adj_entry.maze_row][_origin_col]->SetDownStrokeOn(
                            true, stroke_color);
                    _origin_col = adj_entry.maze_col;
                    _origin_row = adj_entry.maze_row;
                } else if (_origin_row < adj_entry.maze_row) {
                    // we are going down
                    _maze_matrix[_origin_row][_origin_col]->SetDownStrokeOn(
                            true, stroke_color);
                    for (int row = _origin_row + 1; row < adj_entry.maze_row;
                            row++) {
                        _maze_matrix[row][_origin_col]->SetUpStrokeOn(true, stroke_color);
                        _maze_matrix[row][_origin_col]->SetDownStrokeOn(true, stroke_color);
                    }
                    _maze_matrix[adj_entry.maze_row][_origin_col]->SetUpStrokeOn(
                            true, stroke_color);
                    _origin_col = adj_entry.maze_col;
                    _origin_row = adj_entry.maze_row;
                }
            }
        }
        fprintf(stdout, "\tOrigin is now (%u, %u)\n", _origin_row, _origin_col);
    }

    // TODO: Make these functions more robust
    uint32_t GetStartNode() const {
        for (uint32_t col = 0; col < _maze_matrix.size(); col++) {
            if (_maze_matrix[0][col]->CanGoUp()) {
                return _maze_matrix[0][col]->GetNodeIndex();
            }
        }
        return 0;
    }

    uint32_t GetEndNode() const {
        uint32_t row = _maze_matrix.size() - 1;
        for (uint32_t col = 0; col < _maze_matrix.size(); col++) {
            if (_maze_matrix[row][col]->CanGoDown()) {
                return _maze_matrix[row][col]->GetNodeIndex();
            }
        }
        return 0;
    }

    void DrawPath(const Path& path, const char* stroke_color) {
        for (Path::const_iterator node = path.begin(); node != path.end(); node++) {
            this->TravelToNode(*node, stroke_color);
        }
    }

    void ClearPath() {
        for (uint32_t row = 0; row < _maze_matrix.size(); row++) {
            for (uint32_t col = 0; col < _maze_matrix.size(); col++) {
                _maze_matrix[row][col]->ClearStrokes();
                _maze_matrix[row][col]->ClearStrokeColors();
            }
        }
    }

    uint32_t GetCurrentOriginRow() {
        return _origin_row;
    }

    uint32_t GetCurrentOriginColumn() {
        return _origin_col;
    }

    uint32_t GetCurrentNodeIndex() {
        return _maze_matrix[_origin_row][_origin_col]->GetNodeIndex();
    }

    MazeMatrix GetMazeMatrix() {
        return _maze_matrix;
    }

    NodeIndexToCellMap GetNodeMap() {
        return _index_to_cell_map;
    }

private:
    MazeMatrix _maze_matrix;
    AdjacencyList _adj_list;
    uint32_t _origin_row;
    uint32_t _origin_col;
    AdjacencyTravelMatrix _adj_matrix;
    uint32_t _row_count;
    uint32_t _col_count;
    NodeIndexToCellMap _index_to_cell_map;
};

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
                        temp_line.x2, temp_line.y2, maze_cell->GetLeftStrokeColor(), LINE_WIDTH,
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
                        temp_line.x2, temp_line.y2, maze_cell->GetUpStrokeColor(), LINE_WIDTH,
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
                        temp_line.x2, temp_line.y2, maze_cell->GetRightStrokeColor(), LINE_WIDTH,
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
                        temp_line.x2, temp_line.y2, maze_cell->GetDownStrokeColor(), LINE_WIDTH,
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
    for (rapidxml::xml_node<>* line_node = g_node->first_node("line");
            line_node; line_node = line_node->next_sibling("line")) {
        line_count += 1;
        fprintf(stdout, "line [%u]: ", line_count);
        for (rapidxml::xml_attribute<>* attr = line_node->first_attribute();
                attr != NULL; attr = attr->next_attribute()) {
            fprintf(stdout, "%s : %s ", attr->name(), attr->value());
        }
        fprintf(stdout, "\n");
        x1 = atoi(line_node->first_attribute("x1")->value());
        y1 = atoi(line_node->first_attribute("y1")->value());
        x2 = atoi(line_node->first_attribute("x2")->value());
        y2 = atoi(line_node->first_attribute("y2")->value());
        new_line = new Line(x1, y1, x2, y2);
        line_list.push_back(new_line);
        max_x2 = x2 > max_x2 ? x2 : max_x2;
        max_y2 = y2 > max_y2 ? y2 : max_y2;
    }
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
    MazeCell* maze_cell = NULL;
    CellWindow* cell_window = new CellWindow(*left_line, *right_line, *top_line,
            *bottom_line);
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
            maze_cell = new MazeCell(*cell_window, can_go_left, can_go_right,
                    can_go_up, can_go_down);
            if (maze_cell->IsNode()) {
                maze_cell->SetNodeIndex(node_index);
                node_index += 1;
            }
            fprintf(stdout,
                    "cell (%u, %u) left %s | up %s | right %s | down %s\n", row,
                    col, can_go_left ? "True" : "False",
                    can_go_up ? "True" : "False",
                    can_go_right ? "True" : "False",
                    can_go_down ? "True" : "False");
            maze_matrix[row].push_back(maze_cell);
            cell_window->ShiftRight(1);
        }
        cell_window->ResetHorizontalPosition();
        cell_window->ShiftDown(1);
    }
    maxnodes = node_index;
    node_list = new struct Node[maxnodes];
    minHeap = new uint32_t[maxnodes];

    // build adjacency list
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
    fprintf(stdout, "\nMaze Matrix to Graph Mapping\n");
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
            fprintf(stdout, "Node Number %u | Priority = %u | Start_Dis = %u | End_Dis = %u | Previous = %u | Queue_Pos = %u | Dead_End = %s\n", iter, node_list[iter].priority, node_list[iter].start_dis, node_list[iter].end_dis, node_list[iter].previous, node_list[iter].queue_pos, node_list[iter].dead_end ? "True" : "False");
        }
        mt->ClearPath();
        heapSize = 0;
        // Perform pathfinding algorithm
        switch (pathfinder) {
            case 0: fprintf(stdout, "Starting Dijkstra Algorithm...");
                    algorithm_time = Dijkstra(start_node, end_node, adjacency_list);
                    fprintf(stdout, "Done.\nDijkstra Time = %f seconds\n", algorithm_time);
                    break;
            case 1: fprintf(stdout, "Starting Greedy Best Algorithm...");
                    algorithm_time = GreedyBest(start_node, end_node, adjacency_list);
                    fprintf(stdout, "Done.\nGreedy Best Time = %f seconds\n", algorithm_time);
                    break;
            case 2: fprintf(stdout, "Starting A* Algorithm...");
                    algorithm_time = Astar(start_node, end_node, adjacency_list);
                    fprintf(stdout, "Done.\nA* Time = %f seconds\n", algorithm_time);
                    break;
        }
        // create path
        for (uint32_t iter = 0; iter < maxnodes; iter++) {
            fprintf(stdout, "Node Number %u | Priority = %u | Start_Dis = %u | End_Dis = %u | Previous = %u | Queue_Pos = %u | Dead_End = %s\n", iter, node_list[iter].priority, node_list[iter].start_dis, node_list[iter].end_dis, node_list[iter].previous, node_list[iter].queue_pos, node_list[iter].dead_end ? "True" : "False");
        }
        fprintf(stdout, "Saving path to %s --- ", answer_file[pathfinder].c_str());
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
        fprintf(stdout, "Done.\n\n");
    }
    return 0;
}
