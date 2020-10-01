{ x = $1; y = $2; \
  printf(" (* Picture of process state: 0 0.0 to 0.3 0.6 *)\n");\
  printf("\n");\
  printf(" newcurve marktype box fill 1 marksize 0.3 0.6 pts %f %f\n", x + 0.150000, y + 0.300000);\
  printf(" newline pts %f %f %f %f\n", x + 0.000000, y + 0.150000, x + 0.300000, y + 0.150000);\
  printf(" newline pts %f %f %f %f\n", x + 0.000000, y + 0.300000, x + 0.300000, y + 0.300000);\
  printf(" newline pts %f %f %f %f\n", x + 0.000000, y + 0.450000, x + 0.300000, y + 0.450000);\
  printf(" newstring fontsize 7 hjc vjc x %f y %f : PC\n", x + 0.150000, y + 0.075000);\
  printf(" copystring y %f : . . .\n", y + 0.225000);\
  printf(" copystring y %f : R1\n", y + 0.375000);\
  printf(" copystring y %f : R0\n", y + 0.525000);\
  printf("\n");\
}
