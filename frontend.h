#ifndef __FRONTEND_H__
#define __FRONTEND_H__

/* frontend.h - part of mindex
 * Copyright © 2012 Frank Joseph Greer
 *
 *  mindex is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* typdefs */
typedef struct {
  char name[20];
  int  index;
} menu_t;

/* macro */
#define MENU_ADD(num,var,str,idx)		\
  var[num].index = idx;				\
  strcpy(var[num].name,str)

/* Frontend helper functions */
int show_menu(menu_t* menu,int num,int deflt); /* display the main menu, returns the selection */


#endif /* __FRONTEND_H__ */
