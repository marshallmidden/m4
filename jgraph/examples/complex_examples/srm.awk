{ x = $1; y = $2; \
  printf(" (* newgraph *)\n");\
  printf(" (* xaxis min 0 max 2 size 2 nodraw *)\n");\
  printf(" (* yaxis min 0 max 2.5 size 2.5 nodraw *)\n");\
  printf("\n");\
  printf(" (* This is a picture of the SRM for the hypercube *)\n");\
  printf("\n");\
  printf(" newcurve linetype solid linethickness .7 marktype none\n");\
  printf(" pts %f %f %f %f %f %f %f %f %f %f\n", x + 0.000000, y + 0.950000, x + 0.000000, y + 0.500000, x + 1.750000, y + 0.500000, x + 1.750000, y + 0.950000, x + 0.000000, y + 0.950000);\
  printf(" pts %f %f %f %f\n", x + 0.250000, y + 1.450000, x + 0.400000, y + 1.450000);\
  printf(" newcurve linetype solid linethickness .7 marktype none\n");\
  printf(" pts %f %f %f %f %f %f %f %f %f %f\n", x + 1.550000, y + 1.450000, x + 2.000000, y + 1.450000, x + 2.000000, y + 1.000000, x + 1.750000, y + 0.500000, x + 1.750000, y + 0.950000);\
  printf(" pts %f %f\n", x + 2.000000, y + 1.450000);\
  printf("\n");\
  printf(" (* Knobs *)\n");\
  printf(" newcurve marktype circle fill 0 marksize .07 linetype none\n");\
  printf(" pts %f %f %f %f %f %f\n", x + 0.150000, y + 0.600000, x + 0.250000, y + 0.600000, x + 0.350000, y + 0.600000);\
  printf("\n");\
  printf(" newcurve marktype box fill 0 marksize .5 .45 pts %f %f\n", x + 0.800000, y + 0.725000);\
  printf(" newcurve marktype circle fill 1 marksize .07 linetype none\n");\
  printf(" pts %f %f\n", x + 0.950000, y + 0.850000);\
  printf("\n");\
  printf(" newcurve marktype box fill 0 marksize .4 .025 linetype none\n");\
  printf(" pts %f %f %f %f\n", x + 1.400000, y + 0.800000, x + 1.400000, y + 0.700000);\
  printf("\n");\
  printf(" (* Terminal *)\n");\
  printf(" newcurve marktype box fill 1 marksize 1 1 pts %f %f\n", x + 0.900000, y + 1.600000);\
  printf(" newcurve marktype box fill .8 marksize .8 .7 pts %f %f\n", x + 0.900000, y + 1.650000);\
  printf(" newline pts %f %f %f %f\n", x + 0.600000, y + 1.900000, x + 1.200000, y + 1.900000);\
  printf(" newline pts %f %f %f %f\n", x + 0.600000, y + 1.850000, x + 1.000000, y + 1.850000);\
  printf(" newline pts %f %f %f %f\n", x + 0.600000, y + 1.800000, x + 0.700000, y + 1.800000);\
  printf(" newline pts %f %f %f %f\n", x + 0.600000, y + 1.750000, x + 1.000000, y + 1.750000);\
  printf(" newline pts %f %f %f %f\n", x + 0.600000, y + 1.700000, x + 0.800000, y + 1.700000);\
  printf(" newline pts %f %f %f %f\n", x + 0.600000, y + 1.650000, x + 1.200000, y + 1.650000);\
  printf(" newline pts %f %f %f %f\n", x + 0.600000, y + 1.600000, x + 0.600000, y + 1.600000);\
  printf(" newline pts %f %f %f %f\n", x + 0.600000, y + 1.550000, x + 0.800000, y + 1.550000);\
  printf(" newline pts %f %f %f %f\n", x + 0.600000, y + 1.500000, x + 0.800000, y + 1.500000);\
  printf(" newline pts %f %f %f %f\n", x + 0.600000, y + 1.450000, x + 1.100000, y + 1.450000);\
  printf(" newline pts %f %f %f %f\n", x + 0.600000, y + 1.400000, x + 1.100000, y + 1.400000);\
  printf(" newcurve linetype solid linethickness .7 marktype none\n");\
  printf(" pts %f %f %f %f %f %f %f %f %f %f\n", x + 0.400000, y + 2.100000, x + 0.550000, y + 2.300000, x + 1.550000, y + 2.300000, x + 1.550000, y + 1.300000, x + 1.400000, y + 1.100000);\
  printf(" pts %f %f %f %f\n", x + 1.400000, y + 2.100000, x + 1.550000, y + 2.300000);\
  printf("\n");\
  printf(" newcurve marktype circle fill 0 marksize .07 linetype none\n");\
  printf(" pts %f %f %f %f %f %f\n", x + 1.000000, y + 1.200000, x + 1.100000, y + 1.200000, x + 1.200000, y + 1.200000);\
  printf("\n");\
  printf(" (* Keyboard *)\n");\
  printf("\n");\
  printf(" newcurve linetype solid linethickness .7 marktype none\n");\
  printf(" pts %f %f %f %f %f %f %f %f %f %f\n", x + 0.200000, y + 0.050000, x + 0.200000, y + 0.000000, x + 1.300000, y + 0.000000, x + 1.300000, y + 0.050000, x + 0.200000, y + 0.050000);\
  printf(" pts %f %f %f %f %f %f %f %f %f %f\n", x + 0.350000, y + 0.350000, x + 1.450000, y + 0.350000, x + 1.450000, y + 0.300000, x + 1.300000, y + 0.000000, x + 1.300000, y + 0.050000);\
  printf(" pts %f %f\n", x + 1.450000, y + 0.350000);\
  printf("\n");\
  printf(" newcurve linetype solid linethickness .7 marktype none\n");\
  printf(" pts %f %f %f %f %f %f %f %f %f %f\n", x + 0.300000, y + 0.120000, x + 0.380000, y + 0.280000, x + 1.280000, y + 0.280000, x + 1.200000, y + 0.120000, x + 0.300000, y + 0.120000);\
  printf(" newcurve linetype solid linethickness .7 marktype none\n");\
  printf(" pts %f %f %f %f\n", x + 0.326700, y + 0.173300, x + 1.226700, y + 0.173300);\
  printf(" newcurve linetype solid linethickness .7 marktype none\n");\
  printf(" pts %f %f %f %f\n", x + 0.353300, y + 0.226700, x + 1.253300, y + 0.226700);\
  printf(" newcurve linetype solid linethickness .7 marktype none\n");\
  printf(" pts %f %f %f %f %f %f %f %f\n", x + 0.400000, y + 0.120000, x + 0.480000, y + 0.280000, x + 0.580000, y + 0.280000, x + 0.500000, y + 0.120000);\
  printf(" pts %f %f %f %f %f %f %f %f\n", x + 0.600000, y + 0.120000, x + 0.680000, y + 0.280000, x + 0.780000, y + 0.280000, x + 0.700000, y + 0.120000);\
  printf(" pts %f %f %f %f %f %f %f %f\n", x + 0.800000, y + 0.120000, x + 0.880000, y + 0.280000, x + 0.980000, y + 0.280000, x + 0.900000, y + 0.120000);\
  printf(" pts %f %f %f %f %f %f %f %f\n", x + 1.000000, y + 0.120000, x + 1.080000, y + 0.280000, x + 1.180000, y + 0.280000, x + 1.100000, y + 0.120000);\
  printf("\n");\
  printf(" newcurve linetype solid linethickness .7 marktype none\n");\
  printf(" pts %f %f %f %f %f %f %f %f %f %f\n", x + 0.310000, y + 0.270000, x + 0.010000, y + 0.300000, x + 0.000000, y + 0.310000, x + -0.100000, y + 0.600000, x + 0.000000, y + 0.900000);\
  printf("\n");\
  printf("\n");\
}
