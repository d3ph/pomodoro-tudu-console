
/**************************************************************************
 * Copyright (C) 2007-2011 Ruben Pollan Bella <meskio@sindominio.net>     *
 *                                                                        *
 *  This file is part of TuDu.                                            *
 *                                                                        *
 *  TuDu is free software; you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation; version 3 of the License.        *
 *                                                                        *
 *  TuDu is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 **************************************************************************/

#include "editor.h"
#include "cmd.h"

#define cmp(str) text.compare(0, length, str, 0, length)

Editor::Editor()
{
	text = L"";
	cursor = 0;
	key = '\0';
}

wstring& Editor::getText()
{
	return text;
}

int& Editor::cursorPos()
{
	return cursor;
}

Editor::return_t Editor::edit(Window& win, int begin_y, int begin_x, int ncols)
{
	window = &win;
	y = begin_y;
	x = begin_x;
	cols = ncols;
	exit = false;
	result = SAVED;
	echo();
	curs_set(1);
	initialize();

	while (!exit)
	{
		if (window->_getch(key) == KEY_CODE_YES)
		{
			switch (key)
			{
				case KEY_RESIZE:
					return RESIZE;
				case KEY_LEFT: left();
					break;
				case KEY_RIGHT: right();
					break;
				case KEY_UP: up();
					break;
				case KEY_DOWN: down();
					break;
				case KEY_HOME: home();
					break;
				case KEY_END: end();
					break;
				case KEY_BACKSPACE: backspace();
					break;
				case KEY_DC: supr();
					break;
				case KEY_ENTER: enter();
					break;
			}
		}
		else
		{
			switch (key)
			{
				case '\e': esc();
					break;
				case '\n': enter();
					break;
				case '\t': tab();
					break;
				default: other();
					break;
			}
		}
		updateText();
	}
	noecho();
	curs_set(0);
	window->_refresh();

	return result;
}

void Editor::initialize()
{
	window->_move(y, x);
	window->_addstr(text);
	window->_move(y, x+cursor);
	window->_refresh();
}

void Editor::updateText() {}
void Editor::left() {}
void Editor::right() {}
void Editor::up() {}
void Editor::down() {}
void Editor::home() {}
void Editor::end() {}
void Editor::backspace() {}
void Editor::supr() {}
void Editor::tab() {}
void Editor::other() {}

void Editor::esc()
{ 
	exit = true;
	result = NOT_SAVED;
}

void Editor::enter()
{ 
	exit = true;
}


/*
 *  Editor mono line
 */
void LineEditor::updateText()
{
	static int offset = 0;

	if (cols > text.length())
		offset = 0;
	if (cursor >= (int)cols+offset)
		offset = cursor-cols+1;
	if (cursor < offset)
		offset = cursor;

	window->_move(y, x);
	window->_addstr(text.substr(offset, cols));
	for (unsigned int i = text.length()-offset; i < cols; i++) 
			window->_addch(' ');
	window->_move(y, x+cursor-offset);
	window->_refresh();
}

void LineEditor::left()
{
	if (cursor>0) --cursor;
	else if (text.length() == 0)
	{
		exit = true;
		result = NOT_SAVED;
	}
}

void LineEditor::right()
{
	if (cursor<(int)text.length()) ++cursor;
}

void LineEditor::home()
{
	cursor = 0;
}

void LineEditor::end()
{
	cursor = text.length();
}

void LineEditor::backspace()
{
	if (cursor>0) text.erase(--cursor, 1);
}

void LineEditor::supr()
{
	if (cursor<(int)text.length()) text.erase(cursor, 1);
}

void LineEditor::tab()
{
	text.insert(cursor,1,' ');
	++cursor;
}

void LineEditor::other()
{
	text.insert(cursor,1,key);
	++cursor;
}


/*
 *  Editor for task titles, multilinear
 */
void TitleEditor::initialize()
{
	textLines = ((int)text.length()-1) / (int)cols;

	for (int i = 0; i <= textLines; i++)
	{
		wstring line(text.substr(i*cols, cols));
		window->_move(y+i, x);
		window->_addstr(line);
	}
	window->_move(y+cursorLine(), x+cursorCol());
	window->_refresh();
}

void TitleEditor::updateText()
{
	if (textLines != ((int)(text.length()-1) / (int)cols))
	{
		exit = true;
		result = REDRAW;
		return;
	}

	for (int i = 0; i <= textLines; i++)
	{
		wstring line(text.substr(i*cols, cols));
		window->_move(y+i, x);
		window->_addstr(line);
	}

	for (unsigned int i = ((text.length()-1) % cols); i < cols-1; i++)
			window->_addch(' ');
	window->_move(y+cursorLine(), x+cursorCol());
	window->_refresh();
}

void TitleEditor::up()
{
	if (cursorLine() > 0)
		cursor -= cols;
}

void TitleEditor::down()
{
	if ((int)cursorLine() < textLines)
	{
		cursor += cols;
		if (cursor > (int)text.length())
			cursor = text.length();
	}
}

unsigned int TitleEditor::cursorLine()
{
	return cursor / cols;
}

unsigned int TitleEditor::cursorCol()
{
	return cursor % cols;
}


/*
 *  Editor of Categories
 */
CategoryEditor::CategoryEditor():
		LineEditor(), search(categories.end()) {}

void CategoryEditor::tab() /* do completion */
{
	/* if it is no the first time */
	if ((cursor == (int)text.length()) &&
	    (search != categories.end()) &&
	    (text == *search))
	{
		search++;
		if ((search != categories.end()) && 
		   (!cmp(*search)))
		{
			text = *search;
			cursor = text.length();
		}
		else
		{
			text = text.substr(0, length);
			search = first;
			cursor = length;
		}
	}
	/* if it is the first time */
	else
	{
		length = text.length();
		for (search = categories.begin(); 
		    (search != categories.end()) && 
		    (cmp(*search)); 
		    search++);
		if ((search != categories.end()) && 
		    (!cmp(*search)))
		{
			text = *search;
			first = search;
			cursor = text.length();
		}
	}
}


/*
 *  Editor with history, used on search
 */
void HistoryEditor::initialize()
{
	shown = history.begin();
	LineEditor::initialize();
}

void HistoryEditor::up()
{
	if (shown != history.end())
	{
		text = *shown;
		cursor = text.length();
		shown++;
	}
}

void HistoryEditor::down()
{
	if (shown != history.begin())
	{
		shown--;
		text = *shown;
		cursor = text.length();
	}
}

void HistoryEditor::enter()
{
	exit = true;
	history.push_front(text);
}

void HistoryEditor::backspace()
{
	if (text.length() == 0)
	{
		exit = true;
		result = NOT_SAVED;
		return;
	}

	LineEditor::backspace();
}


/*
 *  Command editor
 */
void CmdEditor::initialize()
{
	/* initialize if is new command, 
	 * in other case we expect it to be already initialized */
	if (text == L"")
	{
		search = categories.end();
		com_search = commands.end();
		param = 0;
	}

	HistoryEditor::initialize();
}

void CmdEditor::tab() /* do completion */
{
	vector<wstring> params;
	size_t begin, end;
	wstring rest_params = L"";

	/* Get the command and params in text */
	for (begin = 0, end = text.find(' ', 0); (wstring::npos != end) && (cursor > (int)end);
		       	begin = end+1, end = text.find(' ', begin))
	{
		params.push_back(text.substr(begin, end-begin));
	}
	if (wstring::npos != end) 
	{
		params.push_back(text.substr(begin, end-begin));
		rest_params = text.substr(end+1);
	}
	else
	{
		params.push_back(text.substr(begin));
	}

	/* if is completing the command */
	if (params.size() == 1)
	{
		command_completion(params[0]);
	}
	/* if is completing the param */
	else
	{
		if (commands[params[0]] == L"category")
			category_completion(params.back(), params.size()-1);
	}

	/* regenerate the text string */
	text = L"";
	for (vector<wstring>::iterator p = params.begin(); p != params.end(); p++)
	{
		text += *p + L" ";
	}
	cursor = text.length()-1;
	if (rest_params.empty())
		text.erase(text.length()-1, 1);
	else
		text += rest_params;
	param = params.size()-1;
}

void CmdEditor::command_completion(wstring& com)
{
	/* if it is not the first time */
	if ((param == 0) &&
	    (com_search != commands.end()) &&
	    (com == com_search->first))
	{
		com_search++;
		if ((com_search != commands.end()) && 
		   (!cmp(com_search->first)))
		{
			com = com_search->first;
			cursor = com.length();
		}
		else
		{
			com = com.substr(0, length);
			com_search = com_first;
			cursor = length;
		}
	}
	/* if it is the first time */
	else
	{
		length = com.length();
		//TODO: try upper_bound
		for (com_search = commands.begin(); 
		    (com_search != commands.end()) && 
		    (cmp(com_search->first)); 
		    com_search++);
		if ((com_search != commands.end()) && 
		    (!cmp(com_search->first)))
		{
			com = com_search->first;
			com_first = com_search;
			cursor = com.length();
		}
	}
}

#define param_cmp(str) cat.compare(0, length, str, 0, length)
void CmdEditor::category_completion(wstring& cat, int num_param)
{

	/* if it is no the first time */
	if ((param == num_param) &&
	    (search != categories.end()) &&
	    (cat == *search))
	{
			search++;
			if ((search != categories.end()) && 
			   (!param_cmp(*search)))
			{
				cat = *search;
			}
			else
			{
				cat = cat.substr(0, length);
				search = first;
			}
	}
	/* if it is the first time */
	else
	{
		length = cat.length();
		for (search = categories.begin(); 
		    (search != categories.end()) && 
		    (param_cmp(*search)); 
		    search++);
		if ((search != categories.end()) && 
		    (!param_cmp(*search)))
		{
			cat = *search;
			first = search;
		}
	}
}


/*
 *  Editor of dates
 */
Editor::return_t DateEditor::edit(Window& win, int begin_y, int begin_x)
{
	return Editor::edit(win, begin_y, begin_x, 11);
}

void DateEditor::updateText()
{
	if (cursor >= (int) cols)
		cursor = cols-1;
	window->_addstr(y, x, text);
	window->_move(y, x+cursor);
	window->_refresh();
}

void DateEditor::left()
{
	if (cursor>0) --cursor;
	if ((2==cursor) || (5==cursor)) --cursor;
}

void DateEditor::right()
{
	if (cursor<9) ++cursor;
	if ((2==cursor) || (5==cursor)) ++cursor;
}

void DateEditor::up()
{
	if ((text[cursor] < '0') || (text[cursor] > '9'))
		text[cursor] = '0';
	if (text[cursor] < '9')
		text[cursor]++;
}

void DateEditor::down()
{
	if ((text[cursor] > '0') && (text[cursor] <= '9'))
		text[cursor]--;
}

void DateEditor::home()
{
	cursor = 0;
}

void DateEditor::end()
{
	cursor = 9;
}

void DateEditor::other()
{
	/* if is a number */
	if ((key >= 48) && (key <= 57))
	{
		text[cursor] = key;
		if (cursor<9) ++cursor;
		if ((2==cursor) || (5==cursor)) ++cursor;
	}
}


/*
 *  Editor of priorities
 */
Editor::return_t PriorityEditor::edit(Window& win, int begin_y, int begin_x)
{
	return Editor::edit(win, begin_y, begin_x, 1);
}

void PriorityEditor::updateText()
{
	cursor = 0;
	window->_addstr(y, x, text);
	window->_move(y, x);
	window->_refresh();
}

void PriorityEditor::up()
{
	if ((text != L"N") && (text[cursor] > 49))
	{
		text[cursor]--;
	}
	else
	{
		text = L"1";
	}
}

void PriorityEditor::down()
{
	if ((text != L"N") && (text[cursor] < 53))
	{
		text[cursor]++;
	}
	else
	{
		text = L"5";
	}
}

void PriorityEditor::backspace()
{
	text = L"N";
}

void PriorityEditor::supr()
{
	text = L"N";
}

void PriorityEditor::other()
{
	/* if is a number between 1 and 5 */
	if ((key >= 49) && (key <= 53))
	{
		text[cursor] = key;
	}
}
