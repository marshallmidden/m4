#!/bin/sh

awk '

BEGIN {
	xmarksize = ymarksize = .3
	marktype = "circle"
	vfont = "Times-Roman"
	vfontsize = 8
	efont = "Times-Roman"
	efontsize = 8
	fill = 1.
	xs = ys = 1.
	xmin = ymin = 999999.
	xmax = ymax = -999999.
	n = 0
	m = 0
	mode = 0
	NOLABEL = "_NOLABEL"
	pi = 3.141593
	deg90 = pi / 2.
	elabdist = .1
	vlab = elab = 1
	larrows = rarrows = 0
}

NF == 0 || substr($1, 1, 1) == "#" { next }

$1 == "xmarksize" { xmarksize = $2 ; next }
$1 == "ymarksize" { ymarksize = $2 ; next }
$1 == "marksize" { xmarksize = ymarksize = $2 ; next }
$1 == "marktype" { marktype = $2 ; next }
$1 == "vfont" { vfont = $2 ; next }
$1 == "vfontsize" { vfontsize = $2 ; next }
$1 == "efont" { efont = $2 ; next }
$1 == "efontsize" { efontsize = $2 ; next }
$1 == "font" { vfont = efont = $2 ; next }
$1 == "fontsize" { vfontsize = efontsize = $2 ; next }
$1 == "fill" { fill = $2 ; next }
$1 == "xs" { xs = $2 ; next }
$1 == "ys" { ys = $2 ; next }
$1 == "scale" { xs = ys = $2 ; next }
$1 == "elabdist" { elabdist = $2 ; next }
$1 == "vlab" { vlab = $2 ; next }
$1 == "elab" { elab = $2 ; next }
$1 == "lab" { vlab = elab = $2 ; next }
$1 == "larrows" { larrows = $2 ; next }
$1 == "rarrows" { rarrows = $2 ; next }
$1 == "arrows" { larrows = rarrows = $2 ; next }

$1 == "edges" {
	mode = 1
	printf "newgraph\n"
	printf "xaxis min %f max %f size %f nodraw\n", xmin, xmax, (xmax - xmin) * xs
	printf "yaxis min %f max %f size %f nodraw\n\n", ymin, ymax, (ymax - ymin) * ys
	next
}

$1 == "raw" { mode = 2 ; next }

{
	if (!mode) {
		x[n] = $1
		y[n] = $2
		vlabel[n] = (NF == 3 ? $3 : n)
		if (x[n] < xmin) xmin = x[n]
		else if (x[n] > xmax) xmax = x[n]
		if (y[n] < ymin) ymin = y[n]
		else if (y[n] > ymax) ymax = y[n]
		n++
	}
	else if (mode == 1) {
		src[m] = label_to_node($1)
		dst[m] = label_to_node($2)
		elabel[m] = (NF == 3 ? $3 : NOLABEL)
		m++
	}
	else
		print $0
}

END {
	printf "\n"
	for (i = 0; i < n; i++) {
		printf "newline marktype %s marksize %f %f fill %f pts %f %f\n", marktype, xmarksize, ymarksize, fill, x[i], y[i]
		if (vlab)
			printf "newstring x %f y %f vjc hjc font %s fontsize %d : %s\n", x[i], y[i], vfont, vfontsize, vlabel[i]
	}
	printf "\n"
	for (i = 0; i < m; i++) {
		printf "newline marktype %s marksize %f %f fill %f pts %f %f %f %f ", marktype, xmarksize, ymarksize, fill, x[src[i]], y[src[i]], x[dst[i]], y[dst[i]]
		if (larrows) printf "larrows "
		if (rarrows) printf "rarrows "
		printf "\n"
		if (elab && elabel[i] != NOLABEL) {
			midx = (x[src[i]] + x[dst[i]]) / 2.
			midy = (y[src[i]] + y[dst[i]]) / 2.
			ang = atan2(y[dst[i]] - y[src[i]], x[dst[i]] - x[src[i]])
			nang = ang - deg90
			px = midx + elabdist * cos(nang)
			py = midy + elabdist * sin(nang)
			rot = ang * 180. / pi
			rot = (rot < 0. ? rot + 360. : rot)
			rot = (rot >= 90. && rot <= 270. ? rot + 180. : rot);
			printf "newstring x %f y %f vjc hjc rotate %f font %s fontsize %d : %s\n", px, py, rot, efont, efontsize, elabel[i]
		}
	}
}

function label_to_node(label) {
	for (i = 0; i < n; i++)
		if (vlabel[i] == label)
			return(i)
	return(-1)
}

'
