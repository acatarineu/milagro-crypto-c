/**
 * @file fp.c
 * @author Mike Scott
 * @author Kealan McCusker
 * @date 19th May 2015
 * @brief AMCL mod p functions, small Finite Field arithmetic
 *
 * LICENSE
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/* AMCL mod p functions, small Finite Field arithmetic */
/* SU=m, SU is Stack Usage (NOT_SPECIAL Modulus) */

#include "amcl.h"

/* Fast Modular Reduction Methods */

/* r=d mod m */
/* d MUST be normalised */
/* Products must be less than pR in all cases !!! */
/* So when multiplying two numbers, their product *must* be less than MODBITS+BASEBITS*NLEN */
/* Results *may* be one bit bigger than MODBITS */

#if MODTYPE == PSEUDO_MERSENNE
/* r=d mod m */

/* Converts from BIG integer to n-residue form mod Modulus */
void FP_nres(BIG a)
{
    BIG tmp;
    BIG_rcopy(tmp,a);
}

/* Converts from n-residue form back to BIG integer form */
void FP_redc(BIG a)
{
    BIG tmp;
    BIG_rcopy(tmp,a);
}

/* Reduces a DBIG to BIG exploiting special form of the modulus */
void FP_mod(BIG r,DBIG d)
{
    BIG t,b;
    chunk v,tw;
    BIG_split(t,b,d,MODBITS);

    /* Note that all of the excess gets pushed into t. So if squaring a value with a 4-bit excess, this results in
       t getting all 8 bits of the excess product! So products must be less than pR which is Montgomery compatible */

    if (MConst < NEXCESS)
    {
        BIG_imul(t,t,MConst);

        BIG_norm(t);
        tw=t[NLEN-1];
        t[NLEN-1]&=TMASK;
        t[0]+=MConst*((tw>>TBITS));
    }
    else
    {
        v=BIG_pmul(t,t,MConst);
        tw=t[NLEN-1];
        t[NLEN-1]&=TMASK;
#if CHUNK == 16
        t[1]+=muladd(MConst,((tw>>TBITS)+(v<<(BASEBITS-TBITS))),0,&t[0]);
#else
        t[0]+=MConst*((tw>>TBITS)+(v<<(BASEBITS-TBITS)));
#endif
    }
    BIG_add(r,t,b);
    BIG_norm(r);
}
#endif

/* This only applies to Curve C448, so specialised (for now) */
#if MODTYPE == GENERALISED_MERSENNE

/* Converts from n-residue form back to BIG integer form */
void FP_nres(BIG a)
{
    BIG tmp;
    BIG_rcopy(tmp,a);
}

/* Converts from n-residue form back to BIG integer form */
void FP_redc(BIG a)
{
    BIG tmp;
    BIG_rcopy(tmp,a);
}

/* reduce a DBIG to a BIG exploiting the special form of the modulus */
void FP_mod(BIG r,DBIG d)
{
    BIG t,b;
    chunk carry;
    BIG_split(t,b,d,MBITS);

    BIG_add(r,t,b);

    BIG_dscopy(d,t);
    BIG_dshl(d,MBITS/2);

    BIG_split(t,b,d,MBITS);

    BIG_add(r,r,t);
    BIG_add(r,r,b);
    BIG_norm(r);
    BIG_shl(t,MBITS/2);

    BIG_add(r,r,t);

    carry=r[NLEN-1]>>TBITS;

    r[NLEN-1]&=TMASK;
    r[0]+=carry;

    r[224/BASEBITS]+=carry<<(224%BASEBITS); /* need to check that this falls mid-word */
    BIG_norm(r);

}

#endif

#if MODTYPE == MONTGOMERY_FRIENDLY

/* Converts from BIG integer to n-residue form mod Modulus */
void FP_nres(BIG a)
{
    DBIG d;
    BIG m;
    BIG_rcopy(m,Modulus);
    BIG_dscopy(d,a);
    BIG_dshl(d,NLEN*BASEBITS);
    BIG_dmod(a,d,m);
}

/* Converts from n-residue form back to BIG integer form */
void FP_redc(BIG a)
{
    DBIG d;
    BIG_dzero(d);
    BIG_dscopy(d,a);
    FP_mod(a,d);
}

/* Fast modular reduction from DBIG to BIG exploiting special form of the modulus */
void FP_mod(BIG a,DBIG d)
{
    int i;

    for (i=0; i<NLEN; i++)
        d[NLEN+i]+=muladd(d[i],MConst-1,d[i],&d[NLEN+i-1]);

    BIG_sducopy(a,d);
    BIG_norm(a);
}

#endif

#if MODTYPE == NOT_SPECIAL

/* SU= 120, Converts from BIG integer to n-residue form mod Modulus */
void FP_nres(BIG a)
{
    DBIG d;
    BIG m;
    BIG_rcopy(m,Modulus);
    BIG_dscopy(d,a);
    BIG_dshl(d,NLEN*BASEBITS);
    BIG_dmod(a,d,m);
}

/* SU= 80, Converts from n-residue form back to BIG integer form */
void FP_redc(BIG a)
{
    DBIG d;
    BIG_dzero(d);
    BIG_dscopy(d,a);
    FP_mod(a,d);
}

/* SU= 112, Fast modular reduction from DBIG to BIG exploiting special form of the modulus */
void FP_mod(BIG a,DBIG d)
{
    int i,k;
    BIG md;

#ifdef dchunk
    dchunk t,c,s;
    dchunk dd[NLEN];
    chunk v[NLEN];
#endif

    BIG_rcopy(md,Modulus);

#ifdef COMBA

    t=d[0];
    v[0]=((chunk)t*MConst)&BMASK;
    t+=(dchunk)v[0]*md[0];
    c=(t>>BASEBITS)+d[1];
    s=0;

    for (k=1; k<NLEN; k++)
    {
        t=c+s+(dchunk)v[0]*md[k];
        for (i=k-1; i>k/2; i--) t+=(dchunk)(v[k-i]-v[i])*(md[i]-md[k-i]);
        v[k]=((chunk)t*MConst)&BMASK;
        t+=(dchunk)v[k]*md[0];
        c=(t>>BASEBITS)+d[k+1];
        dd[k]=(dchunk)v[k]*md[k];
        s+=dd[k];
    }
    for (k=NLEN; k<2*NLEN-1; k++)
    {
        t=c+s;
        for (i=NLEN-1; i>=1+k/2; i--) t+=(dchunk)(v[k-i]-v[i])*(md[i]-md[k-i]);
        a[k-NLEN]=(chunk)t&BMASK;
        c=(t>>BASEBITS)+d[k+1];
        s-=dd[k-NLEN+1];
    }
    a[NLEN-1]=(chunk)c&BMASK;


    /*
    	t=d[0]; d[0]=((chunk)t*MConst)&BMASK; t+=(dchunk)d[0]*md[0]; c=(t>>BASEBITS)+d[1];

    	for (k=1;k<NLEN;k++)
    	{
    		t = c+(dchunk)d[0]*md[k];
    		for (i=1;i<k;i++) t+=(dchunk)d[i]*md[k-i];
    		d[k]=((chunk)t*MConst)&BMASK; t+=(dchunk)d[k]*md[0]; c=(t>>BASEBITS)+d[k+1];
    	}
    	for (k=NLEN;k<2*NLEN-1;k++)
    	{
    		t = c;
    		for (i=k-NLEN+1;i<NLEN;i++) t+=(dchunk)d[i]*md[k-i];
    		d[k]=(chunk)t&BMASK; c=(t>>BASEBITS)+d[k+1];
    	}
    	d[2*NLEN-1]=(chunk)c&BMASK;
    */
    /*
    	sum=d[0];
    	if (MConst==-1) sp=(-(chunk)sum)&BMASK;
    	else
    	{
    		if (MConst==1) sp=((chunk)sum)&BMASK;
    		else sp=((chunk)sum*MConst)&BMASK;
    	}
    	d[0]=sp; sum+=(dchunk)sp*md[0];  // no need for &BMASK here!
    	sum=d[1]+(sum>>BASEBITS);

    	for (j=1;j<NLEN;j++)
    	{
    		sum+=(dchunk)d[0]*md[j];
    		for (i=1;i<j;i++) sum+=(dchunk)d[i]*md[j-i];
    		if (MConst==-1) sp=(-(chunk)sum)&BMASK;
    		else
    		{
    			if (MConst==1) sp=((chunk)sum)&BMASK;
    			else sp=((chunk)sum*MConst)&BMASK;
    		}
    		d[j]=sp;
    		dd[j]=(dchunk)sp*md[j];
    		sum+=(dchunk)sp*md[0];  // no need for &BMASK here!
    		sum=d[j+1]+(sum>>BASEBITS);
    	}

    	for (j=NLEN;j<DNLEN-2;j++)
    	{
    		for (i=j-NLEN+1;i<NLEN;i++) sum+=(dchunk)d[i]*md[j-i];
    		d[j]=(chunk)sum&BMASK;
    		sum=d[j+1]+(sum>>BASEBITS);
    	}

    	sum+=(dchunk)d[NLEN-1]*md[NLEN-1];
    	d[DNLEN-2]=(chunk)sum&BMASK;
    	sum=d[DNLEN-1]+(sum>>BASEBITS);
    	d[DNLEN-1]=(chunk)sum&BMASK;
    */
//	BIG_sducopy(a,d);
    BIG_norm(a);


    /* Faster to Combafy it.. Let the compiler unroll the loops! */
    /*
    	sum=d[0];
    	for (j=0;j<NLEN;j++)
    	{
    		for (i=0;i<j;i++) sum+=(dchunk)d[i]*md[j-i];
    		if (MConst==-1) sp=(-(chunk)sum)&BMASK;
    		else
    		{
    			if (MConst==1) sp=((chunk)sum)&BMASK;
    			else sp=((chunk)sum*MConst)&BMASK;
    		}
    		d[j]=sp; sum+=(dchunk)sp*md[0];  // no need for &BMASK here!
    		sum=d[j+1]+(sum>>BASEBITS);
    	}

    	for (j=NLEN;j<DNLEN-2;j++)
    	{
    		for (i=j-NLEN+1;i<NLEN;i++) sum+=(dchunk)d[i]*md[j-i];
    		d[j]=(chunk)sum&BMASK;
    		sum=d[j+1]+(sum>>BASEBITS);
    	}

    	sum+=(dchunk)d[NLEN-1]*md[NLEN-1];
    	d[DNLEN-2]=(chunk)sum&BMASK;
    	sum=d[DNLEN-1]+(sum>>BASEBITS);
    	d[DNLEN-1]=(chunk)sum&BMASK;

    	BIG_sducopy(a,d);
    	BIG_norm(a);
    */
#else
    int j;
    chunk m,carry;
    for (i=0; i<NLEN; i++)
    {
        if (MConst==-1) m=(-d[i])&BMASK;
        else
        {
            if (MConst==1) m=d[i];
            else m=(MConst*d[i])&BMASK;
        }
        carry=0;
        for (j=0; j<NLEN; j++)
            carry=muladd(m,md[j],carry,&d[i+j]);
        d[NLEN+i]+=carry;
    }
    BIG_sducopy(a,d);
    BIG_norm(a);

#endif
}

#endif

/* SU= 48, Tests for BIG equal to zero mod Modulus */
int FP_iszilch(BIG x)
{
    BIG m;
    BIG_rcopy(m,Modulus);
    BIG_mod(x,m);
    return BIG_iszilch(x);
}

/* SU= 48, Outputs a BIG number that is in n-residue form to the console */
void FP_output(BIG r)
{
    BIG c;
    BIG_copy(c,r);
    FP_redc(c);
    BIG_output(c);
}

/* Outputs a BIG number that is in n-residue form to the console, in raw form */
void FP_rawoutput(BIG r)
{
    BIG_rawoutput(r);
}

#ifdef GET_STATS
int tsqr=0,rsqr=0,tmul=0,rmul=0;
int tadd=0,radd=0,tneg=0,rneg=0;
int tdadd=0,rdadd=0,tdneg=0,rdneg=0;
#endif

/* SU= 88, Fast Modular multiplication of two BIGs in n-residue form, mod Modulus */
void FP_mul(BIG r,BIG a,BIG b)
{
    DBIG d;
    chunk ea=EXCESS(a);
    chunk eb=EXCESS(b);
    if ((ea+1)>=(FEXCESS-1)/(eb+1))
    {
#ifdef DEBUG_REDUCE
        printf("Product too large - reducing it %d %d\n",ea,eb);
#endif
        FP_reduce(a);  /* it is sufficient to fully reduce just one of them < p */
#ifdef GET_STATS
        rmul++;
    }
    else
    {
        BIG_norm(a);   /* change here */
    }
    tmul++;
#else
    }
    else
    {
        BIG_norm(a);   /* change here */
    }
#endif

    BIG_norm(b);
    BIG_mul(d,a,b);
    FP_mod(r,d);
}

/* SU= 136, Fast Modular multiplication of a BIG in n-residue form, by a small integer, mod Modulus */
void FP_imul(BIG r,BIG a,int c)
{
    DBIG d;
    BIG m;
    int s=0;
    chunk afx;
    BIG_norm(a);
    if (c<0)
    {
        c=-c;
        s=1;
    }
    afx=(EXCESS(a)+1)*(c+1)+1;
    if (c<NEXCESS && afx<FEXCESS)
        BIG_imul(r,a,c);
    else
    {
        if (afx<FEXCESS)
        {
            BIG_pmul(r,a,c);
        }
        else
        {
            BIG_rcopy(m,Modulus);
            BIG_pxmul(d,a,c);
            BIG_dmod(r,d,m);
        }
    }
    if (s) FP_neg(r,r);
    BIG_norm(r);
}

/* SU= 88, Fast Modular squaring of a BIG in n-residue form, mod Modulus */
void FP_sqr(BIG r,BIG a)
{
    DBIG d;
    chunk ea=EXCESS(a);
    if ((ea+1)>=(FEXCESS-1)/(ea+1))
    {
#ifdef DEBUG_REDUCE
        printf("Product too large - reducing it %d\n",ea);
#endif
        FP_reduce(a);
#ifdef GET_STATS
        rsqr++;
    }
    else
    {
        BIG_norm(a);   /* change here */
    }
    tsqr++;
#else
    }
    else
    {
        BIG_norm(a);   /* change here */
    }
#endif

    BIG_sqr(d,a);
    FP_mod(r,d);
}

/* SU= 16, Modular addition of two BIGs in n-residue form, mod Modulus */
void FP_add(BIG r,BIG a,BIG b)
{
    BIG_add(r,a,b);
    if (EXCESS(r)+2>=FEXCESS)  /* +2 because a and b not normalised */
    {
#ifdef DEBUG_REDUCE
        printf("Sum too large - reducing it %d\n",EXCESS(r));
#endif
        FP_reduce(r);
#ifdef GET_STATS
        radd++;
    }
    tadd++;
#else
    }
#endif
}

/* SU= 56, Modular subtraction of two BIGs in n-residue form, mod Modulus */
void FP_sub(BIG r,BIG a,BIG b)
{
    BIG n;
    FP_neg(n,b);
    FP_add(r,a,n);
}

/* SU= 48, Reduces possibly unreduced BIG mod Modulus */
void FP_reduce(BIG a)
{
    BIG m;
    BIG_rcopy(m,Modulus);
    BIG_mod(a,m);
}

/* SU= 64, Modular negation of a BIG in n-residue form, mod Modulus */
void FP_neg(BIG r,BIG a)
{
    int sb;
    chunk ov;
    BIG m;

    BIG_rcopy(m,Modulus);
    BIG_norm(a);

    ov=EXCESS(a);
    sb=1;
    while(ov!=0)
    {
        sb++;    /* only unpredictable branch */
        ov>>=1;
    }

    BIG_fshl(m,sb);
    BIG_sub(r,m,a);

    if (EXCESS(r)>=FEXCESS)
    {
#ifdef DEBUG_REDUCE
        printf("Negation too large -  reducing it %d\n",EXCESS(r));
#endif
        FP_reduce(r);
#ifdef GET_STATS
        rneg++;
    }
    tneg++;
#else
    }
#endif

}

/* SU= 56, Modular division by 2 of a BIG in n-residue form, mod Modulus */
void FP_div2(BIG r,BIG a)
{
    BIG m;
    BIG_rcopy(m,Modulus);
    BIG_norm(a);
    if (BIG_parity(a)==0)
    {
        BIG_copy(r,a);
        BIG_fshr(r,1);
    }
    else
    {
        BIG_add(r,a,m);
        BIG_norm(r);
        BIG_fshr(r,1);
    }
}

/* Modular inverse of a BIG in n-residue form, mod Modulus */
void FP_inv(BIG w,BIG x)
{
    BIG m;
    BIG_rcopy(m,Modulus);
    BIG_copy(w,x);
    FP_redc(w);

    BIG_invmodp(w,w,m);
    FP_nres(w);
}

/* SU=8 */
/* set n=1 */
void FP_one(BIG n)
{
    BIG_one(n);
    FP_nres(n);
}

/* SU= 136, Fast Modular exponentiation of a BIG in n-residue form, to the power of a BIG, mod Modulus */
void FP_pow(BIG r,BIG a,BIG b)
{
    BIG w,z,zilch;
    int bt;
    BIG_zero(zilch);

    BIG_norm(b);
    BIG_copy(z,b);
    BIG_copy(w,a);
    FP_one(r);
    while(1)
    {
        bt=BIG_parity(z);
        BIG_fshr(z,1);
        if (bt) FP_mul(r,r,w);
        if (BIG_comp(z,zilch)==0) break;
        FP_sqr(w,w);
    }
    FP_reduce(r);
}

/* Tests for BIG a quadratic residue mod Modulus */
int FP_qr(BIG r)
{
    int j;
    BIG m;
    BIG_rcopy(m,Modulus);
    FP_redc(r);
    j=BIG_jacobi(r,m);
    FP_nres(r);
    if (j==1) return 1;
    return 0;

}

/* SU= 160, Fast Modular square root of a BIG in n-residue form, mod Modulus */
void FP_sqrt(BIG r,BIG a)
{
    BIG v,i,b;
    BIG m;
    BIG_rcopy(m,Modulus);
    BIG_mod(a,m);
    BIG_copy(b,m);
    if (MOD8==5)
    {
        BIG_dec(b,5);
        BIG_norm(b);
        BIG_fshr(b,3); /* (p-5)/8 */
        BIG_copy(i,a);
        BIG_fshl(i,1);
        FP_pow(v,i,b);
        FP_mul(i,i,v);
        FP_mul(i,i,v);
        BIG_dec(i,1);
        FP_mul(r,a,v);
        FP_mul(r,r,i);
        BIG_mod(r,m);
    }
    if (MOD8==3 || MOD8==7)
    {
        BIG_inc(b,1);
        BIG_norm(b);
        BIG_fshr(b,2); /* (p+1)/4 */
        FP_pow(r,a,b);
    }
}

/* Tests for equality of two FPs */
int FP_equals(BIG x,BIG y)
{
	FP_reduce(x);
	FP_reduce(y);
	if (BIG_comp(x,y)==0)
        return 1;
    return 0;
}

/*
int main()
{

	BIG r;

	FP_one(r);
	FP_sqr(r,r);

	BIG_output(r);

	int i,carry;
	DBIG c={0,0,0,0,0,0,0,0};
	BIG a={1,2,3,4};
	BIG b={3,4,5,6};
	BIG r={11,12,13,14};
	BIG s={23,24,25,15};
	BIG w;

//	printf("NEXCESS= %d\n",NEXCESS);
//	printf("MConst= %d\n",MConst);

	BIG_copy(b,Modulus);
	BIG_dec(b,1);
	BIG_norm(b);

	BIG_randomnum(r); BIG_norm(r); BIG_mod(r,Modulus);
//	BIG_randomnum(s); norm(s); BIG_mod(s,Modulus);

//	BIG_output(r);
//	BIG_output(s);

	BIG_output(r);
	FP_nres(r);
	BIG_output(r);
	BIG_copy(a,r);
	FP_redc(r);
	BIG_output(r);
	BIG_dscopy(c,a);
	FP_mod(r,c);
	BIG_output(r);


//	exit(0);

//	copy(r,a);
	printf("r=   "); BIG_output(r);
	BIG_modsqr(r,r,Modulus);
	printf("r^2= "); BIG_output(r);

	FP_nres(r);
	FP_sqrt(r,r);
	FP_redc(r);
	printf("r=   "); BIG_output(r);
	BIG_modsqr(r,r,Modulus);
	printf("r^2= "); BIG_output(r);


//	for (i=0;i<100000;i++) FP_sqr(r,r);
//	for (i=0;i<100000;i++)
		FP_sqrt(r,r);

	BIG_output(r);
}
*/
