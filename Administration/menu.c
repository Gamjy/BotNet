#include <stdio.h>
#include <stdlib.h>
#include "menu.h"

void draw()
{
  printf("                           i:tLL;;fLLLLLff                                                        \n");
  printf("                 ,f:.. ,:::ffi    fffffff                  .Ll      :tffffffL:: ,:                \n");
  printf("    :fffffffffffff:fff,Lfff,     .fffflt          Lfffff: ,L,fffffffffffffffffffffffffffffffffl   \n");
  printf(" ,tfffffffffffffffffff... .tt    iff            :fff,ffffffff ------------------------- ffffffff, \n");
  printf(".lf    ifffffffffffff.   .fffL,                 ,,L; tfffffff|          Sameo          |,    .Ll  \n");
  printf("       .ffffffffffffff,Lffffffft           L:f;;Lffffffffffff|    Systeme et Reseau    |fL,    li \n");
  printf("       ,fffffffffffffl fffffff li           ;ffffffffffffffff|   COMMAND AND CONQUER   |fftt      \n");
  printf("      fffffffffffffffffffff,               fLff   iL tfft..ff ------------------------- fft fi    \n");
  printf("     ,fffffffffffffffffffi                tfft   ..; f,.ffffffl Lffffffffffffffffffffft..f:  .i   \n");
  printf("      ;fffffffffffffffff;                 lLffffffl  ,    :fffffffffffffffffffffffffffff   ,i,    \n");
  printf("       lfffffffft     .i                 .tffffffffffffffffffff fffffffffffffffffffffffff         \n");
  printf("        .lfff         ;,                :fffffffffffffffffL.ffffLf;   tffffffLfffffffft           \n");
  printf("         tff:iL                         lffffffffffffffffffL ffffL     ffffft   tffff ,           \n");
  printf("            :ff                         Lffffffffffffffffffff;Lf,       fffl      ffff:           \n");
  printf("              ;fiiffffL.                 Lfffffffffffffffffffffff        fl       f       l,.     \n");
  printf("               :fffffffffl                 ,,,  ;ffffffffffffffL        t        till    :L       \n");
  printf("             ;fffffffffffL;                     LffffffffffffL                    fl   fff:l,.    \n");
  printf("             tffffffffffffffffl                   Lfffffffffft                     ;:        ifff \n");
  printf("              :fffffffffffffff *********************** ffffffL                                    \n");
  printf("               ifffffffffffff  *         MENU        * ffffffL  :L                    tfff;  f.   \n");
  printf("                 Lffffffffff   * 1- Commencer        * fffffL  fL                   lffffffffffl  \n");
  printf("                 Lffffffff     * 2- Aide             * ffffft  Ll                 tffffffffffffft \n");
  printf("                 ffffffff      * 3- Quitter          * ffffi                      tfffffffffffffL \n");
  printf("                 ffffff,       *                     * ff                        Lll.   lfffff.   \n");
  printf("                 lffff         ***********************                                            \n");
  printf("                  :ff.                                                                            \n");
  printf("                  .Lf                                                                             \n");
  printf("                   .L;                                                                            \n");
}


int menu() {

    system("clear");
    char choix;
    int menu = 0;
    draw();
    while (menu<1 || menu>3) {
	scanf("%c", &choix);
	menu = choix - '0';
    }
    system("clear");
    return menu;
}
