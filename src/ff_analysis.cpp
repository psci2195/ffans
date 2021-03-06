/**************************************************************************
* Copyright (C) 2016,2017 Dmytro Matskevych<dimqqqq@mail.ru>
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
#include "ff_analysis.h"
#include "ff_sys_graphics.h"
#include "ff_model.h"
#include "ff_model_graphics.h"
#include "ff_iniParse.h"
#include "ff_image_module.h"
#include "ff_model_parameters.h"

#include <Windows.h>
#include <GdiPlus.h>
#include <string>
#include <GdiPlusBitmap.h>
#include <Ole2.h>
#include <OleCtl.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>
#include <stdio.h>


using namespace Gdiplus;
using namespace std;

class par
{
public:
	int m;
	int n;
	par(int n_, int m_)
	{
		m = m_;
		n = n_;
	}
};
fstream CSVresult = fstream("Savg.csv");
HWND hWnd;
bool Active = true;
struct CameraPosition
{
	float x;
	float y;
	float z;
	int projection_type_status;
};
int counterOfPosition = 0;
int MaxPointOfPosition =0;
/*
struct par
{
	int n;
	int m;
};*/
vector<CameraPosition> position;

void ActiveWindow()
{
	if(Active){hWnd = GetForegroundWindow();Active = false;}
}
 double ff_visousity_mix(double y1,
						double mu1,
						double y2,
						double mu2)
{
	return pow(10,((y1*log10(mu1))+(y2*log10(mu2))));
}
 double ff_molar_part(double nu1, double nu2)
 {
	 return nu1/(nu1+nu2);
 }
 double ff_mol(double m, double mol_m)
 {
	 return m/mol_m;
 }

void GetScreenShot(string name1) //Make Screen Shot
{
    int x1, y1, x2, y2, w, h;
    // get screen dimensions
    x1  = GetSystemMetrics(SM_XVIRTUALSCREEN);
    y1  = GetSystemMetrics(SM_YVIRTUALSCREEN);
    x2  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    y2  = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    w   = x2 - x1;
    h   = y2 - y1;

    // copy screen to bitmap 
	HDC     hScreen = GetDC(hWnd);
    HDC     hDC     = CreateCompatibleDC(hScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, w, h);
    HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
    BOOL    bRet    = BitBlt(hDC, 0, 0, w, h, hScreen, x1, y1, SRCCOPY);

    // save bitmap to clipboard
    OpenClipboard(NULL);
    EmptyClipboard ();
    SetClipboardData(CF_BITMAP, hBitmap);
    CloseClipboard();   


	//CImage image;
	//image.Attach(hBitmap);
	//image.Save("Test",ImageFormatBMP);
	//CloseClipboard();

	//var DataSet = 
	
	HBITMAP hb = (HBITMAP)SelectObject(hDC, old_obj);

	// Convert string to WCHAR*.
	const size_t cSize = strlen(name1.data()) + 1;
	wchar_t* name = new wchar_t[cSize];
	mbstowcs(name, name1.data(), cSize);

	int type = (int)iniGet("ImageSettings", "FileType");

	SaveImage(hBitmap, name, (FileTypes)type);
	
	// clean up
	delete name;
    SelectObject(hDC, old_obj);
    DeleteDC(hDC);
    ReleaseDC(NULL, hScreen);
    DeleteObject(hBitmap);
}

void addPosition(float x, float y, float z, int pts)
{
	position.push_back(CameraPosition());
	MaxPointOfPosition++;
	counterOfPosition = MaxPointOfPosition-1;
	position[counterOfPosition].x = x;
	position[counterOfPosition].y = y;
	position[counterOfPosition].z = z;
	position[counterOfPosition].projection_type_status = pts;
    if(isShowInfo!=0)
    {
	    cout<<"Position["<<counterOfPosition<<"] ("<<position[counterOfPosition].x<<","<<position[counterOfPosition].y<<","<<position[counterOfPosition].z<<") is saved"<<endl;
    }
    counterOfPosition++;
}

void delPosition()
{
	if(MaxPointOfPosition)
	{
        if(isShowInfo!=0)
        {
		    cout<<"Position["<<counterOfPosition<<"] ("<<position[counterOfPosition].x<<","<<position[counterOfPosition].y<<","<<position[counterOfPosition].z<<") is removed"<<endl;
        }
        position.erase(position.begin()+counterOfPosition);
		MaxPointOfPosition--;
		if(counterOfPosition<MaxPointOfPosition-1)
		{
			counterOfPosition++;
		}
		else
		{
			counterOfPosition=0;
		}
	}
	else
	{
		cout<<"U have not saved position yet"<<endl;
	}
}

void ChangePosition()
{
	if(MaxPointOfPosition)
	{
		if(counterOfPosition<MaxPointOfPosition-1)
		{
			counterOfPosition++;
		}
		else
		{
			counterOfPosition=0;
		}
        if(isShowInfo!=0)
        {
		    cout<<"Position["<<counterOfPosition<<"] ("<<position[counterOfPosition].x<<","<<position[counterOfPosition].y<<","<<position[counterOfPosition].z<<") is changed"<<endl;
        }
        projection_type = position[counterOfPosition].projection_type_status;
		x_rot = position[counterOfPosition].x;
		y_rot = position[counterOfPosition].y;
		space_k = position[counterOfPosition].z;
		cbResizeScene(window_width, window_height);
		
	}
	else
	{
		cout<<"U have not saved position yet"<<endl;
	}
}

void auto_set_position(int isAutoSet_)
{
    int iMax;
    if(isAutoSet_ != 0)
    {
    iMax=(int)iniGet("SetPosition","PositionMax");
        for(int i = 1;i<=iMax;i++)
        {
        ostringstream x;
        ostringstream y;
        ostringstream z;
        ostringstream pts;
        x<<"x"<<i;
        y<<"y"<<i;
        z<<"z"<<i;
        pts<<"pts"<<i;
        addPosition(iniGet("SetPosition",x.str()),iniGet("SetPosition",y.str()),iniGet("SetPosition",z.str()),iniGet("SetPosition",pts.str()));
        }
    }
}

void ff_pieces_coord_info()
{
	vector<par> pars;
	vector<vector<int>>parList;
	int counter = 0;
	for (int i = 0; i < pN-1 ; i++)
	{
		//cout << "Pieces #:" << i << "  X_coord: " << r[i].x << "  Y_coord: " << r[i].y << "  Z_coord: " << r[i].z << endl;
		for (int j = i+1 ; j < pN-1 ; j++)
		{
			if (sqrt(pow(r[i].x - r[j].x, 2) + pow(r[i].y - r[j].y, 2) + pow(r[i].z - r[j].z, 2)) <= distances)
			{
			//	cout << i << " and " << j << "distance" << sqrt(pow(r[i].x - r[j].x, 2) + pow(r[i].y - r[j].y, 2) + pow(r[i].z - r[j].z, 2)) << endl;
				pars.push_back(par(i,j));
			}
		}
	}
	try
	{
		while (pars.size() != 0)
		{
			bool isSorted = false;
			for (int i = 0; i < pars.size(); i++)
			{
				vector<int> intList;
				intList.push_back(pars[0].m);
				intList.push_back(pars[0].n);
				pars.erase(pars.begin());
				isSorted = false;
				while (!isSorted)
				{
					isSorted = true;
					for (int j = 0; j < pars.size(); j++)
					{
						isSorted = true;
						for (int k = 0; k < intList.size(); k++)
						{
							if (pars[j].m == intList[k])
							{
								isSorted = false;
								bool isRepeat = false;
								for (int l = 0; l < intList.size(); l++)
								{
									if (pars[j].n == intList[l])
									{
										isRepeat = true;
										break;
									}
								}
								if (!isRepeat)
								{
									intList.push_back(pars[j].n);
								}
								pars.erase(pars.begin() + j);
								j = -1;
								break;
							}
							if (pars[j].n == intList[k])
							{
								isSorted = false;
								bool isRepeat = false;
								for (int l = 0; l < intList.size(); l++)
								{
									if (pars[j].m == intList[l])
									{
										isRepeat = true;
										break;
									}
								}
								if (!isRepeat)
								{
									intList.push_back(pars[j].m);
								}
								pars.erase(pars.begin() + j);
								j = -1;
								break;
							}
						}
					}
				}
				parList.push_back(intList);
			}
		}
	}
	catch (exception e)
	{
		//cout << e.~exception << endl;
	}
	int sum = 0;
	int aggregates = parList.size();
	for (int i = 0; i < aggregates;i++)
	{
		sum += parList[i].size();
	}
	double pN1 = pN;
//	double Savg1 = (sum / aggregates);
	double Savg2 = ((pN1) / ((aggregates + (pN1 - sum))));
	cout <<"Num pieses ,which created agregats is "<< sum <<"  Num of agregats = "<<aggregates<<"Savg ="<<Savg2 <<endl;
	CSVresult << step << ";" << step*dt0 << ";" << aggregates << ";" << sum << ";" << Savg2 << ";" << endl;
}	
int nearest_point(double X0, double* array_eta_car, int n, bool isNext)
{
	double* absDifference = new double[n / 2];
	double most_absDiffSumm = 1;
	int i1 = 0;//1st point for aprox || isNext = true
	int i2 = 3;//2nd point for aprox || isNext = false
	for (int i = 0, j = 0; i < n; i += 2, j++)
	{
		absDifference[j] = abs(array_eta_car[i] - X0);
	}
	most_absDiffSumm = absDifference[0] + absDifference[1] + absDifference[2] + absDifference[3];
	for (int i = 3; i < n / 2; i++)
	{
		if ((absDifference[i] + absDifference[i - 1] + absDifference[i - 2] + absDifference[i - 3]) < most_absDiffSumm)
		{
			most_absDiffSumm = absDifference[i] + absDifference[i - 1] + absDifference[i - 2] + absDifference[i - 3];
			i1 = i-3;
			i2 = i;
			
		}
	}
	delete absDifference;
	if (isNext)
	{
		return i1;
	}
	else
	{
		return i2;
	}
}
double ff_lagrangeAprox(double X0, double* array_eta_car, int n)
{
	int i1 = 2* nearest_point(X0, array_eta_car, n, true) ;//1st point
	int i2 = 2* nearest_point(X0, array_eta_car, n, false);//2nd point
	double result = 0;	
	for (int i = i1; i < i2; i+=2)
	{
		double numerator=1;
		double denominator=1;
		for (int j = i1; j < i2; j += 2)
		{
			if (i != j)
			{
				denominator *= (array_eta_car[i] - array_eta_car[j]);
				numerator *= (X0 - array_eta_car[j]);
			}
		}
		result += array_eta_car[i+1] * (numerator / denominator);
	}
	delete array_eta_car;
	return result;
}
