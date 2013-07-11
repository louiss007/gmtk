/*
**
** PHiPAC Matrix-Matrix Code for the operation:
**    C = alpha*transpose(A)*B + beta*C
**
** Automatically Generated by mm_lgen ($Revision: 1.5 $) using the command:
**    ./mm_lgen -prec double -opA T -opB N -alpha c -l0 1 1 1 -l1 135 144 94 -gen_rout mm_double_TN_c_l1 -file ./src/mm_double_TN_c.c -routine_name mm_double_TN_c -beta c 
**
** Run './mm_lgen -help' for help.
**
** Generated on: Wednesday July 10 2013, 08:33:28 PDT
** Created by: Jeff Bilmes <bilmes@cs.berkeley.edu>
**             http://www.icsi.berkeley.edu/~bilmes/phipac
**
**
** Routine Usage: 
**    mm_double_TN_c(const int M, const int K, const int N, const double *const A, const double *const B, double *const C, const int Astride, const int Bstride, const int Cstride, const double alpha, const double beta)
** where
**  transpose(A) is an MxK matrix
**  B is an KxN matrix
**  C is an MxN matrix
**  Astride is the number of entries between the start of each row of A
**  Bstride is the number of entries between the start of each row of B
**  Cstride is the number of entries between the start of each row of C
**
**
** "Copyright (c) 1995 The Regents of the University of California.  All
** rights reserved."  Permission to use, copy, modify, and distribute
** this software and its documentation for any purpose, without fee, and
** without written agreement is hereby granted, provided that the above
** copyright notice and the following two paragraphs appear in all copies
** of this software.
**
** IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
** DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
** OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
** CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
** THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
** INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
** AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
** ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
** PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
**
*/

extern mm_double_TN_c_l1(const int M, const int K, const int N, const double *const A, const double *const B, double *const C, const int Astride, const int Bstride, const int Cstride, const double alpha);


void
mm_double_TN_c(const int M, const int K, const int N, const double *const A, const double *const B, double *const C, const int Astride, const int Bstride, const int Cstride, const double alpha, const double beta)
{
   int m,k,n;
   const double *a,*b;
   double *c;
   const double *ap,*bp;
   double *cp;
   if (beta == 0.0) {
      double *cprb,*cpre,*cp,*cpe;
      cpre = C + M*Cstride;
      for (cprb = C; cprb != cpre; cprb += Cstride) {
         cpe = cprb + N;
         for (cp = cprb; cp != cpe; cp++) {
            *cp = 0.0;
         }
      }
   } else if (beta != 1.0) {
      double *cprb,*cpre,*cp,*cpe;
      cpre = C + M*Cstride;
      for (cprb = C; cprb != cpre; cprb += Cstride) {
         cpe = cprb + N;
         for (cp = cprb; cp != cpe; cp++) {
            *cp *= (beta);
         }
      }
   }
   for (m=0; (m+135)<=M; m+=135) {
      c = C + m*Cstride;
      a = A + m;
      for (n=0,b=B,cp=c; (n+94)<=N; n+=94,b+=94,cp+=94) {
         for (k=0,bp=b,ap=a; (k+144)<=K; k+=144,bp+=144*Bstride,ap+=144*Astride) {
            mm_double_TN_c_l1(135,144,94,ap,bp,cp,Astride,Bstride,Cstride,alpha);
         }
         if (k < K) {
            mm_double_TN_c_l1(135,K-k,94,ap,bp,cp,Astride,Bstride,Cstride,alpha);
         }
      }
      if (n < N) {
         for (k=0,bp=b,ap=a; (k+144)<=K; k+=144,bp+=144*Bstride,ap+=144*Astride) {
            mm_double_TN_c_l1(135,144,N-n,ap,bp,cp,Astride,Bstride,Cstride,alpha);
         }
         if (k < K) {
            mm_double_TN_c_l1(135,K-k,N-n,ap,bp,cp,Astride,Bstride,Cstride,alpha);
         }
      }
   }
   if (m < M) {
      c = C + m*Cstride;
      a = A + m;
      for (n=0,b=B,cp=c; (n+94)<=N; n+=94,b+=94,cp+=94) {
         for (k=0,bp=b,ap=a; (k+144)<=K; k+=144,bp+=144*Bstride,ap+=144*Astride) {
            mm_double_TN_c_l1(M-m,144,94,ap,bp,cp,Astride,Bstride,Cstride,alpha);
         }
         if (k < K) {
            mm_double_TN_c_l1(M-m,K-k,94,ap,bp,cp,Astride,Bstride,Cstride,alpha);
         }
      }
      if (n < N) {
         for (k=0,bp=b,ap=a; (k+144)<=K; k+=144,bp+=144*Bstride,ap+=144*Astride) {
            mm_double_TN_c_l1(M-m,144,N-n,ap,bp,cp,Astride,Bstride,Cstride,alpha);
         }
         if (k < K) {
            mm_double_TN_c_l1(M-m,K-k,N-n,ap,bp,cp,Astride,Bstride,Cstride,alpha);
         }
      }
   }
}
