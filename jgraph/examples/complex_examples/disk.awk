{ x = $1; y = $2; \
  printf(" (* Picture of a disk:\n");\
  printf(" 0 0 to 0.4 0.4 *)\n");\
  printf("\n");\
  printf(" newcurve marktype ellipse fill .5 marksize 0.4 0.2 pts %f %f\n", x + 0.200000, y + 0.100000);\
  printf(" newcurve marktype box fill .5 gray .5 marksize 0.4 0.2 pts %f %f\n", x + 0.200000, y + 0.200000);\
  printf(" newline pts %f %f %f %f\n", x + 0.000000, y + 0.100000, x + 0.000000, y + 0.300000);\
  printf(" newline pts %f %f %f %f\n", x + 0.400000, y + 0.100000, x + 0.400000, y + 0.300000);\
  printf(" newcurve marktype ellipse fill 1 marksize 0.4 0.2 pts %f %f\n", x + 0.200000, y + 0.300000);\
  printf("\n");\
}
