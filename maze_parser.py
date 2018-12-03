import os
import sys
import math
import argparse
from enum import Enum
from collections import defaultdict
import xml.etree.ElementTree as ET


SVG_LINE_FMT = (
    '<line '
    'x1="{x1}" '
    'y1="{y1}" '
    'x2="{x2}" '
    'y2="{y2}" '
    'fill="none" '
    'stroke="{stroke}" '
    'stroke-width="2" '
    'stroke-linecap="{shape}"'
    '/>'
)

SVG_BODY_FMT = '''<?xml version="1.0" standalone="no"?>
<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN" "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">
<svg width="{svg_width}" height="{svg_height}" version="1.1" xmlns="http://www.w3.org/2000/svg">
  <title>{title}</title>
  <desc>{description}</desc>
  {lines}</svg>
'''


LINE_WIDTH = 2
CELL_WIDTH = 16
CELL_HEIGHT = 16
CELL_MID_HEIGHT = int(CELL_HEIGHT / 2)
CELL_MID_WIDTH = int(CELL_WIDTH / 2)


class LineColor(Enum):
    BLACK = '#000000'
    RED = '#ff3300'


class LineBorderShape(Enum):
    BUTT = 'butt'
    ROUND = 'round'
    SQUARE = 'square'


class Line(object):
    def __init__(self, x1, y1, x2, y2):
        self.x1 = x1
        self.x2 = x2
        self.y1 = y1
        self.y2 = y2

        self.x1_bak = x1
        self.x2_bak = x2

    def reset_x_coords(self):
        self.x1 = self.x1_bak
        self.x2 = self.x2_bak


class MazeCell(object):
    def __init__(self, left_line, right_line, top_line, bottom_line,
                 can_go_left=False,
                 can_go_right=False, can_go_up=False,
                 can_go_down=False,):
        self.can_go_left = can_go_left
        self.can_go_right = can_go_right
        self.can_go_up = can_go_up
        self.can_go_down = can_go_down
        self.is_node = False
        self.marked = False
        self.node_index = 0
        mid_x = int(right_line.x1 - CELL_MID_WIDTH)
        mid_y = int(bottom_line.y1 - CELL_MID_HEIGHT)
        self.left_line = Line(
            left_line.x1,
            mid_y,
            mid_x,
            mid_y
        )
        self.right_line = Line(
            mid_x,
            mid_y,
            mid_x + CELL_MID_WIDTH,
            mid_y
        )
        self.vertical_line_top = Line(
            mid_x,
            top_line.y1,
            mid_x,
            mid_y
        )
        self.vertical_line_bottom = Line(
            mid_x,
            mid_y,
            mid_x,
            bottom_line.y1
        )
        self.left_line_active = False
        self.right_line_active = False
        self.vertical_line_top_active = False
        self.vertical_line_bottom_active = False

    def stroke_up_toggle(self):
        self.vertical_line_top_active = not self.vertical_line_top_active

    def stroke_down_toggle(self):
        self.vertical_line_bottom_active = not self.vertical_line_bottom_active

    def stroke_left_toggle(self):
        self.left_line_active = not self.left_line_active

    def stroke_right_toggle(self):
        self.right_line_active = not self.right_line_active

    def update_isNode(self):
        self.is_node = ((self.can_go_up and not self.can_go_down) or
                        (self.can_go_down and not self.can_go_up) or
                        (self.can_go_left and not self.can_go_right) or
                        (self.can_go_right and not self.can_go_left))

    def stroke_vertical_toggle(self):
        self.stroke_up_toggle()
        self.stroke_down_toggle()

    def stroke_horizontal_toggle(self):
        self.stroke_left_toggle()
        self.stroke_right_toggle()


def get_2d_index_from_node_number(i, matrix_size):
    col = (i - 1) % matrix_size
    row = int(math.floor((i - 1) / matrix_size))
    return row, col


# assumes zero index base
def get_1d_index(i, j, matrix_size):
    return (matrix_size * i) + j + 1


def line_segment_exists(line_seg, line_list):
    for line in line_list:
        if line_seg.x1 >= line.x1 and line_seg.x2 <= line.x2 \
                and line_seg.y1 >= line.y1 and line_seg.y2 <= line.y2:
            return True
    return False


def write_svg_to_file(file_path, maze_matrix, line_list, img_width, img_height):
    line_list_str = ''
    with open(file_path, mode='w') as out_fh:
        for line in line_list:
            line_list_str += '\t'
            line_list_str += SVG_LINE_FMT.format(
                x1=line.x1,
                x2=line.x2,
                y1=line.y1,
                y2=line.y2,
                stroke=LineColor.BLACK.value,
                shape=LineBorderShape.SQUARE.value
            )
            line_list_str += '\n'
        for row in range(0, len(maze_matrix.keys())):
            for col in range(0, len(maze_matrix.keys())):
                cell = maze_matrix[row][col]
                if cell.left_line_active:
                    line_list_str += '\t'
                    line_list_str += SVG_LINE_FMT.format(
                        x1=cell.left_line.x1,
                        x2=cell.left_line.x2,
                        y1=cell.left_line.y1,
                        y2=cell.left_line.y2,
                        stroke=LineColor.RED.value,
                        shape=LineBorderShape.ROUND.value
                    )
                    line_list_str += '\n'
                if cell.vertical_line_top_active:
                    line_list_str += '\t'
                    line_list_str += SVG_LINE_FMT.format(
                        x1=cell.vertical_line_top.x1,
                        x2=cell.vertical_line_top.x2,
                        y1=cell.vertical_line_top.y1,
                        y2=cell.vertical_line_top.y2,
                        stroke=LineColor.RED.value,
                        shape=LineBorderShape.ROUND.value
                    )
                    line_list_str += '\n'
                if cell.right_line_active:
                    line_list_str += '\t'
                    line_list_str += SVG_LINE_FMT.format(
                        x1=cell.right_line.x1,
                        x2=cell.right_line.x2,
                        y1=cell.right_line.y1,
                        y2=cell.right_line.y2,
                        stroke=LineColor.RED.value,
                        shape=LineBorderShape.ROUND.value
                    )
                    line_list_str += '\n'
                if cell.vertical_line_bottom_active:
                    line_list_str += '\t'
                    line_list_str += SVG_LINE_FMT.format(
                        x1=cell.vertical_line_bottom.x1,
                        x2=cell.vertical_line_bottom.x2,
                        y1=cell.vertical_line_bottom.y1,
                        y2=cell.vertical_line_bottom.y2,
                        stroke=LineColor.RED.value,
                        shape=LineBorderShape.ROUND.value
                    )
                    line_list_str += '\n'
        svg_content = SVG_BODY_FMT.format(
            lines=line_list_str,
            title='SVG Matrix',
            description='Generated Matrix',
            svg_width=img_width,
            svg_height=img_height
        )
        out_fh.write(svg_content)


def main(cmd_args):
    tree = ET.parse(cmd_args['svg'])
    root = tree.getroot()
    g_obj = root.find('{http://www.w3.org/2000/svg}g')
    line_list = []
    max_x2 = LINE_WIDTH
    max_y2 = LINE_WIDTH
    for line in g_obj:
        line_list.append(
            Line(
                int(line.attrib['x1']),
                int(line.attrib['y1']),
                int(line.attrib['x2']),
                int(line.attrib['y2'])
            )
        )
        max_x2 = max(int(line.attrib['x2']), max_x2)
        max_y2 = max(int(line.attrib['y2']), max_y2)
    maze_rows = int((max_y2 - LINE_WIDTH) / CELL_HEIGHT)
    maze_cols = int((max_x2 - LINE_WIDTH) / CELL_WIDTH)
    # assert maze_rows == maze_cols
    left_line = Line(2, 2, 2, 2 + CELL_HEIGHT)
    right_line = Line(
        left_line.x1 + CELL_WIDTH,
        2,
        left_line.x1 + CELL_WIDTH,
        2 + CELL_HEIGHT
    )
    top_line = Line(2, 2, 2 + CELL_WIDTH, 2)
    bottom_line = Line(2,
                       top_line.y1 + CELL_HEIGHT,
                       2 + CELL_WIDTH,
                       top_line.y1 + CELL_HEIGHT)
    node_index = 0
    maze_matrix = defaultdict(lambda: [])
    for row in range(0, maze_rows):
        for col in range(0, maze_cols):
            maze_cell = MazeCell(
                left_line,
                right_line,
                top_line,
                bottom_line
            )
            maze_cell.can_go_left = not line_segment_exists(left_line, line_list)
            maze_cell.can_go_up = not line_segment_exists(top_line, line_list)
            maze_cell.can_go_right = not line_segment_exists(right_line, line_list)
            maze_cell.can_go_down = not line_segment_exists(bottom_line, line_list)
            maze_cell.update_isNode()
            if maze_cell.is_node:
                maze_cell.node_index = node_index
                node_index += 1
            print(
                'cell ({},{}) left: {} | '
                'up: {} | right: {} | down: {}'.format(
                    row + 1,
                    col + 1,
                    maze_cell.can_go_left,
                    maze_cell.can_go_up,
                    maze_cell.can_go_right,
                    maze_cell.can_go_down
                )
            )
            maze_matrix[row].append(maze_cell)
            left_line.x1 += CELL_WIDTH
            left_line.x2 = left_line.x1
            top_line.x1 = left_line.x1
            top_line.x2 = top_line.x1 + CELL_WIDTH
            right_line.x1 += CELL_WIDTH
            right_line.x2 = right_line.x1
            bottom_line.x1 = top_line.x1
            bottom_line.x2 = top_line.x2
        left_line.reset_x_coords()
        top_line.reset_x_coords()
        right_line.reset_x_coords()
        bottom_line.reset_x_coords()
        left_line.y1 = left_line.y2
        left_line.y2 += CELL_HEIGHT
        top_line.y1 = left_line.y1
        top_line.y2 = top_line.y1
        right_line.y1 = left_line.y1
        right_line.y2 = left_line.y2
        bottom_line.y1 += CELL_HEIGHT
        bottom_line.y2 = bottom_line.y1
    adjacency_list = defaultdict(lambda: [])
    # horizontal sweep
    for row in range(0, maze_rows):
        num_hops = 0
        src_col = 0
        src_cell = maze_matrix[row][0]
        for col in range(0, maze_cols):
            dest_cell = maze_matrix[row][col]
            if src_cell == dest_cell:
                if not dest_cell.can_go_right:
                    src_col = col + 1
                    if src_col < maze_cols:
                        src_cell = maze_matrix[row][src_col]
                        num_hops = 0
                else:
                    num_hops += 1
            elif dest_cell.can_go_left and \
                    dest_cell.can_go_right and not dest_cell.is_node:
                num_hops += 1
            else:
                adjacency_list[src_cell.node_index].append(
                    (dest_cell.node_index, (row, col), num_hops)
                )
                adjacency_list[dest_cell.node_index].append(
                    (src_cell.node_index, (row, src_col), num_hops)
                )
                num_hops = 0
                if dest_cell.can_go_right:
                    src_cell = dest_cell
                    src_col = col
                    num_hops += 1
                else:
                    src_col = col + 1
                    if src_col >= maze_cols:
                        continue
                    src_cell = maze_matrix[row][src_col]
    # vertical sweep
    for col in range(0, maze_cols):
        num_hops = 0
        src_row = 0
        src_cell = maze_matrix[0][col]
        for row in range(0, maze_rows):
            dest_cell = maze_matrix[row][col]
            if src_cell == dest_cell:
                if not dest_cell.can_go_down:
                    src_row = row + 1
                    if src_row < maze_rows:
                        src_cell = maze_matrix[src_row][col]
                        num_hops = 0
                else:
                    num_hops += 1
            elif dest_cell.can_go_up and dest_cell.can_go_down \
                    and not dest_cell.is_node:
                num_hops += 1
            else:
                adjacency_list[src_cell.node_index].append(
                    (dest_cell.node_index, (row, col), num_hops)
                )
                adjacency_list[dest_cell.node_index].append(
                    (src_cell.node_index, (src_row, col), num_hops)
                )
                num_hops = 0
                if dest_cell.can_go_down:
                    src_cell = dest_cell
                    src_row = row
                    num_hops += 1
                else:
                    src_row = row + 1
                    if src_row >= maze_rows:
                        continue
                    src_cell = maze_matrix[src_row][col]
    print('\nMaze Matrix to Graph mapping')
    for row in range(0, maze_rows):
        for col in range(0, maze_cols):
            print(
                '({},{}) is Node {}'.format(
                    row, col,
                    'yes #{}'.format(maze_matrix[row][col].node_index)
                    if maze_matrix[row][col].is_node else 'No'
                )
            )
    print('\nAdjacency list:')
    for u in range(0, len(adjacency_list.keys())):
        sys.stdout.write('{}: '.format(u))
        for v in adjacency_list[u]:
            sys.stdout.write(
                '--> {}'.format(v[0])
            )
        sys.stdout.write('\n')
    # Example of drawing solution for 5x5 maze
    '''
    maze_matrix[0][2].stroke_up_toggle()
    maze_matrix[0][2].stroke_left_toggle()
    maze_matrix[0][1].stroke_horizontal_toggle()
    maze_matrix[0][0].stroke_right_toggle()
    maze_matrix[0][0].stroke_down_toggle()
    maze_matrix[1][0].stroke_up_toggle()
    maze_matrix[1][0].stroke_right_toggle()
    maze_matrix[1][1].stroke_horizontal_toggle()
    maze_matrix[1][2].stroke_left_toggle()
    maze_matrix[1][2].stroke_down_toggle()
    maze_matrix[2][2].stroke_up_toggle()
    maze_matrix[2][2].stroke_right_toggle()
    maze_matrix[2][3].stroke_horizontal_toggle()
    maze_matrix[2][4].stroke_left_toggle()
    maze_matrix[2][4].stroke_down_toggle()
    maze_matrix[3][4].stroke_vertical_toggle()
    maze_matrix[4][4].stroke_up_toggle()
    maze_matrix[4][4].stroke_left_toggle()
    maze_matrix[4][3].stroke_horizontal_toggle()
    maze_matrix[4][2].stroke_right_toggle()
    maze_matrix[4][2].stroke_down_toggle()
    '''
    solution_path = cmd_args['svg'].replace('.svg', '') + '_solution.svg'
    print('Writing solution to {}..'.format(solution_path))
    write_svg_to_file(
        solution_path,
        maze_matrix,
        line_list,
        max_x2 + LINE_WIDTH,
        max_x2 + LINE_WIDTH
    )


if __name__ == '__main__':
    arg_parser = argparse.ArgumentParser(
        description='Parses SVG maze into adjacency list'
    )
    arg_parser.add_argument(
        'svg',
        help='Path to SVG file'
    )
    args = vars(arg_parser.parse_args())
    main(args)
