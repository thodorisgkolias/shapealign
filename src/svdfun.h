

/* 
 * svdcomp - SVD decomposition routine. 
 * Takes an mxn matrix a and decomposes it into udv, where u,v are
 * left and right orthogonal transformation matrices, and d is a 
 * diagonal matrix of singular values.
 *
 * This routine is adapted from svdecomp.c in XLISP-STAT 2.1 which is 
 * code from Numerical Recipes adapted by Luke Tierney and David Betz.
 *
 * Input to dsvd is as follows:
 *   a = mxn matrix to be decomposed, gets overwritten with u
 *   m = row dimension of a
 *   n = column dimension of a
 *   w = returns the vector of singular values of a
 *   v = returns the right orthogonal transformation matrix
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MIN(x,y) ( (x) < (y) ? (x) : (y) )
#define MAX(x,y) ((x)>(y)?(x):(y))
#define SIGN(a, b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

 
static double PYTHAG(double a, double b)
{
    double at = fabs(a), bt = fabs(b), ct, result;

    if (at > bt)       { ct = bt / at; result = at * sqrt(1.0 + ct * ct); }
    else if (bt > 0.0) { ct = at / bt; result = bt * sqrt(1.0 + ct * ct); }
    else result = 0.0;
    return(result);
}


int dsvd(double **a, int m, int n, double *w, double **v)
{
    int flag, i, its, j, jj, k, l, nm;
    double c, f, h, s, x, y, z;
    double anorm = 0.0, g = 0.0, scale = 0.0;
    double *rv1;
  
 /*   if (m < n) 
    {
       fprintf(stderr, "#rows must be > #cols \n");
        return(0);
    }*/
  
    rv1 = (double *)malloc((unsigned int) n*sizeof(double));

/* Householder reduction to bidiagonal form */
    for (i = 0; i < n; i++) 
    {
        /* left-hand reduction */
        l = i + 1;
        rv1[i] = scale * g;
        g = s = scale = 0.0;
        if (i < m) 
        {
            for (k = i; k < m; k++) 
                scale += fabs((double)a[k][i]);
            if (scale) 
            {
                for (k = i; k < m; k++) 
                {
                    a[k][i] = (double)((double)a[k][i]/scale);
                    s += ((double)a[k][i] * (double)a[k][i]);
                }
                f = (double)a[i][i];
                g = -SIGN(sqrt(s), f);
                h = f * g - s;
                a[i][i] = (double)(f - g);
                if (i != n - 1) 
                {
                    for (j = l; j < n; j++) 
                    {
                        for (s = 0.0, k = i; k < m; k++) 
                            s += ((double)a[k][i] * (double)a[k][j]);
                        f = s / h;
                        for (k = i; k < m; k++) 
                            a[k][j] += (double)(f * (double)a[k][i]);
                    }
                }
                for (k = i; k < m; k++) 
                    a[k][i] = (double)((double)a[k][i]*scale);
            }
        }
        w[i] = (double)(scale * g);
    
        /* right-hand reduction */
        g = s = scale = 0.0;
        if (i < m && i != n - 1) 
        {
            for (k = l; k < n; k++) 
                scale += fabs((double)a[i][k]);
            if (scale) 
            {
                for (k = l; k < n; k++) 
                {
                    a[i][k] = (double)((double)a[i][k]/scale);
                    s += ((double)a[i][k] * (double)a[i][k]);
                }
                f = (double)a[i][l];
                g = -SIGN(sqrt(s), f);
                h = f * g - s;
                a[i][l] = (double)(f - g);
                for (k = l; k < n; k++) 
                    rv1[k] = (double)a[i][k] / h;
                if (i != m - 1) 
                {
                    for (j = l; j < m; j++) 
                    {
                        for (s = 0.0, k = l; k < n; k++) 
                            s += ((double)a[j][k] * (double)a[i][k]);
                        for (k = l; k < n; k++) 
                            a[j][k] += (double)(s * rv1[k]);
                    }
                }
                for (k = l; k < n; k++) 
                    a[i][k] = (double)((double)a[i][k]*scale);
            }
        }
        anorm = MAX(anorm, (fabs((double)w[i]) + fabs(rv1[i])));
    }
  
    /* accumulate the right-hand transformation */
    for (i = n - 1; i >= 0; i--) 
    {
        if (i < n - 1) 
        {
            if (g) 
            {
                for (j = l; j < n; j++)
                    v[j][i] = (double)(((double)a[i][j] / (double)a[i][l]) / g);
                    /* double division to avoid underflow */
                for (j = l; j < n; j++) 
                {
                    for (s = 0.0, k = l; k < n; k++) 
                        s += ((double)a[i][k] * (double)v[k][j]);
                    for (k = l; k < n; k++) 
                        v[k][j] += (double)(s * (double)v[k][i]);
                }
            }
            for (j = l; j < n; j++) 
                v[i][j] = v[j][i] = 0.0;
        }
        v[i][i] = 1.0;
        g = rv1[i];
        l = i;
    }
  
    /* accumulate the left-hand transformation */
    for (i = n - 1; i >= 0; i--) 
    {
        l = i + 1;
        g = (double)w[i];
        if (i < n - 1) 
            for (j = l; j < n; j++) 
                a[i][j] = 0.0;
        if (g) 
        {
            g = 1.0 / g;
            if (i != n - 1) 
            {
                for (j = l; j < n; j++) 
                {
                    for (s = 0.0, k = l; k < m; k++) 
                        s += ((double)a[k][i] * (double)a[k][j]);
                    f = (s / (double)a[i][i]) * g;
                    for (k = i; k < m; k++) 
                        a[k][j] += (double)(f * (double)a[k][i]);
                }
            }
            for (j = i; j < m; j++) 
                a[j][i] = (double)((double)a[j][i]*g);
        }
        else 
        {
            for (j = i; j < m; j++) 
                a[j][i] = 0.0;
        }
        ++a[i][i];
    }

    /* diagonalize the bidiagonal form */
    for (k = n - 1; k >= 0; k--) 
    {                             /* loop over singular values */
        for (its = 0; its < 30; its++) 
        {                         /* loop over allowed iterations */
            flag = 1;
            for (l = k; l >= 0; l--) 
            {                     /* test for splitting */
                nm = l - 1;
                if (fabs(rv1[l]) + anorm == anorm) 
                {
                    flag = 0;
                    break;
                }
                if (fabs((double)w[nm]) + anorm == anorm) 
                    break;
            }
            if (flag) 
            {
                c = 0.0;
                s = 1.0;
                for (i = l; i <= k; i++) 
                {
                    f = s * rv1[i];
                    if (fabs(f) + anorm != anorm) 
                    {
                        g = (double)w[i];
                        h = PYTHAG(f, g);
                        w[i] = (double)h; 
                        h = 1.0 / h;
                        c = g * h;
                        s = (- f * h);
                        for (j = 0; j < m; j++) 
                        {
                            y = (double)a[j][nm];
                            z = (double)a[j][i];
                            a[j][nm] = (double)(y * c + z * s);
                            a[j][i] = (double)(z * c - y * s);
                        }
                    }
                }
            }
            z = (double)w[k];
            if (l == k) 
            {                  /* convergence */
                if (z < 0.0) 
                {              /* make singular value nonnegative */
                    w[k] = (double)(-z);
                    for (j = 0; j < n; j++) 
                        v[j][k] = (-v[j][k]);
                }
                break;
            }
        /*    if (its >= 30) {
                free((void*) rv1);
                fprintf(stderr, "No convergence after 30,000! iterations \n");
                return(0);
        } */
    
            /* shift from bottom 2 x 2 minor */
            x = (double)w[l];
            nm = k - 1;
            y = (double)w[nm];
            g = rv1[nm];
            h = rv1[k];
            f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
            g = PYTHAG(f, 1.0);
            f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g, f))) - h)) / x;
          
            /* next QR transformation */
            c = s = 1.0;
            for (j = l; j <= nm; j++) 
            {
                i = j + 1;
                g = rv1[i];
                y = (double)w[i];
                h = s * g;
                g = c * g;
                z = PYTHAG(f, h);
                rv1[j] = z;
                c = f / z;
                s = h / z;
                f = x * c + g * s;
                g = g * c - x * s;
                h = y * s;
                y = y * c;
                for (jj = 0; jj < n; jj++) 
                {
                    x = (double)v[jj][j];
                    z = (double)v[jj][i];
                    v[jj][j] = (double)(x * c + z * s);
                    v[jj][i] = (double)(z * c - x * s);
                }
                z = PYTHAG(f, h);
                w[j] = (double)z;
                if (z) 
                {
                    z = 1.0 / z;
                    c = f * z;
                    s = h * z;
                }
                f = (c * g) + (s * y);
                x = (c * y) - (s * g);
                for (jj = 0; jj < m; jj++) 
                {
                    y = (double)a[jj][j];
                    z = (double)a[jj][i];
                    a[jj][j] = (double)(y * c + z * s);
                    a[jj][i] = (double)(z * c - y * s);
                }
            }
            rv1[l] = 0.0;
            rv1[k] = f;
            w[k] = (double)x;
        }
    }
    free((void*) rv1);
    return(1);
}


 /************************************************************************/
/* dsvdcmp.C -- contains routines for doing SVD and finding 2-norm      */
/*      condition numbers for matrices.                                         */
/*                                                                      */
/*      Author:         Code originally from Numerical Recipes in C, 2nd ed.    */
/*              modifications by A. Thall.                              */
/*      Date:   Mods done 2. Feb 97.                                    */
/*                                                                      */
/*      Notes:  1.  NumRec routines are faux FORTRAN; in original code,         */
/*              all matrices and vectors were 1-indexed.  This has been         */
/*              remedied here.                                          */
/*              2.  This code is straight C, compiled under C++ to      */
/*              avoid having to do an 'extern "C"' declaration, but no  */
/*              big deal either way.                                    */ 
/*                                                                      */
/************************************************************************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/************************************************************************/
/* Some macros for comparisons, vector allocation, etc.                 */
/************************************************************************/
static double dsqrarg;
#define DSQR(a) ((dsqrarg=(a)) == 0.0 ? 0.0 : dsqrarg*dsqrarg)
static double dmaxarg1,dmaxarg2;
#define DMAX(a,b) (dmaxarg1=(a),dmaxarg2=(b),(dmaxarg1) > (dmaxarg2) ?\
        (dmaxarg1) : (dmaxarg2))
static double dminarg1,dminarg2;
#define DMIN(a,b) (dminarg1=(a),dminarg2=(b),(dminarg1) < (dminarg2) ?\
        (dminarg1) : (dminarg2))
static int iminarg1,iminarg2;
#define IMIN(a,b) (iminarg1=(a),iminarg2=(b),(iminarg1) < (iminarg2) ?\
        (iminarg1) : (iminarg2))
#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

#define DVECTOR(size) ((double *) malloc((unsigned int) size*sizeof(double)))

/************************************************************************/
/* dpythag() -- a double routine for stable computation of Euclidean    */
/*      length of vector [a, b] without overflow.                       */
/************************************************************************/
double dpythag(double a, double b)
{
        double absa,absb;
        absa=fabs(a);
        absb=fabs(b);
        if (absa > absb) return absa*sqrt(1.0+DSQR(absb/absa));
        else return (absb == 0.0 ? 0.0 : absb*sqrt(1.0+DSQR(absa/absb)));
}

/************************************************************************/
/* dsvdcmp() -- a routine (now 0-index) originally from NumRec in C     */
/*      for doing SVD decomposition of an m-by-n matrix.                */
/*      For what meager documentation there is, see NumRec in C, 2nd    */
/*      edition, pg. 67-70.                                             */
/* This function expects a to be an m-by-n matrix in row-major order,   */
/*      w to be a double array of length n, and v to be an n-by-n       */
/*      matrix.  Matrix a will be replaced by U, v will receive the     */
/*      values of V (not V'), and w will get the diagonal components    */
/*      of W, the singular values, where a = U*W*V'.                    */
/************************************************************************/
void dsvdcmp(double **a, int m, int n, double *w, double **v)
{
    int flag,i,its,j,jj,k,l,nm;
    double anorm,c,f,g,h,s,scale,x,y,z,*rv1;

    rv1=DVECTOR(n);
    g=scale=anorm=0.0;
    for (i=0;i<n;i++) {
        l=i+1;
        rv1[i]=scale*g;
        g=s=scale=0.0;
        if (i < m) {
                for (k=i;k<m;k++) scale += fabs(a[k][i]);
                if (scale) {
                        for (k=i;k<m;k++) {
                                a[k][i] /= scale;
                                s += a[k][i]*a[k][i];
                        }
                        f=a[i][i];
                        g = -SIGN(sqrt(s),f);
                        h=f*g-s;
                        a[i][i]=f-g;
                        for (j=l;j<n;j++) {
                                for (s=0.0,k=i;k<m;k++) s += a[k][i]*a[k][j];
                                f=s/h;
                                for (k=i;k<m;k++) a[k][j] += f*a[k][i];
                        }
                        for (k=i;k<m;k++) a[k][i] *= scale;
                }
        }
        w[i]=scale *g;
        g=s=scale=0.0;
        if (i < m && i != (n - 1)) {
                for (k=l;k<n;k++) scale += fabs(a[i][k]);
                if (scale) {
                        for (k=l;k<n;k++) {
                                a[i][k] /= scale;
                                s += a[i][k]*a[i][k];
                        }
                        f=a[i][l];
                        g = -SIGN(sqrt(s),f);
                        h=f*g-s;
                        a[i][l]=f-g;
                        for (k=l;k<n;k++) rv1[k]=a[i][k]/h;
                        for (j=l;j<m;j++) {
                                for (s=0.0,k=l;k<n;k++) s += a[j][k]*a[i][k];
                                for (k=l;k<n;k++) a[j][k] += s*rv1[k];
                        }
                        for (k=l;k<n;k++) a[i][k] *= scale;
                }
        }
        anorm=DMAX(anorm,(fabs(w[i])+fabs(rv1[i])));
    }
    for (i = (n-1); i >= 0;i--) {
        if (i < (n-1)) {
                if (g) {
                        for (j=l;j<n;j++) v[j][i]=(a[i][j]/a[i][l])/g;
                        for (j=l;j<n;j++) {
                                for (s=0.0,k=l;k<n;k++) s += a[i][k]*v[k][j];
                                for (k=l;k<n;k++) v[k][j] += s*v[k][i];
                        }
                }
        for (j=l;j<n;j++) v[i][j]=v[j][i]=0.0;
        }
        v[i][i]=1.0;
        g=rv1[i];
        l=i;
    }
    for (i=IMIN(m,n)-1;i>=0;i--) {
        l=i+1;
        g=w[i];
        for (j=l;j<n;j++) a[i][j]=0.0;
        if (g) {
                g=1.0/g;
                for (j=l;j<n;j++) {
                        for (s=0.0,k=l;k<m;k++) s += a[k][i]*a[k][j];
                        f=(s/a[i][i])*g;
                        for (k=i;k<m;k++) a[k][j] += f*a[k][i];
                }
                for (j=i;j<m;j++) a[j][i] *= g;
        } else for (j=i;j<m;j++) a[j][i]=0.0;
        ++a[i][i];
    }
    for (k = (n-1); k>=0; k--) {
        for (its=1;its<=30;its++) {
                flag=1;
                for (l=k;l>=0;l--) {
                        nm=l-1;
                        if ((double)(fabs(rv1[l])+anorm) == anorm) {
                                flag=0;
                                break;
                        }
                        if ((double)(fabs(w[nm])+anorm) == anorm) break;
                }
                if (flag) {
                        c=0.0;
                        s=1.0;
                        for (i=l;i<=k;i++) {
                                f=s*rv1[i];
                                rv1[i]=c*rv1[i];
                                if ((double)(fabs(f)+anorm) == anorm) break;
                                g=w[i];
                                h=dpythag(f,g);
                                w[i]=h;
                                h=1.0/h;
                                c=g*h;
                                s = -f*h;
                                for (j=0;j<m;j++) {
                                        y=a[j][nm];
                                        z=a[j][i];
                                        a[j][nm]=y*c+z*s;
                                        a[j][i]=z*c-y*s;
                                }
                        }
                }
                z=w[k];
                if (l == k) {
                        if (z < 0.0) {
                                w[k] = -z;
                                for (j=0;j<n;j++) v[j][k] = -v[j][k];
                        }
                        break;
                }             
            /*    if (its == 30)
                    fprintf(stderr, "no convergence in 30 dsvdcmp iterations");
             */
                x=w[l];
                nm=k-1;
                y=w[nm];
                g=rv1[nm];
                h=rv1[k];
                f=((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
                g=dpythag(f,1.0);
                f=((x-z)*(x+z)+h*((y/(f+SIGN(g,f)))-h))/x;
                c=s=1.0;
                for (j=l;j<=nm;j++) { 
                        i=j+1;
                        g=rv1[i];
                        y=w[i];
                        h=s*g;
                        g=c*g;
                        z=dpythag(f,h);
                        rv1[j]=z;
                        c=f/z;
                        s=h/z;
                        f=x*c+g*s;
                        g = g*c-x*s;
                        h=y*s;
                        y *= c;
                        for (jj=0;jj<n;jj++) {
                                x=v[jj][j];
                                z=v[jj][i];
                                v[jj][j]=x*c+z*s;
                                v[jj][i]=z*c-x*s;
                        }
                        z=dpythag(f,h);
                        w[j]=z;
                        if (z) {
                                z=1.0/z;
                                c=f*z;
                                s=h*z;
                        }
                        f=c*g+s*y;
                        x=c*y-s*g;
                        for (jj=0;jj<m;jj++) {
                        y=a[jj][j];
                                z=a[jj][i];
                                a[jj][j]=y*c+z*s;
                                a[jj][i]=z*c-y*s;
                        }
                }
                rv1[l]=0.0;
                rv1[k]=f;
                w[k]=x;
        }                
    } 
    free(rv1); 
}
/* (C) Copr. 1986-92 Numerical Recipes Software 42,. */
