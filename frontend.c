/* frontend.c - part of mindex
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

#include <stdio.h>
#include "frontend.h"

int show_menu(menu_t* menu,int num,int deflt) {
  int retval = 0;

  for (int i = 0; i < num; i++)
    printf("%d:\t%s\n",menu[i].index,menu[i].name);
  printf("\nChoice [%d-%d]: ",menu[0].index,menu[num-1].index);
  scanf("%d",&retval);

  return retval;
}

