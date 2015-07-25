#include "board.h"
#include "textdisplay.h"
#include "concrete_pieces.h"
#include "human.h"
#include "computer.h"
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>

Board* Board::singleton = NULL;


Board::Board() {
	std::vector <Pieces*> tmp;
	tmp.assign(8, NULL);
	theBoard.assign(8, tmp);
	std::vector <Pieces*> empty;
	std::vector < std::vector <Pieces* > > tmp2(8, empty);
	attackBoard.assign(8, tmp2);
	p1Score = 0;
	p2Score = 0;
	turn = 0;
	p1 = NULL;
	p2 = NULL;
	gd = NULL;
	td = NULL;
	playing = false;
	enpassant = NULL;	
}

Board::~Board() {
	delete gd;
	delete td;
	delete p1;
	delete p2;
	p1Score = 0;
	p2Score = 0;
	playing = false;
	enpassant = NULL;
	updateEnpassant = false;
}
	
	
Board* Board::getInstance() {
	if(singleton) return singleton;
	singleton = new Board;
	atexit(cleanup);
	return singleton;
}

void Board::cleanup() {
   delete singleton;
}

void Board::clearGame() {
	for(int i = 0; i < 8; i ++) {
		for(int j = 0; j < 8; j ++) {
			if(theBoard[i][j] != NULL) {
				delete theBoard[i][j];
				theBoard[i][j] = NULL;
			}
			while(attackBoard[i][j].size() != 0) attackBoard[i][j].pop_back();
		}
	}
	turn = 0;
	playing = false;
	enpassant = NULL;
	updateEnpassant = false;
	delete td;
	td = NULL;
}

void Board::remove(int r, int c) {
	if(0 <= r && r < 8 && 0 <= c && c < 8 && theBoard[r][c]) {
		if(theBoard[r][c] != NULL) {
			removeRange(r, c);
			delete theBoard[r][c];
		}
		theBoard[r][c] = NULL;
		td->notify(r, c);
	}
	else std::cout << "invalid remove" << std::endl;
}

void Board::add(int r, int c, char p) {
	if(0 <= r && r < 8 && 0 <= c && c < 8) {
		if(theBoard[r][c]) {
			remove(r, c);
		}
		if(p == 'r' || p == 'R') {
			theBoard[r][c] = new Rook(r, c, p, false);
			updatePiece(r, c);
			updateGrid(r, c);
		}
		if(p == 'n' || p == 'N') {
			theBoard[r][c] = new Knight(r, c, p, false);	
			updatePiece(r, c);
			updateGrid(r, c);
		}
		if(p == 'b' || p == 'B') {
			theBoard[r][c] = new Bishop(r, c, p, false);
			updatePiece(r, c);
			updateGrid(r, c);
		}
		if(p == 'q' || p == 'Q') {
			theBoard[r][c] = new Queen(r, c, p, false);
			updatePiece(r, c);
			updateGrid(r, c);
		}
		if(p == 'k' || p == 'K') {
			theBoard[r][c] = new King(r, c, p, false);
			updatePiece(r, c);
			updateGrid(r, c);
		}
		if(p == 'p' || p == 'P') {
			theBoard[r][c] = new Pawn(r, c, p, false);
			updatePiece(r, c);
			updateGrid(r, c);
		}
		td->notify(r, c, p);
	}
	else std::cout << "not valid add" << std::endl;
}

Pieces* Board::getPiece(int r, int c) { return theBoard[r][c]; }


void Board::removeRange(int r, int c) {
	Pieces* current = theBoard[r][c];
	std::vector < std::pair <int ,int> > range = theBoard[r][c]->getRange();
	for(std::vector < std::pair <int, int> >::iterator it = range.begin(); it != range.end(); it ++) {
		int newr = it->first;
		int newc = it->second;
		std::vector <Pieces*> attack = attackBoard[newr][newc];
		if(attack.size() != 0) {
			for( std::vector < Pieces* >::iterator ait = attackBoard[newr][newc].begin(); ait < attackBoard[newr][newc].end(); ait ++) {
				if(*ait == current) attackBoard[newr][newc].erase(ait);
			}
		}
	}
}


void Board::updatePiece(int r, int c) {
	removeRange(r, c);
	std::vector < std::pair <int ,int> > range = theBoard[r][c]->getRange();
	for(std::vector < std::pair <int, int> >::iterator it = range.begin(); it != range.end(); it ++) {
		int newr = (*it).first;
		int newc = (*it).second;
		if(preCheck(r, c, newr, newc)) {
			attackBoard[newr][newc].push_back(theBoard[r][c]);
		}
	}
}

void Board::updateGrid(int r, int c) {
	std::vector < Pieces *> attack = attackBoard[r][c];
	if(attack.size() == 0) {
		return ;
	}
	else {
		int size = attackBoard[r][c].size();
		std::vector < std::pair <int, int> > total;
		for(int i = 0; i < size; i ++) {
			std::pair <int, int> tmp;
			tmp.first = attackBoard[r][c][i]->getr();
			tmp.second = attackBoard[r][c][i]->getc();
			total.push_back(tmp);
		}
		int bsize = total.size();
		for(int i = 0; i < bsize; i ++) {
			updatePiece(total[i].first, total[i].second);
		}
	}
}




std::vector <int> Board::convert(std::string pos) {
	std::stringstream ss(pos);
	char c;
	int n;
	std::vector <int> result;
	ss >> c >> n;
	result.push_back(8 - n);
	result.push_back(c - 'a');
	return result;
}

bool Board::checkBoard() {
	std::vector <int> pieces (60, 0);
	for(std::vector < std::vector <Pieces*> >::iterator it = theBoard.begin(); it != theBoard.end(); it ++) {
		for(std::vector <Pieces*>::iterator i = it->begin(); i != it->end(); i ++) {
			if(*i != NULL) {
				pieces[(*i)->getName() - 'A'] += 1;
			}
		}
	}
	for(std::vector <int>::iterator it = pieces.begin(); it != pieces.end(); it ++) {
		if((it == pieces.begin() + 42 || it == pieces.begin() + 10) && (*it != 1))  {
			std::cout << "invalid number of Kings" << std::endl;
			return false;  //must have exactly 1 King on each side
		}	
		else if ((it == pieces.begin() + 16 || it == pieces.begin() + 48) && *it > 1) {
			std::cout << "invalid number fo Queens" << std::endl;
			return false; //can have at most one Queen
		}
		else if((it == pieces.begin() + 15 || it == pieces.begin() + 47) && *it > 8) {
			std::cout << "invalid number of Pawns" << std::endl;
			return false; //can have at most 8 Pawn
		}
		else if((it != pieces.begin() + 15 && it != pieces.begin() + 47) && *it > 2)  {
			std::cout << "invalid number of other pieces (e.g. Bishops, Knights or Rooks)" << std::endl;
			return false; //everything else must have at most 2
		}
	}
	if(check('K')) {
		std::cout << "white king is in check" << std::endl;
		return false;
	}
	if(check('k')) {
		std::cout << "black king is in check" << std::endl;
		return false;
	}
	for(int i = 0; i < 8; i ++) {
		Pieces* tmp = theBoard[0][i];
		Pieces* tmp2 = theBoard[7][i];
		if(tmp != NULL && (tmp->getName() == 'P' || tmp->getName() == 'p')) {
			std::cout << "Pawn(s) are not in valid position" << std::endl;
			return false;
		}
		if (tmp2 != NULL && (tmp2->getName() == 'p' || tmp2->getName() == 'P')) {
			std::cout << "Pawn(s) are not in valid position" << std::endl;
			return false;
		}
	}
	return true;
}

bool Board::preCheck(int row, int col, int new_row, int new_col) {
	Pieces *tmp = theBoard[row][col];
	Pieces *newtmp = theBoard[new_row][new_col];
	char n = tmp->getName();
	int diff_row = std::abs(row - new_row);
	int diff_col = std::abs(col - new_col);
	int dir_row;
	int dir_col;
	if (diff_row != 0) {
		dir_row = diff_row / (new_row - row);
	}
	else {
		dir_row = 0;
	}
	if (diff_col != 0) {
		dir_col = diff_col / (new_col - col);
	}
	else {
		dir_col = 0;
	}
	if (tmp->moveCheck(row,col,new_row,new_col) == false) {
		return false;
	}
	else if (n == 'N' || n == 'n' || n == 'K' || n == 'k') {
		return true;
	}
	else if (n == 'R' || n == 'r' || n == 'Q' || n == 'q' || n == 'B' || n == 'b') {
		for (int i = 1; i < std::max(diff_row, diff_col); i++) {
			if (theBoard[row + dir_row * i][col + dir_col * i] != NULL) {
				return false;
			}
		}
		return true;
	}
	else if (n == 'p' && diff_col == 0) {
		if (theBoard[row+1][col] != NULL) {
			return false;
		}
		if (diff_row == 2 && theBoard[row+2][col] != NULL) {
			return false;
		}
		else {
			return true;
		}
	}
	else if (n == 'P' && diff_col == 0) {
		if (theBoard[row-1][col] != NULL) {
			return false;
		}
		if (diff_row == 2 && theBoard[row-2][col] != NULL) {
			return false;
		}
		else {
			return true;
		}
	}
	else if(n == 'p' || n == 'P') return true;
}


bool Board::castling(int r, int c, int nr, int nc, char k) {
	int col_diff = abs(nc - c);
	int dir = col_diff / (nc - c);
	std::vector < Pieces* > tmp;
	if(theBoard[r][c]->getStatus()) {
		std::cout << r << " " << c << " has been moved" << std::endl;
		std::cout << "king has been moved" << std::endl;
		return false;
	}
	if(check(k)) {
		std::cout << "king is in check" << std::endl;
		return false;
	}
	else if (dir > 0) {
		std::cout << "dir > 0" << std::endl;
		if(theBoard[r][c + 3] == NULL) return false;
		if(theBoard[r][c + 3] != NULL && theBoard[r][c + 3]->getStatus()) return false;
		for(int i = 1; i < 3; i ++) {
			if(theBoard[r][c + i * dir] != NULL) return false;
			tmp = attackBoard[r][c + i * dir];
			for(std::vector < Pieces* >::iterator it = tmp.begin(); it != tmp.end(); it ++) {
				if(std::abs((*it)->getName() - k) > 22) return false;
			}
		}
		return true;
	}
	else if (dir < 0) {
		std::cout << "dir < 0" << std::endl;
		if(theBoard[r][c - 4] == NULL) {
			std::cout << "no rook" << std::endl;
			return false;
		}
		if(theBoard[r][c - 4] != NULL && theBoard[r][c - 4]->getStatus()) {
			std::cout << "rook has been moved" << std::endl;
			return false;
		}
		for(int i = 1; i < 4; i ++) {
			if(theBoard[r][c + i * dir] != NULL) {
				std::cout << "there is a piece on the road" << std::endl;
				return false;
			}
			tmp = attackBoard[r][c + i * dir];
			if(i < 3) {
				std::cout << "this is in the loop" << std::endl;
				for(std::vector < Pieces* >::iterator it = tmp.begin(); it != tmp.end(); it ++) {
					if(std::abs((*it)->getName() - k) > 22) {
						std::cout << "maybe check on the road" << std::endl;
						return false;
					}
				}
			}
		}
		return true;
	}
}

bool Board::ruleCheck(int row, int col, int new_row, int new_col) {
	std::cout << "in rule" << std::endl;
	int col_diff, row_diff;
	row_diff = abs(new_row - row);
	col_diff = abs(new_col - col);
	bool isMove = false;
	Pieces* tmp1 = theBoard[row][col];
	std::cout << "Checking does grid " << new_row << " " << new_col << " has " << tmp1->getName() << std::endl;
	for(std::vector <Pieces*>::iterator it = attackBoard[new_row][new_col].begin(); it != attackBoard[new_row][new_col].end(); it ++) {
		if((*it) == tmp1) {
			isMove = true;
		}
	}
	if((tmp1->getName() == 'p' || tmp1->getName() == 'P') && col_diff == 1) {
		std::cout << col_diff << std::endl;
		if(theBoard[new_row][new_col] != NULL || (static_cast< Pawn* >(theBoard[row][new_col]) == enpassant && enpassant != NULL)) isMove = true;
		else {
			return false;
		}
	}
	if((tmp1->getName() == 'K' || tmp1->getName() == 'k') && (col_diff == 2)) {
		std::cout << "running castling" << std::endl;
		if(castling(row, col, new_row, new_col, tmp1->getName())) return true;
		else {
			std::cout << "castling fail" << std::endl;
			return false;
		}
	}
	if(theBoard[new_row][new_col] != NULL && abs(tmp1->getName() - theBoard[new_row][new_col]->getName()) < 25) {
		std::cout << "you cannot eat allies " << std::endl;
		return false;
	}
	else {
		Pieces* tmp = NULL;
		if(theBoard[new_row][new_col] != NULL) {
			tmp = theBoard[new_row][new_col];
			removeRange(new_row, new_col);
		}
		removeRange(row, col);
		theBoard[new_row][new_col] = theBoard[row][col];
		theBoard[row][col] = NULL;
		theBoard[new_row][new_col]->setr(new_row);
		theBoard[new_row][new_col]->setc(new_col);
		theBoard[new_row][new_col]->setRange();
		updateGrid(row, col);
		updateGrid(new_row, new_col);
		updatePiece(new_row, new_col);
		if((turn == 0 && check('K')) || (turn == 1 && check('k'))) {
			std::cout << "this move will put your king in check" << std::endl;
			removeRange(new_row, new_col);
			theBoard[row][col] = theBoard[new_row][new_col];
			theBoard[new_row][new_col] = tmp;
			theBoard[row][col]->setr(row);
			theBoard[row][col]->setc(col);
			theBoard[row][col]->setRange();
			updateGrid(row, col);
			updateGrid(new_row, new_col);
			if(tmp) updatePiece(new_row, new_col);
			updatePiece(row, col);
			return false;
		}
		else {
			removeRange(new_row, new_col);
			theBoard[row][col] = theBoard[new_row][new_col];
			theBoard[new_row][new_col] = tmp;
			theBoard[row][col]->setr(row);
			theBoard[row][col]->setc(col);
			theBoard[row][col]->setRange();
			updateGrid(row, col);
			updateGrid(new_row, new_col);
			if(tmp) updatePiece(new_row, new_col);
			updatePiece(row, col);
			if(isMove) return true;
			else return false;	
		}
	}
}


void Board::notify(std::string move, char team) {
	if(move == "resign") {
		if(turn == 0) {
			std::cout << "black wins!" << std::endl;
			p2Score ++;
			turn = 0;
			playing = false;
		}
		else {
			std::cout << "white wins!" << std::endl;
			p1Score ++;
			playing = false;
			turn = 0;
		}
	}
	else {
		std::stringstream ss(move);
		std::string pos1, pos2;
		ss >> pos1 >> pos2;
		int oldr, oldc, newr, newc;
		oldr = convert(pos1)[0];
		oldc = convert(pos1)[1];
		newr = convert(pos2)[0];
		newc = convert(pos2)[1];
		if(oldr < 0 || oldr > 7 || oldc < 0 || oldc > 7 || newr < 0 || newr > 7 || newc < 0 || newc > 7) {
			std::cout << "out of range please enter again" << std::endl;
			if(turn == 0) p1->makeMove();
			else p2->makeMove();
			return;
		}
		std::cout << "initial over" << std::endl;
		std::cout << "checking from " << oldr << " " << oldc << " to " << newr << " " << newc << std::endl;
		if(theBoard[oldr][oldc] == NULL ||!ruleCheck(oldr, oldc, newr, newc)) {
			std::cout << "invalid move please enter again" << std::endl;
			if(turn == 0) p1->makeMove();
			else p2->makeMove();
			return;
		}
		else {
			char piece = theBoard[oldr][oldc]->getName();
			if(abs(piece - team) > 25) {
				std::cout << "you cannot move your opponents pieces" << std::endl;
				if(turn == 0) p1->makeMove();
				else p2->makeMove();
				return;
			}
			else {
				int col_diff = abs(oldc - newc);
				char name = theBoard[oldr][oldc]->getName();
				if((name == 'k' || name == 'K') && col_diff == 2) {
					int dir = col_diff / (newc - oldc);
					this->move(oldr, oldc, newr, newc);
					if(dir > 0) this->move(oldr, oldc + 3, oldr, oldc + 1);
					else this->move(oldr, oldc - 4, oldr, oldc - 1);
					return ;
				}
				this->move(oldr, oldc, newr, newc);
			}
		}
	}
}
				




bool Board::check(char king) {
	int row, col;
	char piece;
	for(int i = 0; i < 8; i ++) 
		for(int j = 0; j < 8; j ++) 
			if(theBoard[i][j] != NULL) 
				if(theBoard[i][j]->getName() == king) {
					row = i;
					col = j;
					std::cout << "the king locatets at " << row << " " << col << std::endl;
				}
	std::vector< Pieces* > tmp = attackBoard[row][col];
	for(std::vector< Pieces* >::iterator it = tmp.begin(); it != tmp.end(); it ++) {
		if(abs((*it)->getName() - king) > 22) {
			std::cout << "in check" << std::endl;
			return true;
		}
	}
	return false;
}

bool Board::checkMate(char king) {
	int r, c, checkr, checkc;
	char name;
	for(int i = 0; i < 8; i ++) 
		for(int j = 0; j < 8; j ++) 
			if(theBoard[i][j] != NULL) 
				if(theBoard[i][j]->getName() == king) {
					r = i;
					c = j;
				}
	std::vector< std::pair <int, int> > range = theBoard[r][c]->getRange();
	for(std::vector < std::pair <int, int> >::iterator it = range.begin(); it != range.end(); it ++) {
		int nr = it->first;
		int nc = it->second;
		if(ruleCheck(r, c, nr, nc)) return false;
	}
	int count = 0;
	std::vector <Pieces *> attack = attackBoard[r][c];
	for(std::vector <Pieces *>::iterator it = attack.begin(); it != attack.end(); it ++) {
		name = (*it)->getName();
		if(abs(name - king) > 22) {
			checkr = (*it)->getr();
			checkc = (*it)->getc();
			count ++;
		}
	}
	if(count > 1) return true;
	else {
		attack = attackBoard[checkr][checkc];
		for(std::vector <Pieces *>::iterator it = attack.begin(); it != attack.end(); it ++) {
			name = (*it)->getName();
			if(abs(name - king) < 22) return false;
		}
		if(name == 'n' || 'N') return true;
		else {
			int row_dir, col_dir;
			int row_diff = abs(checkr - r);
			int col_diff = abs(checkc - c);
			if(row_diff == 0) row_dir = 0;
			else row_dir = row_diff / (checkr - r);
			if(col_diff == 0) col_dir = 0;	
			else col_diff = col_diff / (checkc - c);
			for(int i = 1; i < std::max(row_diff, col_diff); i ++) {
				checkr = r + row_dir * i;
				checkc = c + row_dir * c;
				attack = attackBoard[checkr][checkc];
				for(std::vector <Pieces *>::iterator it = attack.begin(); it != attack.end(); it ++) {
					name = (*it)->getName();
					if(abs(name - king) < 22) return false;
				}
			}
			return true;
		}
	}
}

void Board::move(int oldr, int oldc, int newr, int newc) {
	std::cout << "into the move " << std::endl;
	removeRange(oldr, oldc);
	char name = theBoard[oldr][oldc]->getName();
	if(name == 'p' || name == 'P') {
		if(abs(newr - oldr) == 2) {
			enpassant = static_cast<Pawn *>(theBoard[oldr][oldc]);
			updateEnpassant = true;
		}
		else if(theBoard[newr][newc] == NULL && abs(newc - oldc) == 1) {
			remove(oldr, newc);
			updateEnpassant = false;
		}
		else updateEnpassant = false;
	}
	if(theBoard[newr][newc] != NULL) {
		if(static_cast<Pawn *>(theBoard[newr][newc]) == enpassant) enpassant = NULL;
		remove(newr, newc);
		updateEnpassant = false;
	}
	theBoard[oldr][oldc]->setMove(true);
	std::cout << "setting " << theBoard[oldr][oldc]->getName() << "'s Move to true" << std::endl;
	theBoard[newr][newc] = theBoard[oldr][oldc];
	theBoard[oldr][oldc] = NULL;
	theBoard[newr][newc]->setr(newr);
	theBoard[newr][newc]->setc(newc);
	theBoard[newr][newc]->setRange();
	updateGrid(oldr, oldc);
	updateGrid(newr, newc);
	updatePiece(newr, newc);
	if(name != 'p' && name != 'P') updateEnpassant = false;
	td->notify(oldr, oldc);
	td->notify(newr, newc, name);
	td->print();
}



void Board::play() {
	std::string command;
	while(std::cin >> command) {
		if(command == "setup") {
			delete td;
			td = new TextDisplay();
			td->print();
			while(true) {
				std::string opt;
				std::cin >> opt;
				if(opt == "-") {
					std::string pos;
					std::cin >> pos;
					remove(convert(pos)[0], convert(pos)[1]);
					td->print();
				}
				else if(opt == "+") {
					char piece;
					std::string pos;
					std::cin >> pos >> piece;
					if(piece == 'r' || piece == 'R' ||piece == 'B' ||piece == 'b' ||piece == 'N' ||piece == 'n' ||piece == 'Q' ||piece == 'q' ||piece == 'K' ||piece == 'k' ||piece == 'P' ||piece == 'p') {
						add(convert(pos)[0], convert(pos)[1], piece);
						td->print();
					}
					else std::cout << "not valid piece" << std::endl;
				}
				else if(opt == "=") {
					std::string color;
					std::cin >> color;
					if(color == "white") turn = 0;
					else if(color == "black") turn = 1;
					else std::cout << "not valid color" << std::endl;
				}
				else if(opt == "stdinit") {              //this command provide a standard initial state of a chess
					for(int i = 0; i < 8; i ++) {
						add(1, i, 'p');
						add(6, i, 'P');
					}
					add(0, 0, 'r');
					add(0, 7, 'r');
					add(0, 1, 'n');
					add(0, 6, 'n');
					add(0, 2, 'b');
					add(0, 5, 'b');
					add(0, 3, 'q');
					add(0, 4, 'k');
					add(7, 0, 'R');
					add(7, 7, 'R');
					add(7, 1, 'N');
					add(7, 6, 'N');
					add(7, 2, 'B');
					add(7, 5, 'B');
					add(7, 3, 'Q');
					add(7, 4, 'K');
					td->print();
				}
				else if(opt == "grid") {
					int r = 0, c = 0;
					std::cin >> r >> c;
					std::cout << "the pieces in grid " << r << " " << c << " are(is)" << std::endl;
					int size = attackBoard[r][c].size();
					for(int i = 0; i < size; i++) {
						std::cout << attackBoard[r][c][i]->getName() << std::endl;
					}
				}

				else if(opt == "done") {
					if(checkBoard()) break;
					else std::cout << "the current board is invalid you cannot exit setup mode" << std::endl;
				}
			}
		}
		else if(command == "game") {
			playing = true;
			std::string player1, player2;
			std::cin >> player1 >> player2;
			if(player1 == "human") p1 = new Human(this, 'A');
			else p1 = new Computer(this, 'A');
			if(player2 == "human") p2 = new Human(this, 'z');
			else p2 = new Computer(this, 'z');
			td->print();
			std::cout << "the battle begins!" << std::endl;
			while(true) {
				if(!updateEnpassant) {
					enpassant = NULL;
				}
				if(turn == 0 && playing) {
					std::cout << "white's turn to move" << std::endl;
					p1->makeMove();
					if(check('k')) {
						std::cout << "Black is in check!" << std::endl;
						if(checkMate('k')) {
							std::cout << "Checkmate! White wins!" << std::endl;
							playing = false;
							p1Score ++;
							break;
						}
					}
				}
				else if(turn == 1 && playing) {
					std::cout << "black's turn to move" << std::endl;
					p2->makeMove();
					if(check('K')) {
						std::cout << "White is in check!" << std::endl;
						if(checkMate('K')) {
							std::cout << "Checkmate! Black wins!" << std::endl;
							playing = false;
							p2Score ++;
							break;
						}
					}
				}
				turn = !turn;
				if(!playing) {
					std::cout << "White: " << p1Score << std::endl;
					std::cout << "Black: " << p2Score << std::endl;
					clearGame();
					break;
				}
			}
		}
	}
}


