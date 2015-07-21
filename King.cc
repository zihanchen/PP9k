#include "concrete_pieces.h"
#include <iostream>
#include <cmath>

bool King::moveCheck(int row, int col, int new_row, int new_col) {
	if ((row < 0) || (row > 7) || (col < 0) || (col > 7) ||
		(new_row < 0) || (new_row > 7) || (new_col < 0) || (new_row > 7)) {
		return false;
	}
	int diff_row = std::abs(row - new_row);
	int diff_col = std::abs(col - new_col);
	if (getStatus() == false && diff_row == 0 && diff_col == 2) {
		return true;
	}
	if (diff_row + diff_col == 1 || diff_row * diff_col == 1) {
		return true;
	}
	else {
		return false;
	}
}

	

