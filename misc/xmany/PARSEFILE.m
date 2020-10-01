# Format of PARSEFILE:
#-----------------------------------------------------------------------------
-from m.nts.umn.edu:0.0		# where keyboard and mouse are located
#-----------------------------------------------------------------------------
-nowrap	nsew
# -wrap	n
# -wrap	s
# -wrap	e
# -wrap	w
-wrap	ew
# -wrap	ns
# -wrap	nsew
# ?? 2000-04-04
#-----------------------------------------------------------------------------
-geom 1x1			# number of displays
# left
0,0	m.nts.umn.edu:0.1	# on_screen = 0
#-----------------------------------------------------------------------------
# Works 2000-04-04
# -geom 2x1			# number of displays
# left
# 0,0	l.nts.umn.edu:0.0	# on_screen = 0
# 2nd from left
# 1,0	n.nts.umn.edu:0.0	# on_screen = 1
#-----------------------------------------------------------------------------
# ?? 2000-04-04
# -geom 2x1			# number of displays
# left
# 0,0	m.nts.umn.edu:0.1	# on_screen = 0
# 2nd from left
# 1,0	n.nts.umn.edu:0.0	# on_screen = 1
#-----------------------------------------------------------------------------
# Broken 2000-03-28
# Test case --			6 across, 3 machines (from is one of them).
# -geom 6x1			# number of displays
# left
# 0,0	m.nts.umn.edu:0.2	# on_screen = 0
# 2nd from left
# 1,0	m.nts.umn.edu:0.0	# on_screen = 1
# 3rd from left
# 2,0	m.nts.umn.edu:0.1	# on_screen = 2
# 4th from left
# 3,0	n.nts.umn.edu:0.0	# on_screen = 3
# 4th from left
# 4,0	l.nts.umn.edu:0.0	# on_screen = 4
# right
# 5,0	l.nts.umn.edu:0.1	# on_screen = 5
#-----------------------------------------------------------------------------
# End of file PARSEFILE
