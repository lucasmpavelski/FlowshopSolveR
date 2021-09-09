#pragma once

static double * tmpvect;
static double * tmx;
static double ** rotation;
static double ** rot2;
static double ** linearTF;
static double * peaks21;
static double * peaks22;
static int * rperm;
static int * rperm21;
static int * rperm22;
static double ** Xlocal;
static double ** Xlocal21;
static double ** Xlocal22;
static double ** arrScales;
static double ** arrScales21;
static double ** arrScales22;

/*
 * Noiseless functions testbed. All functions are ranged in [-5, 5]^DIM.
 */
/*isInitDone status changes when either DIM or trialid change.*/
/*it also changes when a new initialisation has been done (since it rewrites the values of Xopt, Fopt...)*/

class CECFunction {
    public:
    double Fopt;


void computeXopt(int seed, int _DIM) {
    int i;

    unif(tmpvect, _DIM, seed);
    for (i = 0; i < _DIM; i++)
    {
        Xopt[i] = 8 * floor(1e4 * tmpvect[i])/1e4 - 4;
        if (Xopt[i] == 0.0)
            Xopt[i] = -1e-5;
    }
}
;}

class CECF1 : public CECFunction {
    public:

    double eval(double* x) {
        double Fadd, r, Fval, Ftrue = 0.;
        double res;
        Fadd = Fopt;
        for (i = 0; i < DIM; i++)
        {
            r = x[i] - Xopt[i];
            Ftrue += r * r;
        }
        Ftrue += Fadd;
        return Ftrue;
    }
}


TwoDoubles f1(double* x) {
    /*Sphere function*/

    int i, rseed; /*Loop over dim*/
    static unsigned int funcId = 1;
    double Fadd, r, Fval, Ftrue = 0.;
    TwoDoubles res;

    if (!isInitDone)
    {
        rseed = funcId + 10000 * trialid;
        /*INITIALIZATION*/
        Fopt = computeFopt(funcId, trialid);
        computeXopt(rseed, DIM);
        isInitDone = 1;
    }

    Fadd = Fopt;
    /* COMPUTATION core*/
    for (i = 0; i < DIM; i++)
    {
        r = x[i] - Xopt[i];
        Ftrue += r * r;
    }
    Ftrue += Fadd;
    Fval = Ftrue; /* without noise*/
    res.Ftrue = Ftrue;
    res.Fval = Fval;
    return res;
}

TwoDoubles f2(double* x) {
    /* separable ellipsoid with monotone transformation, condition 1e6*/

    int i, rseed; /*Loop over dim*/
    static double condition = 1e6;
    static unsigned int funcId = 2;
    double Fadd, Fval, Ftrue = 0.;
    TwoDoubles res;

    if (!isInitDone)
    {
        rseed = funcId + 10000 * trialid;
        /*INITIALIZATION*/
        Fopt = computeFopt(funcId, trialid);
        computeXopt(rseed, DIM);
        isInitDone = 1;
    }

    Fadd = Fopt;

    for (i = 0; i < DIM; i++)
    {
        tmx[i] = x[i] - Xopt[i];
    }

    monotoneTFosc(tmx);

    /* COMPUTATION core*/
    for (i = 0; i < DIM; i++)
    {
        Ftrue += pow(condition, ((double)i)/((double)(DIM-1))) * tmx[i] * tmx[i];
    }
    Ftrue += Fadd;
    Fval = Ftrue; /* without noise*/

    res.Fval = Fval;
    res.Ftrue = Ftrue;
    return res;
}

TwoDoubles f3(double* x) {
    /* Rastrigin with monotone transformation separable "condition" 10*/
    int i, rseed; /*Loop over dim*/

    static unsigned int funcId = 3;
    static double condition = 10.;
    static double beta = 0.2;
    double tmp, tmp2, Fadd, Fval, Ftrue = 0.;
    TwoDoubles res;

    if (!isInitDone)
    {
        rseed = funcId + 10000 * trialid;
        /*INITIALIZATION*/
        Fopt = computeFopt(funcId, trialid);
        computeXopt(rseed, DIM);
        isInitDone = 1;
    }

    Fadd = Fopt;
    for (i = 0; i < DIM; i++)
    {
        tmx[i] = x[i] - Xopt[i];
    }

    monotoneTFosc(tmx);
    for (i = 0; i < DIM; i++)
    {
        tmp = ((double)i)/((double)(DIM-1));
        if (tmx[i] > 0)
            tmx[i] = pow(tmx[i], 1 + beta * tmp * sqrt(tmx[i]));
        tmx[i] = pow(sqrt(condition), tmp) * tmx[i];
    }
    /* COMPUTATION core*/
    tmp = 0.;
    tmp2 = 0.;
    for (i = 0; i < DIM; i++)
    {
        tmp += cos(2*M_PI*tmx[i]);
        tmp2 += tmx[i]*tmx[i];
    }
    Ftrue = 10 * (DIM - tmp) + tmp2;
    Ftrue += Fadd;
    Fval = Ftrue; /* without noise*/

    res.Fval = Fval;
    res.Ftrue = Ftrue;
    return res;
}

TwoDoubles f4(double* x) {
    /* skew Rastrigin-Bueche, condition 10, skew-"condition" 100*/

    int i, rseed; /*Loop over dim*/
    static unsigned int funcId = 4;
    static double condition = 10.;
    static double alpha = 100.;
    double tmp, tmp2, Fadd, Fval, Fpen = 0., Ftrue = 0.;
    TwoDoubles res;

    if (!isInitDone)
    {
        rseed = 3 + 10000 * trialid; /* Not the same as before.*/
        /*INITIALIZATION*/
        Fopt = computeFopt(funcId, trialid);
        computeXopt(rseed, DIM);
        for (i = 0; i < DIM; i += 2)
            Xopt[i] = fabs(Xopt[i]); /*Skew*/
        isInitDone = 1;
    }
    Fadd = Fopt;

    for (i = 0; i < DIM; i++) {
        tmp = fabs(x[i]) - 5.;
        if (tmp > 0.)
            Fpen += tmp * tmp;
    }
    Fpen *= 1e2;
    Fadd += Fpen;

    for (i = 0; i < DIM; i++)
    {
        tmx[i] = x[i] - Xopt[i];
    }

    monotoneTFosc(tmx);
    for (i = 0; i < DIM; i++)
    {
        if (i % 2 == 0 && tmx[i] > 0)
            tmx[i] = sqrt(alpha) * tmx[i];
        tmx[i] = pow(sqrt(condition), ((double)i)/((double)(DIM-1))) * tmx[i];
    }
    /* COMPUTATION core*/
    tmp = 0.;
    tmp2 = 0.;
    for (i = 0; i < DIM; i++)
    {
        tmp += cos(2*M_PI*tmx[i]);
        tmp2 += tmx[i]*tmx[i];
    }
    Ftrue = 10 * (DIM - tmp) + tmp2;
    Ftrue += Fadd;
    Fval = Ftrue; /* without noise*/

    res.Fval = Fval;
    res.Ftrue = Ftrue;
    return res;
}

TwoDoubles f5(double* x) {
    /* linear slope*/
    int i, rseed; /*Loop over dim*/
    static unsigned int funcId = 5;
    static double alpha = 100.;
    static double Fadd; /*Treatment is different from other functions.*/
    double tmp, Fval, Ftrue = 0.;
    TwoDoubles res;

    if (!isInitDone)
    {
        rseed = funcId + 10000 * trialid;
        /*INITIALIZATION*/
        Fopt = computeFopt(funcId, trialid);
        Fadd = Fopt;
        computeXopt(rseed, DIM);
        for (i = 0; i < DIM; i ++)
        {
            tmp = pow(sqrt(alpha), ((double)i)/((double)(DIM-1)));
            if (Xopt[i] > 0)
            {
                Xopt[i] = 5.;
            }
            else if (Xopt[i] < 0)
            {
                Xopt[i] = -5.;
            }
            Fadd += 5. * tmp;
        }
        isInitDone = 1;
    }

    /* BOUNDARY HANDLING*/
    /* move "too" good coordinates back into domain*/
    for (i = 0; i < DIM; i++) {
        if ((Xopt[i] == 5.) && (x[i] > 5))
            tmx[i] = 5.;
        else if ((Xopt[i] == -5.) && (x[i] < -5))
            tmx[i] = -5.;
        else
            tmx[i] = x[i];
    }

    /* COMPUTATION core*/
    for (i = 0; i < DIM; i++)
    {
        if (Xopt[i] > 0) {
            Ftrue -= pow(sqrt(alpha), ((double)i)/((double)(DIM-1))) * tmx[i];
        } else {
            Ftrue += pow(sqrt(alpha), ((double)i)/((double)(DIM-1))) * tmx[i];
        }
    }
    Ftrue += Fadd;
    Fval = Ftrue; /* without noise*/

    res.Fval = Fval;
    res.Ftrue = Ftrue;
    return res;
}

TwoDoubles f6(double* x) {
    /* attractive sector function*/
    int i, j, k, rseed; /*Loop over dim*/
    static unsigned int funcId = 6;
    static double alpha = 100.;
    double Fadd, Fval, Ftrue = 0.;
    TwoDoubles res;

    if (!isInitDone)
    {
        static double condition = 10.;
        rseed = funcId + 10000 * trialid;
        /*INITIALIZATION*/
        Fopt = computeFopt(funcId, trialid);
        computeXopt(rseed, DIM);
        computeRotation(rotation, rseed + 1000000, DIM);
        computeRotation(rot2, rseed, DIM);
        /* decouple scaling from function definition*/
        for (i = 0; i < DIM; i ++)
        {
            for (j = 0; j < DIM; j++)
            {
                linearTF[i][j] = 0.;
                for (k = 0; k < DIM; k++) {
                    linearTF[i][j] += rotation[i][k] * pow(sqrt(condition), ((double)k)/((double)(DIM-1))) * rot2[k][j];
                }
            }
        }
        isInitDone = 1;
    }
    Fadd = Fopt;

    /* BOUNDARY HANDLING*/
    /* TRANSFORMATION IN SEARCH SPACE*/
    for (i = 0; i < DIM; i++) {

        tmx[i] = 0.;
        for (j = 0; j < DIM; j++) {
            tmx[i] += linearTF[i][j] * (x[j] - Xopt[j]);
        }
    }

    /* COMPUTATION core*/
    for (i = 0; i < DIM; i++)
    {
        if (tmx[i] * Xopt[i] > 0)
            tmx[i] *= alpha;
        Ftrue += tmx[i] * tmx[i];
    }

    /*MonotoneTFosc...*/
    if (Ftrue > 0)
    {
        Ftrue = pow(exp(log(Ftrue)/0.1 + 0.49*(sin(log(Ftrue)/0.1) + sin(0.79*log(Ftrue)/0.1))), 0.1);
    }
    else if (Ftrue < 0)
    {
        Ftrue = -pow(exp(log(-Ftrue)/0.1 + 0.49*(sin(0.55 * log(-Ftrue)/0.1) + sin(0.31*log(-Ftrue)/0.1))), 0.1);
    }
    Ftrue = pow(Ftrue, 0.9);
    Ftrue += Fadd;
    Fval = Ftrue; /* without noise*/

    res.Fval = Fval;
    res.Ftrue = Ftrue;
    return res;
}

TwoDoubles f7(double* x) {
    /* step-ellipsoid, condition 100*/

    int i, j, rseed; /*Loop over dim*/
    static unsigned int funcId = 7;
    static double condition = 100.;
    static double alpha = 10.;
    double x1, tmp, Fadd, Fval, Fpen = 0., Ftrue = 0.;
    TwoDoubles res;

    if (!isInitDone)
    {
        rseed = funcId + 10000 * trialid;
        /*INITIALIZATION*/
        Fopt = computeFopt(funcId, trialid);
        computeXopt(rseed, DIM);
        computeRotation(rotation, rseed + 1000000, DIM);
        computeRotation(rot2, rseed, DIM);
        isInitDone = 1;
    }
    Fadd = Fopt;

    /* BOUNDARY HANDLING*/
    for (i = 0; i < DIM; i++)
    {
        tmp = fabs(x[i]) - 5.;
        if (tmp > 0.)
        {
            Fpen += tmp * tmp;
        }
    }
    Fadd += Fpen;

    /* TRANSFORMATION IN SEARCH SPACE*/
    for (i = 0; i < DIM; i++) {

        tmpvect[i] = 0.;
        tmp = sqrt(pow(condition/10., ((double)i)/((double)(DIM-1))));
        for (j = 0; j < DIM; j++) {
            tmpvect[i] += tmp * rot2[i][j] * (x[j] - Xopt[j]);
        }

    }
    x1 = tmpvect[0];

    for (i = 0; i < DIM; i++) {
        if (fabs(tmpvect[i]) > 0.5)
            tmpvect[i] = round(tmpvect[i]);
        else
            tmpvect[i] = round(alpha * tmpvect[i])/alpha;
    }

    for (i = 0; i < DIM; i++) {
        tmx[i] = 0.;
        for (j = 0; j < DIM; j++) {
            tmx[i] += rotation[i][j] * tmpvect[j];
        }
    }

    /* COMPUTATION core*/
    for (i = 0; i < DIM; i++)
    {
        Ftrue += pow(condition, ((double)i)/((double)(DIM-1))) * tmx[i] * tmx[i];
    }
    Ftrue = 0.1 * fmax(1e-4 * fabs(x1), Ftrue);

    Ftrue += Fadd;
    Fval = Ftrue; /* without noise*/

    res.Fval = Fval;
    res.Ftrue = Ftrue;
    return res;
}

TwoDoubles f8(double* x) {
    /* Rosenbrock, non-rotated*/
    static unsigned int funcId = 8;
    int i, rseed; /*Loop over dim*/
    static double scales;
    double tmp, Fadd, Fval, Ftrue = 0.;
    TwoDoubles res;

    if (!isInitDone)
    {
        rseed = funcId + 10000 * trialid;

        scales = fmax(1., sqrt((double)DIM) / 8.);
        /*INITIALIZATION*/
        Fopt = computeFopt(funcId, trialid);
        computeXopt(rseed, DIM);
        for (i = 0; i < DIM; i ++)
            Xopt[i] *= 0.75;
        isInitDone = 1;
    }
    Fadd = Fopt;

    /* BOUNDARY HANDLING*/

    /* TRANSFORMATION IN SEARCH SPACE*/
    for (i = 0; i < DIM; i++) {
        tmx[i] = scales * (x[i] - Xopt[i]) + 1;
    }

    /* COMPUTATION core*/
    for (i = 0; i < DIM - 1; i++)
    {
        tmp = (tmx[i] * tmx[i] - tmx[i+1]);
        Ftrue += tmp * tmp;
    }
    Ftrue *= 1e2;
    for (i = 0; i < DIM - 1; i ++)
    {
        tmp = (tmx[i] - 1.);
        Ftrue += tmp * tmp;
    }
    Ftrue += Fadd;
    Fval = Ftrue; /* without noise*/

    res.Fval = Fval;
    res.Ftrue = Ftrue;
    return res;
}

TwoDoubles f9(double* x) {
    /* Rosenbrock, rotated*/
    int i, j, rseed; /*Loop over dim*/
    static unsigned int funcId = 9;
    double scales, tmp, Fadd, Fval, Ftrue = 0.;
    TwoDoubles res;

    if (!isInitDone)
    {
        rseed = funcId + 10000 * trialid;
        /*INITIALIZATION*/
        Fopt = computeFopt(funcId, trialid);
        /* computeXopt(rseed, DIM);*/
        computeRotation(rotation, rseed, DIM);
        scales = fmax(1., sqrt((double)DIM) / 8.);
        for (i = 0; i < DIM; i ++)
        {
            for (j = 0; j < DIM; j++)
                linearTF[i][j] = scales * rotation[i][j];
        }
/*         for (i = 0; i < DIM; i++)
           {
               Xopt[i] = 0.;
               for (j = 0; j < DIM; j++)
               {
                   Xopt[i] += linearTF[j][i] * 0.5/scales/scales;
                   //computed only if Xopt is returned which is not the case at this point.
               }
            }*/
        isInitDone = 1;
    }
    Fadd = Fopt;

    /* BOUNDARY HANDLING*/

    /* TRANSFORMATION IN SEARCH SPACE*/
    for (i = 0; i < DIM; i++) {
        tmx[i] = 0.5;
        for (j = 0; j < DIM; j++) {
            tmx[i] += linearTF[i][j] * x[j];
        }
    }

    /* COMPUTATION core*/
    for (i = 0; i < DIM - 1; i++)
    {
        tmp = (tmx[i] * tmx[i] - tmx[i+1]);
        Ftrue += tmp * tmp;
    }
    Ftrue *= 1e2;
    for (i = 0; i < DIM - 1; i ++)
    {
       tmp = (tmx[i] - 1.);
        Ftrue += tmp * tmp;
    }

    Ftrue += Fadd;
    Fval = Ftrue; /* without noise*/

    res.Fval = Fval;
    res.Ftrue = Ftrue;
    return res;
}

TwoDoubles f10(double* x) {
    /* ellipsoid with monotone transformation, condition 1e6*/
    int i, j, rseed; /*Loop over dim*/
    static unsigned int funcId = 10;
    static double condition = 1e6;
    double Fadd, Fval, Ftrue = 0.;
    TwoDoubles res;

    if (!isInitDone)
    {
        rseed = funcId + 10000 * trialid;
        /*INITIALIZATION*/
        Fopt = computeFopt(funcId, trialid);
        computeXopt(rseed, DIM);
        computeRotation(rotation, rseed + 1000000, DIM);
        isInitDone = 1;
    }
    Fadd = Fopt;
    /* BOUNDARY HANDLING*/

    /* TRANSFORMATION IN SEARCH SPACE*/
    for (i = 0; i < DIM; i++)
    {
        tmx[i] = 0.;
        for (j = 0; j < DIM; j++) {
            tmx[i] += rotation[i][j] * (x[j] - Xopt[j]);
        }
    }

    monotoneTFosc(tmx);
    /* COMPUTATION core*/
    for (i = 0; i < DIM; i++)
    {
        Ftrue += pow(condition, ((double)i)/((double)(DIM-1))) * tmx[i] * tmx[i];
    }
    Ftrue += Fadd;
    Fval = Ftrue; /* without noise*/

    res.Fval = Fval;
    res.Ftrue = Ftrue;
    return res;
}
