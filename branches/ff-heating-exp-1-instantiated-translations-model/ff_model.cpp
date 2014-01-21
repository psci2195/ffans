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

//system inclusions
///////////////////
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>

//#include <Eigen/Dense>
//using namespace std;
//using namespace Eigen;

#include "ff_model.h"
#include "ff_model_parameters.h"
#include "ff_model_graphics.h"
#include "ff_model_io.h"

// working variables 
////////////////////
boost::mt19937 rng;
boost::normal_distribution<>* nd; // TODO need delete
boost::variate_generator<boost::mt19937&, boost::normal_distribution<> >* var_nor;
boost::uniform_01<>* ud;
boost::variate_generator<boost::mt19937&, boost::uniform_01<> >* var_uni;

ff_vect_t r[pN + 1];  //particles positions
double Rp_to_c[pN + 1];
ff_vect_t m[pN + 1];  //particles magnetic moment direction
ff_vect_t mt[pN + 1];
int m_sat[pN + 1];

ff_vect_t F[pN + 1];
//ff_vect_t P[pN + 1];
ff_vect_t tau[pN + 1]; // torque
//ff_vect_t tau_r[pN + 1]; // random torque

ff_vect_t v[pN + 1];
ff_vect_t w[pN + 1]; // angular velocity vector

ff_vect_t drt[pN + 1];
ff_vect_t drt_r[pN + 1]; // instantiated random translation
ff_vect_t dvt[pN + 1];
ff_vect_t dvt_r[pN + 1]; // instantiated random velocity
double dv_r[pN + 1]; // extra random velocity magnitude
ff_vect_t dphi[pN + 1];
ff_vect_t dphi_r[pN + 1]; // instantiated random rotation
ff_vect_t dm[pN + 1];
ff_vect_t dw[pN + 1];
double dw_r[pN + 1]; // extra random angular velocity magnitude

//ff_vect_t dir110[13];

int exist_p[pN + 1]; // particle existence; number of primary aggregate inside
//int aggregated_p[pN + 1][pN + 1]; // map of particles aggregation, in case of dW > G_barrier
double Rp[pN + 1];
double Vp[pN + 1];
double m0p[pN + 1];
double M0p[pN + 1];
double I0p[pN + 1]; // particle moment of inertia
double r0modp[pN + 1];
double Vselfp[pN + 1];
double C2[pN + 1];
double gamma_rot[pN + 1];

int time_go = 0;

double B_hyst[21];
double Mz_hyst[21];
double B_hyst_n[21];
double Mz_hyst_n[21];

double t; // time
double dT = 0;
//double dT_prev = 0;
double T_basic = 0;
double T_mean = 0;
/*double T_mean_loc = 0;
double T_mean_loc_prev = 0;
double T_mean_loc_prev_revert = 0;*/
long k_mean = 0;
double Ek = 0;
double Ek_rot = 0;
double Ek_tr = 0;
//double dW[pN + 1]; // work
double g_Bz_prev;
long step = 0;

double kB = 0;
int hyst_mode = 1;
double mz_tot;
long glob_start_step = 1;
long glob_start_step_susc = 1;
double glob_start_t;
double mz_glob = 0; // global mean average start from the hyst. point switch
ff_vect_t m_tot_glob;

//double g_theta[pN + 1], g_phi[pN + 1];

int g_hyst_start_line;
int g_hyst_up_line;
int g_hyst_bottom_line;

double BmanX = 0;
double BmanY = 0;
double BmanZ = 0;

ff_vect_t m_tot;

ff_vect_t dir110[13];

//double k_force_adapt = 0;

ff_vect_t r_brown_valid_0;

long pN_oleic_drop = 0;
long pN_oleic_drop_I = 0;
long pN_oleic_drop_II = 0;
long pN_oleic_drop_III = 0;
double d[14 + 1];

double dt_red = dt0; // reducing time indicator for the random translation

double phi_vol_fract_oleic = 0;

void ff_model_upgrade_ext_field(void)
{
    g_Bz_prev = Bext(0,0,0).z;
    switch (hyst_mode)
    {
    case 1:
        kB = 0;
        g_hyst_start_line = 1;
        g_hyst_up_line = 0;
        g_hyst_bottom_line = 0;
        break;

    case 2:
        kB = 0.25;
        g_hyst_start_line = 1;
        g_hyst_up_line = 0;
        g_hyst_bottom_line = 0;
        break;

    case 3:
        kB = 0.5;
        g_hyst_start_line = 1;
        g_hyst_up_line = 0;
        g_hyst_bottom_line = 0;
        break;

    case 4:
        kB = 0.75;
        g_hyst_start_line = 1;
        g_hyst_up_line = 0;
        g_hyst_bottom_line = 0;
        break;

    case 5:
        kB = 1;
        g_hyst_start_line = 1;
        g_hyst_up_line = 1;
        g_hyst_bottom_line = 0;
        break;

    case 6:
        kB = 0.75;
        g_hyst_start_line = 0;
        g_hyst_up_line = 1;
        g_hyst_bottom_line = 0;
        break;

    case 7:
        kB = 0.5;
        g_hyst_start_line = 0;
        g_hyst_up_line = 1;
        g_hyst_bottom_line = 0;
        break;

    case 8:
        kB = 0.25;
        g_hyst_start_line = 0;
        g_hyst_up_line = 1;
        g_hyst_bottom_line = 0;
        break;

    case 9:
        kB = 0;
        g_hyst_start_line = 0;
        g_hyst_up_line = 1;
        g_hyst_bottom_line = 0;
        break;

    case 10:
        kB = -0.25;
        g_hyst_start_line = 0;
        g_hyst_up_line = 1;
        g_hyst_bottom_line = 0;
        break;

    case 11:
        kB = -0.5;
        g_hyst_start_line = 0;
        g_hyst_up_line = 1;
        g_hyst_bottom_line = 0;
        break;

    case 12:
        kB = -0.75;
        g_hyst_start_line = 0;
        g_hyst_up_line = 1;
        g_hyst_bottom_line = 0;
        break;

    case 13:
        kB = -1;
        g_hyst_start_line = 0;
        g_hyst_up_line = 1;
        g_hyst_bottom_line = 1;
        break;

    case 14:
        kB = -0.75;
        g_hyst_start_line = 0;
        g_hyst_up_line = 0;
        g_hyst_bottom_line = 1;
        break;

    case 15:
        kB = -0.5;
        g_hyst_start_line = 0;
        g_hyst_up_line = 0;
        g_hyst_bottom_line = 1;
        break;

    case 16:
        kB = -0.25;
        g_hyst_start_line = 0;
        g_hyst_up_line = 0;
        g_hyst_bottom_line = 1;
        break;

    case 17:
        kB = 0;
        g_hyst_start_line = 0;
        g_hyst_up_line = 0;
        g_hyst_bottom_line = 1;
        break;

    case 18:
        kB = 0.25;
        g_hyst_start_line = 0;
        g_hyst_up_line = 0;
        g_hyst_bottom_line = 1;
        break;

    case 19:
        kB = 0.5;
        g_hyst_start_line = 0;
        g_hyst_up_line = 0;
        g_hyst_bottom_line = 1;
        break;

    case 20:
        kB = 0.75;
        g_hyst_start_line = 0;
        g_hyst_up_line = 0;
        g_hyst_bottom_line = 1;
        break;
    }
}

void ff_model_auto_hyst(void)
{
    int tmp;

    if ((glob_start_step == 1) && (t - glob_start_t >= 0))
        glob_start_step = step;


    if ((t - glob_start_t) >= 0.25 /*0.25 of the 1/4.0 of period*/ * 0.25 * (1 / nu_ext))
    {

        hyst_mode++;
        tmp = 0;
        if (hyst_mode == 21) {hyst_mode = 5; tmp = 1;}
        ff_model_upgrade_ext_field();
        if (tmp) {g_hyst_up_line = 1;g_hyst_start_line = 0;}

 //       ff_io_save_hyst();

        mz_glob = 0;
        glob_start_t = t;
        glob_start_step = step;

    }
}

int ff_model_check_smooth_dr(long p)
{
    double dr, rmod, dphimag;
    long ps;
    int res = 1;

    dr = sqrt(MUL(drt[p], drt[p]));
    //rmod = d[4];
	rmod = 2 * Rp[p];
	//rmod = 2 * delta;

	dphimag = sqrt(MUL(dphi[p], dphi[p]));

    if (rmod > 0)
        if ((dr / rmod > smooth_r) || (dphimag / pi > smooth_r))
        {
			//if ((dr / rmod > smooth_r)) printf("\n DEBUG SMOOTH dr = %e", dr);
			for(ps = 1; ps <= p; ps ++)
            {
                r[ps].x -= drt[ps].x;
                r[ps].y -= drt[ps].y;
                r[ps].z -= drt[ps].z;

				m[ps].x -= dm[ps].x;
                m[ps].y -= dm[ps].y;
                m[ps].z -= dm[ps].z;
            }
            dt /= 2.0;
            slow_steps += 10;
            res = 0;
            step--;

			T_mean -= T_basic;
			k_mean --;
			/*T_mean_loc -= T_basic;
			k_bm_inst --;
			if (k_bm_inst == k_bm_inst_max - 1)
			{
				if (dT < 0) k_force_adapt *= k_force_adapt_0;
				else k_force_adapt /= k_force_adapt_0;

				dT= dT_prev;

				T_mean_loc_prev = T_mean_loc_prev_revert;
			}*/
        }

        return res;
}

/*
int ff_model_check_smooth_dv(long p)
{
double dv, vmod, dm, mmag;
long ps;
int res = 1;
ff_vect_t v_prev;

v_prev.x = v[p].x - dvt[p].x;
v_prev.y = v[p].y - dvt[p].y;
v_prev.z = v[p].z - dvt[p].z;

dv = sqrt(MUL(dvt[p], dvt[p]));
vmod = sqrt(MUL(v_prev, v_prev));

//printf("\n !!! %e", vmod * dt / R0);

if (vmod > 0)
if (dv / vmod > smooth_v)
{
for(ps = 1; ps <= p; ps ++)
{
v[ps].x -= dvt[ps].x;
v[ps].y -= dvt[ps].y;
v[ps].z -= dvt[ps].z;

m[p].x = m_prev[p].x;
m[p].y = m_prev[p].y;
m[p].z = m_prev[p].z;

F[p] = F0[p];
}

for(ps = 1; ps <= pN; ps ++) // ALL particles because dr was first in the flow
{
r[ps].x -= drt[ps].x;
r[ps].y -= drt[ps].y;
r[ps].z -= drt[ps].z;
}

dt /= 2.0;
slow_steps += 10;
res = 0;
}

return res;
}
*/

/*double ff_model_distrib_f(double theta, long p)
{
double ret_f;
ff_vect_t B1; // excluding self field
double Bmag;
double Ct; 
double dtheta = pi / 10.0; // hardcode

B1.x = B[p].x - Bself * m[p].x / m0;
B1.y = B[p].y - Bself * m[p].y / m0;
B1.z = B[p].z - Bself * m[p].z / m0;

Bmag = sqrt(MUL(B1,B1));

Ct = m0 * Bmag / (kb * T);

ret_f = (Ct / (exp(Ct) - exp(-Ct))) * exp(Ct * cos(theta)) * sin(theta) * dtheta;

return ret_f;
}*/

/*double ff_model_distrib_f_r(double theta, long p)
{
double ret_f;
double G0;
double Ct; 
double dtheta = pi / 10.0; // hardcode

G0 = sqrt(MUL(F[p],F[p])) * r0mod * sqrt(dt);
Ct = G0 / (kb * T);

ret_f = (Ct / (exp(Ct) - exp(-Ct))) * exp(Ct * cos(theta)) * sin(theta) * dtheta;

return ret_f;
}*/

/*
void __deprecated__ff_model_set_rand_dir(long p)
{
    long i;
    double dtheta; // step in [R]andom long
    double R_points[10 + 1];
    long R;
    double theta, phi;
    double Fmag, tmag, bmag;
    ff_vect_t Ft, t, b;

    /*
    dtheta = pi / 10.0;
    for (i = 1; i <= 10; i++)
    {
    R_points[i] = 32768.0 * ff_model_distrib_f_r((i - 0.5) * dtheta, p);
    if (i > 1) R_points[i] += R_points[i - 1];
    //printf("\n i = %d, Rpoints = %f", i, R_points[i]);
    }

    R = rand();

    theta = 0.5 * dtheta; // default for zero
    for (i = 1; i <= 9; i++)
    if ((R > R_points[i])&&(R <= R_points[i + 1]))
    {
    theta = (i - 0.5) * dtheta;
    break;
    }
    *//*
    phi = 2 * pi * rand() / 32768.0;
    theta = pi * rand() / 32768.0;

    Fmag = sqrt(MUL(F[p],F[p]));

    if (Fmag == 0) Fmag = 0.00000000001;

    Ft = F[p];
    if ((F[p].x == 0)&&(F[p].y == 0)) {Ft.x = 1E-5; Ft.y = 1E-5;}

    t.x = - Ft.y;
    t.y =   Ft.x;
    t.z =   0;

    tmag = sqrt(MUL(t,t));

    b.x = t.y * Ft.z - t.z * Ft.y;
    b.y = t.z * Ft.x - t.x * Ft.z;
    b.z = t.x * Ft.y - t.y * Ft.x;

    bmag = sqrt(MUL(b,b));

    drt[p].x += r0modp[p] * sqrt(dt) * (Ft.x * cos(theta) / Fmag + t.x * sin(theta) * cos(phi) / tmag + b.x * sin(theta) * sin(phi) / bmag);
    drt[p].y += r0modp[p] * sqrt(dt) * (Ft.y * cos(theta) / Fmag + t.y * sin(theta) * cos(phi) / tmag + b.y * sin(theta) * sin(phi) / bmag);
    drt[p].z += r0modp[p] * sqrt(dt) * (Ft.z * cos(theta) / Fmag + t.z * sin(theta) * cos(phi) / tmag + b.z * sin(theta) * sin(phi) / bmag);
}*/

/*void ff_model_set_m(long p)
{
long i;
double dtheta; // step in [R]andom long
double R_points[10 + 1];
long R;
double theta, phi;
double Bmag, tmag, bmag;
ff_vect_t Bt, t, b;

dtheta = pi / 10.0;
for (i = 1; i <= 10; i++)
{
R_points[i] = 32768.0 * ff_model_distrib_f((i - 0.5) * dtheta, p);
if (i > 1) R_points[i] += R_points[i - 1];
//printf("\n i = %d, Rpoints = %f", i, R_points[i]);
}

R = rand();

theta = 0.5 * dtheta; // default for zero
for (i = 1; i <= 9; i++)
if ((R > R_points[i])&&(R <= R_points[i + 1]))
{
theta = (i - 0.5) * dtheta;
break;
}
phi = 2 * pi * rand() / 32768.0;

Bmag = sqrt(MUL(B[p],B[p]));

if (Bmag == 0) Bmag = 0.00000000001;

Bt = B[p];
if ((B[p].x == 0)&&(B[p].y == 0)) {Bt.x = 1E-10; Bt.y = 1E-10;}

t.x = - Bt.y;
t.y =   Bt.x;
t.z =   0;

tmag = sqrt(MUL(t,t));

b.x = t.y * Bt.z - t.z * Bt.y;
b.y = t.z * Bt.x - t.x * Bt.z;
b.z = t.x * Bt.y - t.y * Bt.x;

bmag = sqrt(MUL(b,b));

m[p].x = m0 * (Bt.x * cos(theta) / Bmag + t.x * sin(theta) * cos(phi) / tmag + b.x * sin(theta) * sin(phi) / bmag);
m[p].y = m0 * (Bt.y * cos(theta) / Bmag + t.y * sin(theta) * cos(phi) / tmag + b.y * sin(theta) * sin(phi) / bmag);
m[p].z = m0 * (Bt.z * cos(theta) / Bmag + t.z * sin(theta) * cos(phi) / tmag + b.z * sin(theta) * sin(phi) / bmag);
}*/

ff_vect_t ff_model_nonloc_torque(long p)
{
    //long p;
    register long ps;
    double mxs, mys, mzs;
    double dx, dy, dz;
    double dx_q_plus, dy_q_plus, dz_q_plus;
    double dx_q_minus, dy_q_minus, dz_q_minus, dR_q_plus, dR_q_minus;
    double eps, max_eps;

    double MUL2mod__dR5mod1, dR2__dR5mod1;
    double dR, dR2, dR5, dR5mod1, MUL2mod;

    double tBx, tBy, tBz;
    double dtBx, dtBy, dtBz;    // field of single ps-th particle
    double dtBqx, dtBqy, dtBqz;
    double tBmag, tm, tms;
    double tk;

    double esx, esy, esz;

    double mag_qs;

    long p_prev;

    double sign;
    int is_last;

    ff_vect_t ttB;
	ff_vect_t ttau;

    ttau.x = ttau.y = ttau.z = 0;
	
	//	do
    //	{
    //for(p = 1; p <= pN; p ++)
        if (exist_p[p])
        {
            ttB = Bext(r[p].x, r[p].y, r[p].z);
            tBx = ttB.x;
            tBy = ttB.y;
            tBz = ttB.z;

            for(ps = 1; ps <= pN; ps ++)
                if (exist_p[ps])
                    if (p != ps)
                    {
                        /*#ifdef SECONDARY
                        for(di = -2; di <= 2; di++)
                        for(dj = -2; dj <= 2; dj++) // including zero!
                        {
                        #endif*/
                        dx = - r[ps].x + r[p].x;// + di * Lx;
                        dy = - r[ps].y + r[p].y;// + dj * Ly;
                        dz = - r[ps].z + r[p].z;

                        dR = sqrt(dx * dx + dy * dy + dz * dz);

						dR2 = dR * dR;

                        if (1)
                        {

                            mxs = m[ps].x;
                            mys = m[ps].y;
                            mzs = m[ps].z;

                            MUL2mod = 3 * (mxs * dx + mys * dy + mzs * dz);

                            dR5 = pow(dR,5);
                            dR5mod1 = dR5 / C5;

                            MUL2mod__dR5mod1 = MUL2mod / dR5mod1;
                            dR2__dR5mod1 = dR2 / dR5mod1;

                            dtBx = dx * MUL2mod__dR5mod1 - mxs * dR2__dR5mod1;
                            dtBy = dy * MUL2mod__dR5mod1 - mys * dR2__dR5mod1;
                            dtBz = dz * MUL2mod__dR5mod1 - mzs * dR2__dR5mod1;

                            if (dtBx != dtBx) printf("\n DEBUG 12 p = %d ps = %d dtBx = %e r[p].x = %e r[ps].x = %e mxs = %e mys = %e mzs = %e", p, ps, dtBx, r[p].x, r[ps].x, mxs, mys, mzs);

                        } 
                        else
                        {
                             
                        }

                        /*tms = sqrt(MUL(m[ps],m[ps]));
                        esx = m[ps].x / tms;
                        esy = m[ps].y / tms;
                        esz = m[ps].z / tms;

                        dx_q_plus  = dx - esx * Rp[ps]; // "plus" is mag. charge
                        dx_q_minus = dx + esx * Rp[ps];

                        dy_q_plus  = dy - esy * Rp[ps];
                        dy_q_minus = dy + esy * Rp[ps];

                        dz_q_plus  = dz - esz * Rp[ps];
                        dz_q_minus = dz + esz * Rp[ps];

                        dR_q_plus = sqrt(dx_q_plus * dx_q_plus + dy_q_plus * dy_q_plus + dz_q_plus * dz_q_plus);
                        dR_q_minus = sqrt(dx_q_minus * dx_q_minus + dy_q_minus * dy_q_minus + dz_q_minus * dz_q_minus);

                        mag_qs = tms / (4.0 * Rp[ps] / 3.0);

                        dtBqx = C5 * mag_qs * (dx_q_plus / pow(dR_q_plus, 3) - dx_q_minus / pow(dR_q_minus, 3));
                        dtBqy = C5 * mag_qs * (dy_q_plus / pow(dR_q_plus, 3) - dy_q_minus / pow(dR_q_minus, 3));
                        dtBqz = C5 * mag_qs * (dz_q_plus / pow(dR_q_plus, 3) - dz_q_minus / pow(dR_q_minus, 3));

                        tBx += dtBqx * tms / m0p[ps] + dtBx * (1 - tms / m0p[ps]);
                        tBy += dtBqy * tms / m0p[ps] + dtBy * (1 - tms / m0p[ps]);
                        tBz += dtBqz * tms / m0p[ps] + dtBz * (1 - tms / m0p[ps]);*/

                        tBx += dtBx;
                        tBy += dtBy;
                        tBz += dtBz;

                        /*#ifdef SECONDARY
                        } // di and dj
                        #endif*/
                    } // if (p != ps)

                    tBmag = sqrt(tBx * tBx + tBy * tBy + tBz * tBz);
                    /*tm = sqrt(MUL(m[p],m[p]));

                    eps = acos((m[p].x * tBx + m[p].y * tBy + m[p].z * tBz) / (tBmag * tm));

                    if (p == 1) max_eps = eps;
                    else if (eps > max_eps) max_eps = eps;*/

#ifndef SECONDARY
					/*m[p].x = m0p[p] * tBx / tBmag;
                    m[p].y = m0p[p] * tBy / tBmag;
                    m[p].z = m0p[p] * tBz / tBmag;*/

					ttau.x =   m[p].y * tBz - m[p].z * tBy;
					ttau.y = - m[p].x * tBz + m[p].z * tBx;
					ttau.z =   m[p].x * tBy - m[p].y * tBx;
					
#endif

        }// if (exist_p[p])
        //	}
        //	while(max_eps > m_h_eff_tol);
	return ttau;
}

ff_vect_t ff_model_nonloc_force(long p)
{
    register long ps;
    double tFx, tFy, tFz;
    double dtFx, dtFy, dtFz;
    double dreptFx, dreptFy, dreptFz; // repulsion
    double dR, dRtemp, l, dR2, dR3, dR2mod, dR5, dR5mod, MUL1, MUL2, MUL3;
    //double R1 = 0.3 * R0;
    double Cmod;
    //double Cmod1 = Ch * m0 * m0;
    //double Cmod1_repulsion = 5 * m0 * m0;
    double dx, dy, dz;
    double tk;
    double sec_pow;
    ff_vect_t ttF;
    
    double MUL1__dR5mod, MUL2__dR5mod, MUL3__dR5mod, MUL1_MUL2__dR2mod__dR5mod;

    double mx, my, mz;
    double mxs, mys, mzs;
    long p_prev;

    int is_last;
    double sign, tms, mag_qs;

    //ff_vect_t ttau;

	double dd, tt;

    //long di, dj;

    tFx = 0;
    tFy = 0;
    tFz = 0;

    for(ps = 1; ps <= pN; ps ++)
        if (exist_p[ps])
            /*#ifdef SECONDARY
            for(di = -2; di <= 2; di++)
            for(dj = -2; dj <= 2; dj++) // including zero!
            {
            #endif*/
            if (p != ps)
            {

                dx = r[ps].x - r[p].x;// + di * Lx;
                dy = r[ps].y - r[p].y;// + dj * Ly;
                dz = r[ps].z - r[p].z;

                mx = m[p].x;
                my = m[p].y;
                mz = m[p].z;

                mxs = m[ps].x;
                mys = m[ps].y;
                mzs = m[ps].z;

                //dipole-dipole
                dR = sqrt(dR2 = dx * dx + dy * dy + dz * dz);
                dR3 = dR2 * dR;

                //if (dR == 0) printf ("\n \n \n dR == 0 !!!");

                // Non local force and self-confid. magnetic field
                dR2mod = dR2 / 5.0; //modified
                dR5mod = (dR5 = pow(dR,5)) / C1; //modified

                MUL2 = mxs * dx + mys * dy + mzs * dz;

                //#ifndef SECONDARY
                //if (p < ps)  !!!!!
                //{

                {
                    //#endif
                    MUL1 = mx * dx + my * dy + mz * dz;
                    MUL3 = mx * mxs + my * mys + mz * mzs;

                    MUL1__dR5mod = MUL1 / dR5mod;
                    MUL2__dR5mod = MUL2 / dR5mod;
                    MUL3__dR5mod = MUL3 / dR5mod;
                    MUL1_MUL2__dR2mod__dR5mod = MUL1 * MUL2 / (dR2mod * dR5mod);

                    dtFx = -(MUL1__dR5mod * mxs + MUL2__dR5mod * mx
                        + MUL3__dR5mod * dx - MUL1_MUL2__dR2mod__dR5mod * dx);

                    dtFy = -(MUL1__dR5mod * mys + MUL2__dR5mod * my
                        + MUL3__dR5mod * dy - MUL1_MUL2__dR2mod__dR5mod * dy);

                    dtFz = -(MUL1__dR5mod * mzs + MUL2__dR5mod * mz
                        + MUL3__dR5mod * dz - MUL1_MUL2__dR2mod__dR5mod * dz);

                    //if (p == 50) printf("\n DEBUG 7 ps = %d F.x = %e", ps, dtFx);
                    //if (p == 50) printf("\n DEBUG 7 ps = %d m[p].x = %e", ps, m[p].x);
                    //if (p == 50) printf("\n DEBUG 7 ps = %d m[ps].x = %e", ps, m[ps].x);

                } 

                if (dR >= Rp[p] + Rp[ps])
				{
					tFx += dtFx;
					tFy += dtFy;
					tFz += dtFz;
				}

                /*
                if (dR <= 5 * R0) // exp phenomenology
                {
                tFx += - (EPS * dx / dR) * (exp(-(dR - 2 * R00) / ro1) / ro1 - exp(-(dR - 2 * R00) / ro2) / ro2);
                tFy += - (EPS * dy / dR) * (exp(-(dR - 2 * R00) / ro1) / ro1 - exp(-(dR - 2 * R00) / ro2) / ro2);
                tFz += - (EPS * dz / dR) * (exp(-(dR - 2 * R00) / ro1) / ro1 - exp(-(dR - 2 * R00) / ro2) / ro2);
                }
                */

#ifndef SECONDARY
				if (dR <= Rp[p] + Rp[ps]) //soft sphere condition
				{
					/*Cmod = 1 * m0p[p] * m0p[ps] * (C1 / dR5);

					tFx += -dx * Cmod;
                    tFy += -dy * Cmod;
                    tFz += -dz * Cmod;*/

					/*if ((dW[p] > G_barrier) && (!aggregated_p[p][ps]))
					{
						aggregated_p[p][ps] = 1;
						//printf("\n aggregated_p");
					}*/

					dRtemp = Rp[p] + Rp[ps];
					dd = Rp[p] + Rp[ps];
					l = 2 * (dRtemp - dd) / dd;
					tt = 2 * delta / dd;

					tFx += - Ch_ss * (dx / dRtemp) * (2 * pow(dd, 2) * kb * T * N_oa * pi * log((tt + 1) / (l / 2 + 1)) / tt);
					tFy += - Ch_ss * (dy / dRtemp) * (2 * pow(dd, 2) * kb * T * N_oa * pi * log((tt + 1) / (l / 2 + 1)) / tt);
					tFz += - Ch_ss * (dz / dRtemp) * (2 * pow(dd, 2) * kb * T * N_oa * pi * log((tt + 1) / (l / 2 + 1)) / tt);
				}

                // Entropic repulsion
				if ((dR > Rp[p] + Rp[ps]) && (dR <= Rp[p] + Rp[ps] + 2 * delta)) 
                {
                    /*if (Ch > 5)
                        Cmod = Ch * m0p[p] * m0p[ps] * (C1 / dR5);
                    else
                        Cmod = 5 * m0p[p] * m0p[ps] * (C1 / dR5);*/

					/*tFx += -dx * Cmod;
                    tFy += -dy * Cmod;
                    tFz += -dz * Cmod;*/

					dd = Rp[p] + Rp[ps];
					l = 2 * (dR - dd) / dd;
					tt = 2 * delta / dd;

					tFx += - (dx / dR) * (2 * pow(dd, 2) * kb * T * N_oa * pi * log((tt + 1) / (l / 2 + 1)) / tt);
					tFy += - (dy / dR) * (2 * pow(dd, 2) * kb * T * N_oa * pi * log((tt + 1) / (l / 2 + 1)) / tt);
					tFz += - (dz / dR) * (2 * pow(dd, 2) * kb * T * N_oa * pi * log((tt + 1) / (l / 2 + 1)) / tt);
                }
#endif

#ifndef SECONDARY
                // attraction
                //if ((dR > Rp[p] + Rp[ps] )&&(dR < 3 * (Rp[p] + Rp[ps]) / 2.0 )) // the Heaviside step function  and dR5 dependence finally is similar to the well-known exp. phenomenology
				//if (dR > Rp[p] + Rp[ps] + 2 * smooth_r * delta)
				if (dR > (Rp[p] + Rp[ps]) * (1 + smooth_r))
                {
                    /*Cmod = Ch * m0p[p] * m0p[ps] * (C1 / dR5);

                    tFx += dx * Cmod;
                    tFy += dy * Cmod;
                    tFz += dz * Cmod;*/

					tFx += - (dx / dR) * (- 64 * A_H * dR * pow(Rp[p] * Rp[ps], 3) / (6 * pow(dR * dR - pow(Rp[p] - Rp[ps], 2), 2) * pow(dR * dR - pow(Rp[p] + Rp[ps], 2), 2)));
					tFy += - (dy / dR) * (- 64 * A_H * dR * pow(Rp[p] * Rp[ps], 3) / (6 * pow(dR * dR - pow(Rp[p] - Rp[ps], 2), 2) * pow(dR * dR - pow(Rp[p] + Rp[ps], 2), 2)));
					tFz += - (dz / dR) * (- 64 * A_H * dR * pow(Rp[p] * Rp[ps], 3) / (6 * pow(dR * dR - pow(Rp[p] - Rp[ps], 2), 2) * pow(dR * dR - pow(Rp[p] + Rp[ps], 2), 2)));

					/*if (aggregated_p[p][ps])
					{
						Cmod = 5 * m0p[p] * m0p[ps] * (C1 / dR5);

						tFx += dx * Cmod;
						tFy += dy * Cmod;
						tFz += dz * Cmod;
					}*/
                }
#endif

                /*Fnonloc[p][ps].x = tFx;
                Fnonloc[p][ps].y = tFy;
                Fnonloc[p][ps].z = tFz;*/

                //#ifndef SECONDARY
                //} // end of if p < ps
                /*else
                {
                tFx -= Fnonloc[ps][p].x; // third Newton law
                tFy -= Fnonloc[ps][p].y;
                tFz -= Fnonloc[ps][p].z;
                }*/ // end of if p > ps
                //#endif
                //tBx += dx * MUL2mod__dR5mod1 - mxs * dR2__dR5mod1;
                //tBy += dy * MUL2mod__dR5mod1 - mys * dR2__dR5mod1;
                //tBz += dz * MUL2mod__dR5mod1 - mzs * dR2__dR5mod1;
                /*#ifdef SECONDARY
                }
                #endif*/
            } // end of the particles loop


            //tBmag = sqrt(tBx * tBx + tBy * tBy + tBz * tBz);

            // here we should add self magnetic field inside the particle
            //B[p].x = tBx + Bself * m[p].x / m0;
            //B[p].y = tBy + Bself * m[p].y / m0;
            //B[p].z = tBz + Bself * m[p].z / m0;

            /*m_prev[p].x = m[p].x;
            m_prev[p].y = m[p].y;
            m_prev[p].z = m[p].z;*/

            //m[p].x = m0 * tBx / tBmag;
            //m[p].y = m0 * tBy / tBmag;
            //m[p].z = m0 * tBz / tBmag;
            //ff_model_set_m(p);

            // force
            ttF.x = tFx;
            ttF.y = tFy;
            ttF.z = tFz;

            //if (ttF.x != ttF.x) printf("\n DEBUG 10 p = %d F.x = %e", p, ttF.x);
            //if (ttF.y != ttF.y) printf("\n DEBUG 10 p = %d F.y = %e", p, ttF.y);
            //if (ttF.z != ttF.z) printf("\n DEBUG 10 p = %d F.z = %e", p, ttF.z);

            return ttF;
}

double ff_model_G_steric(long p, long ps)
{
	double G_steric = 0;
	double z = 0;
	double dx = 0; 
	double dy = 0; 
	double dz = 0;
	double dR = 0;
	double dRmax = 0;

	dx = r[p].x - r[ps].x;
	dy = r[p].y - r[ps].y;
	dz = r[p].z - r[ps].z;
	dR = sqrt(dx * dx + dy * dy + dz * dz);

	dRmax = Rp[ps] + Rp[p] + 2 * delta;

	// Entropic repulsion
	if (dR <= dRmax)
		G_steric = (pi * kb * pow(dR - (2 * delta + Rp[ps] + Rp[p]), 2) * ((Rp[ps] + Rp[p]) * 
			       (dR + delta) - (pow(Rp[ps], 3) + pow(Rp[p], 3)) / (Rp[p] + Rp[ps])) * N_oa * T) / ( 6 * delta * dR);
	else G_steric = 0;

	return G_steric;
}

ff_vect_t ff_model_force(long p)
{
    ff_vect_t tF;
    ff_vect_t tddF;

    tF.x = tF.y = tF.z = 0;

    // non-local
    tddF = ff_model_nonloc_force(p);
    tF.x += tddF.x;
    tF.y += tddF.y;
    tF.z += tddF.z;

    // Gravitation
    //tF.z += - C3;

    // Buoyancy force
    //tF.z +=   C6;

	// oleic droplet surface tension force
	if (fabs(Rp_to_c[p] - R_oleic) < Rp[p])
	{
		tF.x += - sigma_sf_nano * 2 * pi * Rp[p] * r[p].x / Rp_to_c[p];
		tF.y += - sigma_sf_nano * 2 * pi * Rp[p] * r[p].y / Rp_to_c[p];
		tF.z += - sigma_sf_nano * 2 * pi * Rp[p] * r[p].z / Rp_to_c[p];
	}

    /*tF.x += P[p].x;
    tF.y += P[p].y;
    tF.z += P[p].z;*/

    return tF;
}

ff_vect_t ff_model_torque(long p)
{
	ff_vect_t ttau;
    ff_vect_t dtau;

    ttau.x = ttau.y = ttau.z = 0;

    // non-local
    dtau = ff_model_nonloc_torque(p);
    ttau.x += dtau.x;
    ttau.y += dtau.y;
    ttau.z += dtau.z;

    // Gravitation
    ttau.z += 0;

    // Buoyancy
    ttau.z += 0;

	//printf("\n %e", ttau.x);

    /*ttau.x += tau_r[p].x;
    ttau.y += tau_r[p].y;
    ttau.z += tau_r[p].z;*/

    return ttau;
}

void ff_model_next_step(void)
{ 

    ff_vect_t f, ttau;
    double I;
    long p;
    int chk;
    long mz_tot_n;
    ff_vect_t r0;
	//double C2, gamma_rot;
	double M0, I0;
	ff_vect_t ex, ey, ez; // basis of rotation around ez = w
	double tmmag;
	double theta_0_r, phi_0_r;
	double Mtot;
	
    Ek = Ek_tr = Ek_rot = 0;
    mz_tot = 0;
    m_tot.x = m_tot.y = m_tot.z = 0;
    mz_tot_n = 0;
	pN_oleic_drop = 0;
	pN_oleic_drop_I = pN_oleic_drop_II = pN_oleic_drop_III = 0;
	phi_vol_fract_oleic = 0;

	if (time_go)
    {
        step++;
		//printf("\n ============================", step);
        //printf("\n Step %d", step);

        //ff_model_m_setting();

		//ff_model_update_dT();

        for (p = 1; p <= pN; p++)
            if (exist_p[p])
            {
                //Rp_to_c[p] = sqrt(MUL(r[p], r[p]));
								
				/*if (k_bm_inst == 1)*/
				//ff_model_inst_random_trans_update(p);
				
				f = ff_model_force(p);
				ttau = ff_model_torque(p);
                F[p].x = f.x; F[p].y = f.y; F[p].z = f.z;
				tau[p].x = ttau.x; tau[p].y = ttau.y; tau[p].z = ttau.z;

                /*DEBUG*/ if (f.x != f.x) printf("\n DEBUG 1 p = %d f.x = %e", p, f.x);
                /*DEBUG*/ if (f.y != f.y) printf("\n DEBUG 1 p = %d f.y = %e", p, f.y);
                /*DEBUG*/ if (f.z != f.z) printf("\n DEBUG 1 p = %d f.z = %e", p, f.z);

				/*DEBUG*/ //if (tau[p].x != tau[p].x) printf("\n DEBUG 1 p = %d ttau.x = %e", p, tau[p].x);
                /*DEBUG*/ //if (ttau.y != ttau.y) printf("\n DEBUG 1 p = %d ttau.y = %e", p, ttau.y);
                /*DEBUG*/ //if (tau[p].z != tau[p].z) printf("\n DEBUG 1 p = %d ttau.z = %e", p, tau[p].z);

                /*DEBUG*/ if (v[p].x != v[p].x) printf("\n DEBUG 1.1 p = %d r[p].x = %e v[p].x = %e", p, r[p].x, v[p].x);
                /*DEBUG*/ if (v[p].y != v[p].y) printf("\n DEBUG 1.1 p = %d r[p].y = %e v[p].y = %e", p, r[p].y, v[p].y);
                /*DEBUG*/ if (v[p].z != v[p].z) printf("\n DEBUG 1.1 p = %d r[p].z = %e v[p].z = %e", p, r[p].z, v[p].z);

				/*DEBUG*/ if (w[p].x != w[p].x) printf("\n DEBUG 1.2 p = %d", p);
                /*DEBUG*/ if (w[p].y != w[p].y) printf("\n DEBUG 1.2 p = %d", p);
                /*DEBUG*/ if (w[p].z != w[p].z) printf("\n DEBUG 1.2 p = %d", p);
            }

            for (p = 1; p <= pN; p++)
                if (exist_p[p])
                {
                    if (Rp_to_c[p] > R_oleic)
					{
						C2[p] = 6 * pi * eta * Rp[p];
						gamma_rot[p] = 8 * pi * eta * pow(Rp[p], 3);
					}
					else
					{
						C2[p] = 6 * pi * eta_oleic * Rp[p];
						gamma_rot[p] = 8 * pi * eta_oleic * pow(Rp[p], 3);
					}

					M0 = M0p[p];
					I0 = I0p[p];

					drt[p].x = F[p].x * dt / C2[p] +		 
                        (v[p].x - F[p].x / C2[p]) * (1 - exp(- C2[p] * dt / M0)) * M0 / C2[p]
						+ drt_r[p].x * dt / dt0;

                    drt[p].y = F[p].y * dt / C2[p] +		 
                        (v[p].y - F[p].y / C2[p]) * (1 - exp(- C2[p] * dt / M0)) * M0 / C2[p]
						+ drt_r[p].y * dt / dt0;

                    drt[p].z = F[p].z * dt / C2[p] +		 
                        (v[p].z - F[p].z / C2[p]) * (1 - exp(- C2[p] * dt / M0)) * M0 / C2[p]
						+ drt_r[p].z * dt / dt0;

                    //if (brownian_shifts) ff_model_set_rand_dir(p);

					dphi[p].x = tau[p].x * dt / gamma_rot[p] +		 
                    (w[p].x - tau[p].x / gamma_rot[p]) * (1 - exp(- gamma_rot[p] * dt / I0)) * I0 / gamma_rot[p]
					+ dphi_r[p].x * dt / dt0;

					dphi[p].y = tau[p].y * dt / gamma_rot[p] +		 
                    (w[p].y - tau[p].y / gamma_rot[p]) * (1 - exp(- gamma_rot[p] * dt / I0)) * I0 / gamma_rot[p]
					+ dphi_r[p].y * dt / dt0;

					dphi[p].z = tau[p].z * dt / gamma_rot[p] +		 
                    (w[p].z - tau[p].z / gamma_rot[p]) * (1 - exp(- gamma_rot[p] * dt / I0)) * I0 / gamma_rot[p]
					+ dphi_r[p].z * dt / dt0;

					/*DEBUG*/ if (dphi[p].x != dphi[p].x) printf("\n DEBUG 1 p = %d dphi[p].x = %e", p, dphi[p].x);
                    /*DEBUG*/ if (dphi[p].y != dphi[p].y) printf("\n DEBUG 1 p = %d dphi[p].y = %e", p, dphi[p].y);
                    /*DEBUG*/ if (dphi[p].z != dphi[p].z) printf("\n DEBUG 1 p = %d dphi[p].z = %e", p, dphi[p].z);

					//ff_model_check_overlapp(p); // hard sphere condition

					//printf("\n %e", dphi[p].y);

                    r[p].x += drt[p].x;
                    r[p].y += drt[p].y;
                    r[p].z += drt[p].z;

					dm[p].x = dm[p].y = dm[p].z = 0;

					mt[p] = m[p];
					// turn around ex
					mt[p].y = mt[p].y * cos(dphi[p].x) - mt[p].z * sin(dphi[p].x);
					mt[p].z = mt[p].y * sin(dphi[p].x) + mt[p].z * cos(dphi[p].x);
					// turn around ey
					mt[p].z = mt[p].z * cos(dphi[p].y) - mt[p].x * sin(dphi[p].y);
					mt[p].x = mt[p].z * sin(dphi[p].y) + mt[p].x * cos(dphi[p].y);
					// turn around ez
					mt[p].x = mt[p].x * cos(dphi[p].z) - mt[p].y * sin(dphi[p].z);
					mt[p].y = mt[p].x * sin(dphi[p].z) + mt[p].y * cos(dphi[p].z);

					dm[p].x = mt[p].x - m[p].x;
					dm[p].y = mt[p].y - m[p].y;
					dm[p].z = mt[p].z - m[p].z;

					m[p].x += dm[p].x;
					m[p].y += dm[p].y;
					m[p].z += dm[p].z;

					tmmag = sqrt(MUL(m[p], m[p]));

					m[p].x *= m0p[p] / tmmag;
					m[p].y *= m0p[p] / tmmag;
					m[p].z *= m0p[p] / tmmag;

					//if (m0p[p] == 0) printf("\n !!!");

					//printf("\n %e", sqrt(MUL(m[p],m[p])) / m0p[p]);

                    if (r[p].x != r[p].x) printf("\n DEBUG 2 p = %d r[p].x = %e v[p].x = %e", p, r[p].x, v[p].x);
                    if (r[p].y != r[p].y) printf("\n DEBUG 2 p = %d r[p].y = %e v[p].y = %e", p, r[p].y, v[p].y);
                    if (r[p].z != r[p].z) printf("\n DEBUG 2 p = %d r[p].z = %e v[p].z = %e", p, r[p].z, v[p].z);

                    chk = ff_model_check_smooth_dr(p);
					if ( chk == 0)
						{
							//k_bm_inst = 1;
							goto t_end;
						}

					ff_model_check_walls(p);
					
					//k_bm_inst++;
					//if (k_bm_inst == k_bm_inst_max) k_bm_inst = 1;
					/*if (k_bm_inst == k_bm_inst_max)
					{
						k_bm_inst = 1;
						T_mean_loc = 0;
					}*/
                } // end of loop for dr

                r0.x = r0.y = r0.z = 0;
				Mtot = 0;
				
                for (p = 1; p <= pN; p++)
                    if (exist_p[p])
                    {
					    /*if (Rp_to_c[p] > R_oleic)
					    {
					    	C2 = 6 * pi * eta * Rp[p];
					    	gamma_rot = 8 * pi * eta * pow(Rp[p], 3);
					    }
					    else
					    {
					    	C2 = 6 * pi * eta_oleic * Rp[p];
					    	gamma_rot = 8 * pi * eta_oleic * pow(Rp[p], 3);
					    }*/
					    
						M0 = M0p[p];
					    I0 = I0p[p];

						// C2 is a friction
                        dvt[p].x = (F[p].x / C2[p]) + (v[p].x - F[p].x / C2[p]) * exp(- C2[p] * dt / M0) - v[p].x;
                        dvt[p].y = (F[p].y / C2[p]) + (v[p].y - F[p].y / C2[p]) * exp(- C2[p] * dt / M0) - v[p].y;
                        dvt[p].z = (F[p].z / C2[p]) + (v[p].z - F[p].z / C2[p]) * exp(- C2[p] * dt / M0) - v[p].z;

                        v[p].x += dvt[p].x;
                        v[p].y += dvt[p].y;
                        v[p].z += dvt[p].z;

						w[p].x = (tau[p].x / gamma_rot[p]) + (w[p].x - tau[p].x / gamma_rot[p]) * exp(- gamma_rot[p] * dt / I0);
						w[p].y = (tau[p].y / gamma_rot[p]) + (w[p].y - tau[p].y / gamma_rot[p]) * exp(- gamma_rot[p] * dt / I0);
						w[p].z = (tau[p].z / gamma_rot[p]) + (w[p].z - tau[p].z / gamma_rot[p]) * exp(- gamma_rot[p] * dt / I0);

                        if (v[p].x != v[p].x) printf("\n DEBUG 3 p = %d v[p].x = %e", p, v[p].x);
                        if (v[p].y != v[p].y) printf("\n DEBUG 3 p = %d v[p].y = %e", p, v[p].y);
                        if (v[p].z != v[p].z) printf("\n DEBUG 3 p = %d v[p].z = %e", p, v[p].z);

                        //chk = ff_model_check_smooth_dv(p);
                        //if ( chk == 0) goto t_end;

                        r0.x += M0p[p] * r[p].x;
                        r0.y += M0p[p] * r[p].y;
                        r0.z += M0p[p] * r[p].z;
						Mtot += M0p[p];

                        mz_tot += m[p].z;
                        m_tot.x += m[p].x;
                        m_tot.y += m[p].y;
                        m_tot.z += m[p].z;
                        mz_tot_n++;

						//if (p == 1) ff_model_brownian_validation(p);

						/*dW[p] = MUL(F[p], drt[p]) + MUL(tau[p], dphi[p]) 
							   - C2[p] * ((v[p].x - dvt[p].x / 2) * drt[p].x + (v[p].y - dvt[p].y / 2) * drt[p].y + (v[p].z - dvt[p].z / 2) * drt[p].z)
							   - gamma_rot[p] * ((w[p].x - dw[p].x / 2) * dphi[p].x + (w[p].y - dw[p].y / 2) * dphi[p].y + (w[p].z - dw[p].z / 2) * dphi[p].z);*/
                    } // end of loop for dv

					//printf("\n DEBUG 5 r.x = %e v.x = %e", r[50].x, v[50].x);

                    // TODO: need the number of existing (exist_p) particles
                    r0.x /= Mtot;
                    r0.y /= Mtot;
                    r0.z /= Mtot;

                    I = 0;
                    for (p = 1; p <= pN; p++)
                        I += M0p[p] * (pow(r[p].x - r0.x, 2)
                        + pow(r[p].y - r0.y, 2)
                        + pow(r[p].z - r0.z, 2));

                    mz_tot *= pN / mz_tot_n; // in loop is interrupted then needs to increase
                    mz_glob += mz_tot;

                    m_tot_glob.x += m_tot.x;
                    m_tot_glob.y += m_tot.y;
                    m_tot_glob.z += m_tot.z;

                    t += dt;
					dt_red -= dt;					
					
					for (p = 1; p <= pN; p++) if (exist_p[p])
					{
						Ek_tr  += M0p[p] * MUL(v[p],v[p]) / 2.0;
						Ek_rot += I0p[p] * MUL(w[p],w[p]) / 2.0;
					};

					Ek = Ek_tr + Ek_rot;
					ff_model_update_dT();

					for (p = 1; p <= pN; p++)
					{
						Rp_to_c[p] = sqrt(MUL(r[p], r[p]));
						ff_model_update_conc_in_oleic(p);
						
						if (dt_red <= 0)
						{
							ff_model_inst_random_trans_update(p);

							/*dv_r[p] = sqrt(3 * kb * dT / M0p[p]);
							dw_r[p] = sqrt(3 * kb * dT / I0p[p]);
							
							theta_0_r = (*var_uni)() * pi;   // rotation vector random direction
							phi_0_r = (*var_uni)() * 2 * pi;
							v[p].x += dv_r[p] * sin(theta_0_r) * cos(phi_0_r);
							v[p].y += dv_r[p] * sin(theta_0_r) * sin(phi_0_r);
							v[p].z += dv_r[p] * cos(theta_0_r);

							theta_0_r = (*var_uni)() * pi;  
							phi_0_r = (*var_uni)() * 2 * pi;
							w[p].x += dw_r[p] * sin(theta_0_r) * cos(phi_0_r);
							w[p].y += dw_r[p] * sin(theta_0_r) * sin(phi_0_r);
							w[p].z += dw_r[p] * cos(theta_0_r);*/
						}
					}

					phi_vol_fract_oleic /= (4 / 3.0) * pi * pow(R_oleic, 3);
					phi_vol_fract_oleic *= 100;

					if (dt_red <= 0) dt_red = dt0;

					if (auto_reversal) ff_model_auto_hyst();
                    if (step % 10000 == 0) ff_io_autosave();
					else if ((step < 10000) && (step % 1000 == 0)) ff_io_autosave();
					else if ((step < 1000) && (step % 100 == 0)) ff_io_autosave();

                    if (setting_plot)
                    {
						if (step % 1000 == 0) ff_io_save_setting(m_tot,I);
						else if ((step < 1000) && (step % 100 == 0)) ff_io_save_setting(m_tot,I);
						else if ((step < 100) && (step % 10 == 0)) ff_io_save_setting(m_tot,I);
                    }

                    // end for case of interrupted kinetic loops
t_end:

                    if (slow_steps > 0) slow_steps--;
                    if (slow_steps%10 == 1) dt *= 2;

    } // time_go

    ff_mgr_show_next_step();
}

int ff_model_check_walls(long p)
{
    int res = 0;
    double dr;

    //walls
    // Oz
    if (r[p].z < -Lz / 2.0)
    {
        r[p].z = -Lz / 2.0;
        v[p].z *= -1;
        res = 1;
    }

    if (r[p].z >  Lz / 2.0)
    {
        r[p].z =  Lz / 2.0;
        v[p].z *= -1;
        res = 1;
    }

    // Ox
    if (r[p].x < -Lx / 2.0)
    {
        r[p].x = -Lx / 2.0;
        v[p].x *= -1;
        res = 1;
    }

    if (r[p].x >  Lx / 2.0)
    {
        r[p].x =  Lx / 2.0;
        v[p].x *= -1;
        res = 1;
    }

    // Oy
    if (r[p].y < -Ly / 2.0)
    {
        r[p].y = -Ly / 2.0;
        v[p].y *= -1;
        res = 1;
    }

    if (r[p].y >  Ly / 2.0)
    {
        r[p].y =  Ly / 2.0;
        v[p].y *= -1;
        res = 1;
    }

    return res;
}

/*void ff_model_check_overlapp(long p)
{
long ps;
double dR, dx, dy, dz;
double dr;

//for (p = 1; p <= pN; p++)
for(ps = 1; ps <= pN; ps ++)
if (p != ps)
{
dx = r[ps].x - r[p].x - drt[p].x;
dy = r[ps].y - r[p].y - drt[p].y;
dz = r[ps].z - r[p].z - drt[p].z;

dR = sqrt(dx * dx + dy * dy + dz * dz);

if (dR <= Rp[p] + Rp[ps])
{
//r[ps].x -= drt[ps].x;
//r[ps].y -= drt[ps].y;
//r[ps].z -= drt[ps].z;

//v[ps].x -= dvt[ps].x;
//v[ps].y -= dvt[ps].y;
//v[ps].z -= dvt[ps].z;
	dr = Rp[p] + Rp[ps] - dR;
	drt[p].x += - (dx / dR) * dr - delta * 0.1;
	drt[p].y += - (dy / dR) * dr - delta * 0.1;
	drt[p].z += - (dz / dR) * dr - delta * 0.1;
}
}
}*/

// External heterogeneous field. Source is shifted on X axis. It is normilized on ext. magnetic moment directed along X axis.
ff_vect_t B_het(double x1, double y1, double z1, int what_shift)
{
    double x0, y0, z0;
    double x, y, z;
    double r, mr;
    ff_vect_t B;

    x0 = - Lx * (what_shift == 1);
    y0 = - Ly * (what_shift == 2);
    z0 = - Lz * (what_shift == 3);

    x = x1 - x0;
    y = y1 - y0;
    z = z1 - z0;

    if (what_shift == 1) mr = x;
    if (what_shift == 2) mr = y;
    if (what_shift == 3) mr = z;

    r = sqrt(x * x + y * y + z * z);

    B.x = C5 * (3 * x * mr / pow(r, 5) - (what_shift == 1) / pow(r, 3));
    B.y = C5 * (3 * y * mr / pow(r, 5) - (what_shift == 2) / pow(r, 3));
    B.z = C5 * (3 * z * mr / pow(r, 5) - (what_shift == 3) / pow(r, 3));

    return B;
}

ff_vect_t Bext(double x, double y, double z)
{
    ff_vect_t tBext;
    double normX, normY, normZ;

    if (!manual_field_control)
    {
        tBext.x = tBext.y = 1E-50;
        tBext.z = B0 * sin(kB * pi / 2.0);
    }
    else
    {
        if (ext_field_is_homo)
        {
            tBext.x = BmanX /*Oe*/ * (1.0 / (4.0 * pi * 1E-3)) * mu0; // Tesla
            tBext.y = BmanY /*Oe*/ * (1.0 / (4.0 * pi * 1E-3)) * mu0; // Tesla
            tBext.z = BmanZ /*Oe*/ * (1.0 / (4.0 * pi * 1E-3)) * mu0; // Tesla
        }
        else
        {
            normX = (BmanX /*Oe*/ * (1.0 / (4.0 * pi * 1E-3)) * mu0 / B_het(0, 0, 0, 1).x);
            normY = (BmanY /*Oe*/ * (1.0 / (4.0 * pi * 1E-3)) * mu0 / B_het(0, 0, 0, 2).y);
            normZ = (BmanZ /*Oe*/ * (1.0 / (4.0 * pi * 1E-3)) * mu0 / B_het(0, 0, 0, 3).z);

            tBext.x = normX * B_het(x, y, z, 1).x + normY * B_het(x, y, z, 2).x + normZ * B_het(x, y, z, 3).x;
            tBext.y = normX * B_het(x, y, z, 1).y + normY * B_het(x, y, z, 2).y + normZ * B_het(x, y, z, 3).y;
            tBext.z = normX * B_het(x, y, z, 1).z + normY * B_het(x, y, z, 2).z + normZ * B_het(x, y, z, 3).z;
        }
    }

    return tBext;
}

/*void ff_model_init_sediment(void)
{
long i, j, k;
ff_vect_t r0, r1, rb; //rb - basic; we build sphere around it
long n;
double a = (2 + 0.1) * R0;
long p, pb;
int res;
long pt;
int jdir;

rb.x = rb.y = 0;
//rb.z = -0.99 * Lz / 2.0;
rb.z = 0;

jdir = 0;

p = 1;
pb = p;

r[p].x = rb.x;
r[p].y = rb.y;
r[p].z = rb.z;

p++;

while (p <= pN)
{
jdir++;

r1.x = rb.x + a * dir110[jdir].x / sqrt(2.0);
r1.y = rb.y + a * dir110[jdir].y / sqrt(2.0);
r1.z = rb.z + a * dir110[jdir].z / sqrt(2.0);

if ((fabs(r1.x) > Lx / 2.0)||(fabs(r1.y) > Ly / 2.0)||(fabs(r1.z) > Lz / 2.0))
{res = 0; goto m1;}

res = 1;
for(pt = 1; pt < p; pt++)
{
if ((fabs(r1.x - r[pt].x) > 0.1 * R0)||(fabs(r1.y - r[pt].y) > 0.1 * R0)||(fabs(r1.z - r[pt].z) > 0.1 * R0)) res = 1;
else {res = 0; goto m1;}
}
m1:

if (res == 1)
{
r[p].x = r1.x;
r[p].y = r1.y;
r[p].z = r1.z;

p++;
}
if ( jdir == 12)
{
jdir = 0;

pb++;

rb.x = r[pb].x;
rb.y = r[pb].y;
rb.z = r[pb].z;
}
} // p for
}

void ff_model_dir_init(void)
{

long i;

i = 1;
dir110[i].x = 1; dir110[i].y = 1; dir110[i].z = 0;
i++;
dir110[i].x = 1; dir110[i].y = 0; dir110[i].z = 1;
i++;
dir110[i].x = 0; dir110[i].y = 1; dir110[i].z = 1;
i++;

dir110[i].x = -1; dir110[i].y =  1; dir110[i].z = 0;
i++;
dir110[i].x = -1; dir110[i].y =  0; dir110[i].z = 1;
i++;
dir110[i].x =  0; dir110[i].y = -1; dir110[i].z = 1;
i++;


dir110[i].x = -1; dir110[i].y = -1; dir110[i].z = 0;
i++;
dir110[i].x = -1; dir110[i].y = 0; dir110[i].z = -1;
i++;
dir110[i].x = 0; dir110[i].y = -1; dir110[i].z = -1;
i++;

dir110[i].x = 1; dir110[i].y =  -1; dir110[i].z = 0;
i++;
dir110[i].x = 1; dir110[i].y =  0; dir110[i].z = -1;
i++;
dir110[i].x =  0; dir110[i].y = 1; dir110[i].z = -1;

}*/

void ff_model_init(void)
{
    long p, tp;
    double theta, phi;
    ff_vect_t dr;
    double dR;
    double sigma;
    long i,j;

    // Brownian motion -  parameters
    ///////////////////////////////////////////////////
	
	//k_force_adapt = 1;
	//k_force_adapt = k_force_adapt_0 / sqrt(1E-9); //sqrt(1E-9) is selected regular dt
	
	// Dimensionless variance (sigma^2) of the random displacement along the single axis e_x
    sigma = 1;

    srand( (unsigned)time( NULL ) );
    rng.seed(static_cast<unsigned int>(std::time(0)));

    //boost::normal_distribution<> nd(0.0, 1.0);
    nd = new boost::normal_distribution<> (0.0, sigma);
	ud = new boost::uniform_01<> ();

    //boost::variate_generator<boost::mt19937&, boost::normal_distribution<> > var_nor(rng, *nd);
    var_nor = new boost::variate_generator<boost::mt19937&, boost::normal_distribution<> > (rng, *nd);
	var_uni = new boost::variate_generator<boost::mt19937&, boost::uniform_01<> > (rng, *ud);

    //ff_model_dir_init();

    t = 0; // time

    //printf("\n m0 = %e, N = %d", m0, pN);
    //printf("\n %e", M0);
    glob_start_t = start_t;

    m_tot_glob.x = m_tot_glob.y = m_tot_glob.z = 0;

    for (long h = 1; h <= 20; h++)
    {
        B_hyst[h] = B_hyst_n[h] = Mz_hyst[h] = Mz_hyst_n[h] = 0;
    }

    ff_model_size_dispersion_init();
	p = 1;
    while (p <= pN)
    {
again:

        if (start_ideal)
        {
            r[p].x = -0.4999 * Lx + 0.99 * Lx * (*var_uni)();
            r[p].y = -0.4999 * Ly + 0.99 * Ly * (*var_uni)();
            r[p].z = -0.4999 * Lz + 0.99 * Lz * (*var_uni)();
			Rp_to_c[p] = sqrt(MUL(r[p], r[p]));

			/*r[p].x = - R_oleic + 2 * R_oleic * (*var_uni)();
			r[p].y = - R_oleic + 2 * R_oleic * (*var_uni)();
			r[p].z = - R_oleic + 2 * R_oleic * (*var_uni)();*/

			w[p].x = w[p].y = w[p].z = 0;

            //if (p == 5)
            //r[p].x = r[p].y = r[p].z = 0;

            /*Rp_to_c[p] = sqrt(MUL(r[p], r[p]));
			if (Rp_to_c[p] > R_oleic - Rp[p]) goto again;*/
			Rp_to_c[p] = sqrt(MUL(r[p], r[p]));
			if (Rp_to_c[p] > Lx / 4.0) goto again;
			
			for (tp = 1; tp < p; tp++)
            {
                dr.x = r[p].x - r[tp].x;
                dr.y = r[p].y - r[tp].y;
                dr.z = r[p].z - r[tp].z;

                dR = sqrt(MUL(dr,dr));

                if (dR <= Rp[p] + Rp[tp]) goto again;
            }
        } // start_ideal

        v[p].x = v[p].y = v[p].z = 0;
		w[p].x = w[p].y = w[p].z = 0;
		drt_r[p].x = drt_r[p].y = drt_r[p].z = 0;
		dphi_r[p].x = dphi_r[p].y = dphi_r[p].z = 0;
		//dW[p] = 0;
		//for (tp = 1; tp <= pN; tp++) aggregated_p[p][tp] = 0;

        /*theta = pi * rand() / 32768.0;
        phi = 2 * pi * rand() / 32768.0;

        m[p].x = m0p[p] * sin(theta) * cos(phi);
        m[p].y = m0p[p] * sin(theta) * sin(phi);
        m[p].z = m0p[p] * cos(theta);*/

		exist_p[p] = 1;

		//m_freeze[p] = 0;
        m_sat[p] = 0;

        p++;
    }

    if (load_at_start) ff_io_load();

    //if (start_sediment) ff_model_init_sediment();

	r_brown_valid_0 = r[1];
}

// Update of the random force
void ff_model_inst_random_trans_update(long p)
{
	double dx, dy, dz, dphi; // instantiated effective random translation
	double theta_0, phi_0; // random direction of the torque 
	//double dt0;
	double D, D_rot, gamma, gamma_rot;
	double M0, I0;
	double sigma;
	double sigma_rot;
    
	//dt0 = dt * k_bm_inst_max;
	if (Rp_to_c[p] > R_oleic)
	{
		gamma = 6 * pi * eta * Rp[p];
		gamma_rot = 8 * pi * eta * pow(Rp[p], 3);
	}
	else
	{
		gamma = 6 * pi * eta_oleic * Rp[p];
		gamma_rot = 8 * pi * eta_oleic * pow(Rp[p], 3);
	}
	D = kb * dT / gamma;
	D_rot = kb * dT / gamma_rot;
	M0 = M0p[p];
	I0 = I0p[p];

	// instantiation of position change
	// [Langevin equation + Stokes' law]
	sigma = D * (2 * dt0 + (M0 / gamma) * (- 3 + 4 * exp(- gamma * dt0 / M0) - exp(- 2 * gamma * dt0 / M0)));
    dx = (*var_nor)() * sqrt(sigma);
	dy = (*var_nor)() * sqrt(sigma);
	dz = (*var_nor)() * sqrt(sigma);

	drt_r[p].x = dx;
	drt_r[p].y = dy;
	drt_r[p].z = dz;

	// instantiation of rotation of the magnetization direction
	// [Euler-Langevin equation + Stokes' law]
	sigma_rot = D_rot * (2 * dt0 + (I0 / gamma_rot) * (- 3 + 4 * exp(- gamma_rot * dt0 / I0) - exp(- 2 * gamma_rot * dt0 / I0)));
	dphi = (*var_nor)() * sqrt(3 * sigma_rot); // rotation magnitude
	theta_0 = (*var_uni)() * pi;   // rotation vector random direction
	phi_0 = (*var_uni)() * 2 * pi;

	dphi_r[p].x = dphi * sin(theta_0) * cos(phi_0);
	dphi_r[p].y = dphi * sin(theta_0) * sin(phi_0);
	dphi_r[p].z = dphi * cos(theta_0);

	/*Px = (gamma * dx) / (dt0 - M0 * (1 - exp(- gamma * dt0 / M0)) / gamma);
	Py = (gamma * dy) / (dt0 - M0 * (1 - exp(- gamma * dt0 / M0)) / gamma);
	Pz = (gamma * dz) / (dt0 - M0 * (1 - exp(- gamma * dt0 / M0)) / gamma);
	tau_r_phi = (gamma_rot * dphi ) / (dt0 - I0 * (1 - exp(- gamma_rot * dt0 / I0)) / gamma_rot);*/

	/*Px = (*var_nor)() * sqrt(2 * kb * T * gamma / dt);
	Py = (*var_nor)() * sqrt(2 * kb * T * gamma / dt);
	Pz = (*var_nor)() * sqrt(2 * kb * T * gamma / dt);
	tau_r_phi = (*var_nor)() * sqrt(6 * kb * T * gamma_rot / dt);*/

	//printf("\n t1 = %e", M0 / gamma);
	//printf("\n %e", gamma * dt0 / M0);
	//printf("\n %e", gamma * dx / (M0 * v[p].x));

	/*P[p].x = Px * k_force_adapt;
	P[p].y = Py * k_force_adapt;
	P[p].z = Pz * k_force_adapt;

	tau_r[p].x = tau_r_phi * sin(theta_0) * cos(phi_0) * k_force_adapt;
	tau_r[p].y = tau_r_phi * sin(theta_0) * sin(phi_0) * k_force_adapt;
	tau_r[p].z = tau_r_phi * cos(theta_0) * k_force_adapt;*/
}

void ff_model_update_dT(void)
{
	T_basic = (2 / 6.0) * Ek / (kb * pN); // degree of freedom number is 6
	dT = T - T_basic;
	T_mean += T_basic;
	k_mean ++;
	//T_mean_loc += T_basic;
	
	if (dT < 0)
	{
		printf("\n WARNING: dT < 0");
		dT = 0;
	}

	//printf("\n dT = %e", dT);

	/*if (k_bm_inst == k_bm_inst_max - 1)
	{
		dT_prev = dT;
		dT = T - T_mean_loc / k_bm_inst;
		if (dT > 0) k_force_adapt *= k_force_adapt_0;
		else k_force_adapt /= k_force_adapt_0;
		T_mean_loc_prev_revert = T_mean_loc_prev;
		T_mean_loc_prev = T_mean_loc;
	}
	k_bm_inst ++;*/

	//if ((T_mean > T) && (step % 10 == 0)) k_force_adapt /= k_force_adapt_0;
	//if ((T_mean < T) && (step % 10 == 0)) k_force_adapt *= k_force_adapt_0;
}

void ff_model_size_dispersion_init(void)
{
	//double d[14 + 1];
	double F[14 + 1];
	long i, p;
	long imax = 14;
	double Ftot;
	double random_points[14 + 1];
	double random_value;

	d[1] = 33.33333;
	F[1] = 0.0108843537;

	d[2] = 50;
	F[2] = 0.0795918367;

	d[3] = 66.6666;
	F[3] = 0.1850340136;

	d[4] = 83.333;
	F[4] = 0.1972789116;

	d[5] = 100;
	F[5] = 0.1925170068;

	d[6] = 116.66666;
	F[6] = 0.1217687075;

	d[7] = 133.3333;
	F[7] = 0.1006802721;

	d[8] = 150;
	F[8] = 0.074829932;

	d[9] = 166.666666;
	F[9] = 0.056462585;

	d[10] = 183.33333;
	F[10] = 0.0394557823;

	d[11] = 200;
	F[11] = 0.0278911565;

	d[12] = 216.66666;
	F[12] = 0.019047619;

	d[13] = 233.33333;
	F[13] = 0.0115646259;

	d[14] = 250;
	F[14] = 0.006122449;
	
	Ftot = 0;
	for (i = 1; i <= imax; i++)
	{
		d[i] *= 1E-10 * kr; // metric system
		Ftot += F[i];
	}

	for (i = 1; i <= imax; i++)
	{
		random_points[i] = F[i] / Ftot;
        if (i > 1) random_points[i] += random_points[i - 1];
	}
	//printf("\n random_points[14] = %e", random_points[14]);
	//printf("\n random_points[13] = %e", random_points[13]);

	for (p = 1; p <= pN; p++)
	{
		random_value = (*var_uni)();

		//printf("\n random_value = %e", random_value);
		
		if (random_value <= random_points[1])
		{
			Rp[p] = 0.5 * d[1] + delta;
			ff_model_size_dispersion_param_calc(Rp[p] - delta, p);
		}
		for (i = 1; i <= imax - 1; i++)
		if ((random_value > random_points[i]) && (random_value <= random_points[i + 1]))
		{
			Rp[p] = 0.5 * d[i + 1] + delta;
			ff_model_size_dispersion_param_calc(Rp[p] - delta, p);
			break;
		}
	}
}

void ff_model_size_dispersion_param_calc(double R, long p)
{
	double d;
	d = 2 * R;
	double V = pi * pow(d, 3) / 6.0;
	double tm = rop * V;
	double theta, phi;
	
	Vp[p] = V;
	M0p[p] = tm;
	I0p[p] = (2 / 5.0) * M0p[p] * R * R;

	double m_mol_rel = 231.6;
    double m_mol = m_mol_rel * 1E-3 / Na;
    double N_mol = tm / m_mol;
	double s_mol = 4.1 * muB;
    double s = s_mol * N_mol;

	m0p[p] = s;

	if (t == 0)
	{
		theta = pi * rand() / 32768.0;
		phi = 2 * pi * rand() / 32768.0;

		m[p].x = m0p[p] * sin(theta) * cos(phi);
		m[p].y = m0p[p] * sin(theta) * sin(phi);
		m[p].z = m0p[p] * cos(theta);
	}
}

void ff_model_brownian_validation(long p)
{
	double gamma = 6 * pi * eta * Rp[p];
	double D = kb * T / gamma;
	double dr_root_theory = 0;
	double dr_root_sim = 0;

	dr_root_sim = sqrt(pow(r[p].x - r_brown_valid_0.x, 2) + pow(r[p].y - r_brown_valid_0.y, 2) + pow(r[p].z - r_brown_valid_0.z, 2));
	dr_root_theory = sqrt(6 * D * t);

	//printf("\n ff_model_brownian_validation: %e %%", 100 * (dr_root_sim - dr_root_theory) / dr_root_theory);
}

void ff_model_update_conc_in_oleic(long p)
{
	//if (Rp_to_c[p] <= R_oleic)
	if (Rp_to_c[p] <= Lx / 4.0)
	{
		pN_oleic_drop++;
		if ((2 * Rp[p] >= d[1]) && (2 * Rp[p] <= d[2])) pN_oleic_drop_I++;
		if ((2 * Rp[p] >= d[3]) && (2 * Rp[p] <= d[7])) pN_oleic_drop_II++;
		if ((2 * Rp[p] >= d[8]) && (2 * Rp[p] <= d[14])) pN_oleic_drop_III++;

		phi_vol_fract_oleic += Vp[p];
	}
}