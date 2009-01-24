/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#define BG_BOLD 	"1"
#define BG_UNDERLINE 	"4"
#define BG_BLINKING 	"5"
#define BG_FG_INVERSION "7"
#define FG_BLACK 	"30"
#define FG_RED 		"31"
#define FG_GREEN	"32"
#define FG_YELLOW 	"33"
#define FG_BLUE 	"34"
#define FG_MAGENTA 	"35"
#define FG_CYAN 	"36"
#define FG_GRAY 	"37"
#define FG_WHITE 	"38"
#define BG_BLACK 	"40"
#define BG_RED 		"41"
#define BG_GREEN 	"42"
#define BG_YELLOW 	"43"
#define BG_BLUE 	"44"
#define BG_MAGENTA 	"45"
#define BG_CYAN 	"46"
#define BG_GRAY 	"47"

#define _(bg,fg,bl,string)	"\E[" bg ";" fg ";" bl "m" string "\E[0m"


