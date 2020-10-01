BEGIN	{ printf("{ x = $1; y = $2; \\\n"); }
	{ inpts = 0; instr = 0; npts = 0; inx = 0; \
          iny = 0;\
          printf("  printf(\"");\
          for (i = 1; i <= NF; i++) {\
            if (instr) { \
              printf(" %s", $i); \
            } else if (inpts > 0) {\
              printf(" %%f");\
              pts[npts] = $i;\
              if (inpts % 2) x[npts] = "x"; else x[npts] = "y";\
              inpts++;\
              npts++;\
            } else if (inx) {\
              printf(" %%f");\
              pts[npts] = $i;\
              x[npts] = "x";\
              npts++;\
              inx = 0;\
            } else if (iny) {\
              printf(" %%f");\
              pts[npts] = $i;\
              x[npts] = "y";\
              npts++;\
              iny = 0;\
            } else {\
              printf(" %s", $i);\
              if ($i == ":") instr = 1;\
              if ($i == "x") inx = 1;\
              if ($i == "y") iny = 1;\
              if ($i == "pts") inpts = 1; \
            }\
          }\
          printf("\\n\"");\
          if (npts > 0) {\
            for (i = 0; i < npts; i++) {\
              printf(", %s + %f", x[i], pts[i]);\
            }\
          }\
          printf(");\\\n");\
        }
END	{ printf("}\n"); }
