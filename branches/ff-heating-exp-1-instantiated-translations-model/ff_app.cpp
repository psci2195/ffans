/**************************************************************************
 * Copyright (C) 2011,2013 Dr. Bogdan Tanygin<b.m.tanygin@gmail.com>
 * All rights reserved.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "ff_app.h"

#include "ff_sys_graphics.h"
#include "ff_model_graphics.h"
#include "ff_model.h"

int main(int argc, char **argv)
//int _tmain(int argc, _TCHAR* argv[])
{
    /*ff_gr_params_t *ff_gr_params;
    ff_mgr_params_t *ff_mgr_params;
    ff_model_params_t *ff_model_params;
    
    ff_gr_params = new ff_gr_params_t;
    ff_mgr_params = new ff_mgr_params_t;
    ff_model_params = new ff_model_params_t;*/

	FILE *file, *file1, *file2, *file2_I, *file2_II, *file2_III;

    file  = fopen("setting_M.dat", "w");
    file1 = fopen("setting_I.dat", "w");
	file2 = fopen("setting_n_oleic.dat", "w");
	file2_I = fopen("setting_n_oleic_I.dat", "w");
	file2_II = fopen("setting_n_oleic_II.dat", "w");
	file2_III = fopen("setting_n_oleic_III.dat", "w");

	fclose(file);
    fclose(file1);
	fclose(file2);
	fclose(file2_I);
	fclose(file2_II);
	fclose(file2_III);
    
    // system graphics init
    ff_gr_init(argc, argv);
    
    // model graphics init
    ff_mgr_init();
    
    // physical model init
    ff_model_init();
    
    // run graphics loop
    ff_gr_loop();
    
    /*delete ff_gr_params;
    delete ff_mgr_params;
    delete ff_model_params;*/
    
    return 0;
}