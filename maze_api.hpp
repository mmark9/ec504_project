
#ifndef MAZE_API_HPP_
#define MAZE_API_HPP_

#include <map>
#include <vector>
#include <string>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <algorithm>
#include "logging.h"
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
			bool can_go_up, bool can_go_down, bool is_node) {
		_can_go_left = can_go_left;
		_can_go_right = can_go_right;
		_can_go_up = can_go_up;
		_can_go_down = can_go_down;
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
		_is_node = is_node;
	}

	MazeCell(const CellWindow& cell, bool can_go_left, bool can_go_right,
			bool can_go_up, bool can_go_down, bool is_node) {
		_can_go_left = can_go_left;
		_can_go_right = can_go_right;
		_can_go_up = can_go_up;
		_can_go_down = can_go_down;
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
		_is_node = is_node;
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

	void StartPath(uint32_t node_index, TravelDirection start_direction,
			const char* stroke_color) {
		// TODO: test this
		if (_index_to_cell_map.find(node_index) == _index_to_cell_map.end())
			return;
		uint32_t row = _index_to_cell_map[node_index].maze_row;
		uint32_t col = _index_to_cell_map[node_index].maze_col;
		if (row <= _maze_matrix.size() && col <= _maze_matrix.size()) {
			_origin_row = row;
			_origin_col = col;
			switch (start_direction) {
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
			_maze_matrix[_origin_row][_origin_col]->SetUpStrokeOn(true,
					stroke_color);
		} else if (_maze_matrix[_origin_row][_origin_col]->CanGoDown()) {
			_maze_matrix[_origin_row][_origin_col]->SetDownStrokeOn(true,
					stroke_color);
		}
	}

	void TravelToNode(uint32_t node_index, const char* stroke_color) {
		// first check if they are adjacent
		uint32_t origin_node_index =
				_maze_matrix[_origin_row][_origin_col]->GetNodeIndex();
		AdjacenyEntry adj_entry =\
 _adj_matrix[origin_node_index][node_index];
		if (adj_entry.is_valid) {
			if (VERBOSE) {
				fprintf(stdout, "Attempting to travel from %u to %u..\n",
					origin_node_index, node_index);
				fprintf(stdout, "\tOrigin was (%u, %u)\n", _origin_row,
					_origin_col);
			}
			if (_origin_row == adj_entry.maze_row) {
				if (_origin_col > adj_entry.maze_col) {
					// we are  moving to the left
					_maze_matrix[_origin_row][_origin_col]->SetLeftStrokeOn(
							true, stroke_color);
					for (int col = _origin_col - 1; col > adj_entry.maze_col;
							col--) {
						_maze_matrix[_origin_row][col]->SetLeftStrokeOn(true,
								stroke_color);
						_maze_matrix[_origin_row][col]->SetRightStrokeOn(true,
								stroke_color);
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
						_maze_matrix[_origin_row][col]->SetLeftStrokeOn(true,
								stroke_color);
						_maze_matrix[_origin_row][col]->SetRightStrokeOn(true,
								stroke_color);
					}
					_maze_matrix[_origin_row][adj_entry.maze_col]->SetLeftStrokeOn(
							true, stroke_color);
					_origin_col = adj_entry.maze_col;
					_origin_row = adj_entry.maze_row;
				}
			} else if (_origin_col == adj_entry.maze_col) {
				if (_origin_row > adj_entry.maze_row) {
					// we are going up
					_maze_matrix[_origin_row][_origin_col]->SetUpStrokeOn(true,
							stroke_color);
					for (int row = _origin_row - 1; row > adj_entry.maze_row;
							row--) {
						_maze_matrix[row][_origin_col]->SetUpStrokeOn(true,
								stroke_color);
						_maze_matrix[row][_origin_col]->SetDownStrokeOn(true,
								stroke_color);
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
						_maze_matrix[row][_origin_col]->SetUpStrokeOn(true,
								stroke_color);
						_maze_matrix[row][_origin_col]->SetDownStrokeOn(true,
								stroke_color);
					}
					_maze_matrix[adj_entry.maze_row][_origin_col]->SetUpStrokeOn(
							true, stroke_color);
					_origin_col = adj_entry.maze_col;
					_origin_row = adj_entry.maze_row;
				}
			}
			if (VERBOSE) {
				fprintf(stdout, "\tOrigin is now (%u, %u)\n", _origin_row,
					_origin_col);
			}
		} else {
			if (VERBOSE) {
				fprintf(stdout, "Cannot travel from node %u to %u; not connected\n",
					origin_node_index, node_index);
			}
		}
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
		for (Path::const_iterator node = path.begin(); node != path.end();
				node++) {
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
		_origin_col = 0;
		_origin_row = 0;
	}

	NodeIndexToCellMap GetNodeMap() {
		return _index_to_cell_map;
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



#endif /* MAZE_API_HPP_ */
