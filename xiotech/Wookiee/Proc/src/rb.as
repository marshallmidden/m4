# $Id: rb.as 143007 2010-06-22 14:48:58Z m4 $
#******************************************************************************
#
#  NAME: rb.as
#
#  PURPOSE:
#       To provide red-black interval tree related functions.
#
#       Red-Black trees are a self balancing binary search tree where
#       insert, delete and locations are inherently of order log2N where
#       N is the size in elements of the tree.  This tree can also
#       be used to determine if a specified interval overlaps any
#       other intervals stored in the tree.
#
#       NOTE:  As this tree is intended to be used as a database for
#              cached disk sectors it is assumed that none of the
#              intervals stored into the tree overlap.
#
#       The interval is specified by the key, <rbkey> as the lower bound with
#       the upper bound specified by the <rbkeym> field in the node.
#
#       The elements (nodes) are maintained with ascending order of the
#       64 bit key stored in each node.
#
#       An element with a null parent pointer is considered to be
#       the root of a tree.  An element with two NIL children and null parent is
#       considered to be the root of an empty tree.  New inserted elements are
#       always placed with NIL child pointers.  NIL is a pointer to a
#       special marker node, <._nil>.
#
#       This code is based on pseudo-code in a discussion of RB trees in
#
#       _Introduction to Algorithms_, Thomas Cormen, ISBN 0-262-03141-8
#                                     (MIT Press, 1990)
#
#  FUNCTIONS:
#       RB$insert   - Add element to tree.
#       RB$delete   - Delete element from tree.
#       RB$locate   - Locate element in tree.
#
#  Copyright (c) 1996 - 2008 Xiotech Corporation.  All rights reserved.
#
#******************************************************************************
#
# --- Global Functions --------------------------------------------------------
#
        .globl  ._nil
        .globl  RB$insert
        .globl  rb$ifixup
        .globl  RB$delete
        .globl  rb$dfixup
        .globl  rb$rleft
        .globl  rb$rright
        .globl  RB$locateNext
        .globl  RB$locateStart
        .globl  RB$foverlap
        .globl  RB$noverlap
        .globl  nil
#
# --- Local data --------------------------------------------------------------
#
        .section cds, data
#
._nil:
nil:
        .space   rbesize,0              # NIL placemarker
#
        .text
#
# --- Executable code ---------------------------------------------------------
#
#******************************************************************************
#
#  NAME: RB$insert
#
#  PURPOSE:
#       Insert an element into a RB tree.
#
#  DESCRIPTION:
#       This routine is used to insert a node into an existing RB tree.
#       The node is inserted in the tree in ascending order according to
#       the 64-bit key in each element.  After the insertion the tree
#       is rebalanced.
#
#       The node to be inserted must have the <rbkey> field set to the
#       desired lower bound and the <rbkeym> value set to
#       the endpoint of the range.
#
#  CALLING SEQUENCE:
#       call    RB$insert
#
#  INPUT:
#       g0 = address of root element of tree.
#       g1 = element to insert in tree.
#
#  OUTPUT:
#       g0 = possible new root of tree
#       g1 = T/F; FALSE = operation not successful (duplicate key)
#
#******************************************************************************
#
RB$insert:
        ldl     rbkey(g1),r12           # Get key value
        lda     ._nil,r8                # get NIL value
        ldconst 0,r10                   # Preset parent pointer to Null
        ldconst 0,r4                    # Preset  pointer to Null
        ldconst RED,r11                 # Preset RED color
# FIX #1, MSHDEBUG      mov     0,r15   # Preset parent pointer
#
# --- Locate future parent
#
        cmpo    0,g0                    # Check for null root (empty tree)
        mov     g0,r3                   # r3 = current node
        sele    g0,g1,g0                # Select current node if no root
        be      .rbi30                  # Jif only one node in tree
#
.rbi10:
#
        ldq     rbkey(r3),r4            # Get key value
                                        #     and left/right child pointers
        mov     r3,r10                  # Setup parent node pointer
#
# --- Check MSB of current to insertion node
#
        cmpobne r13,r5,.rbi20           # Jif MSBs differ
#
# --- Check for duplicate key condition
#
        cmpobe  r12,r4,.rbi100          # Jif duplicate key
#
# --- Select right child if insertion key  > node key; otherwise left child
#
.rbi20:
        selg    r6,r7,r3                # Select proper child pointer
        selg    rbcleft,rbcright,r4     # Get child pointer offset
        cmpobne r8,r3,.rbi10            # Jif pointer not NIL
#
# --- Insertion point found.  Insert node.
#
.rbi30:
        mov     r8,r9                   # Set up for long store
        addo    r4,r10,r4               # Formulate pointer to parent's child
                                        #  for this node
        stl     r8,rbcleft(g1)          # Set left child to NIL
                                        #     right child to NIL
        stl     r10,rbparent(g1)        #     parent value
                                        #     color to RED
#
        cmpobe  g1,g0,.rbi40            # Jif at root node
        st      g1,(r4)                 # Set correct parent child pointer to
                                        #  point to this new node
#
# --- Node is inserted into tree.  Now rebalance.
#
.rbi40:
        call    rb$ifixup               # Rebalance tree if necessary
        mov     TRUE,g1                 # Signal successful insertion
        ret
#
.rbi100:
        cmpo    0,0                     # Should never occur
        faulte                          # Fault so that it can be debugged
        ret
#
#******************************************************************************
#
#  NAME: rb$ifixup
#
#  PURPOSE:
#       Rebalance a RB tree if necessary after a node insertion.
#
#  DESCRIPTION:
#       This routine is used to rebalance a tree according to the
#       Red-Black rules after a node has been inserted into a tree.
#       This operation may recolor nodes and may cause rotations.
#       This operation completes in order log2N time where N
#       is the size of the tree in elements.
#
#  CALLING SEQUENCE:
#       call    rb$ifixup
#
#  INPUT:
#       g0 = root address of tree
#       g1 = address of newly inserted element in tree.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
rb$ifixup:
        ldconst BLACK,r15
        ldconst RED,r14
        mov     g1,r3                   # r3 = current node
#
.rbif10:
## V2: In rbi.as, the code is okay. However, I put this change there
## too to keep code same in both the files.
        cmpobe  g0,r3,.rbif100         # Jif at root node (done)
        ld      rbparent(r3),r4         # Get parent pointer
        ldob    rbcolor(r4),r5          # Get parent color
        cmpobne RED,r5,.rbif100        # Jif no red-red violation
        ld      rbparent(r4),r6         # Get grandparent's pointer
        ldl     rbcleft(r6),r8          # Get grandparent's left/right child pointer
#
# --- Red-Red violation exists.  Check uncle node for color.
#
        cmpobne r4,r8,.rbif50           # Jif parent is not g'parent's
                                        #  left child
#
# --- get uncle node
#
#       r9 = right uncle pointer
#
        ldob    rbcolor(r9),r8          # Load color from node
        cmpobe  BLACK,r8,.rbif30        # jif uncle BLACK
#
# --- Set parent color to BLACK and uncle to BLACK, and grandfather to RED
#
        stob    r15,rbcolor(r4)         # Set parent color
        stob    r15,rbcolor(r9)         # Set uncle color
        stob    r14,rbcolor(r6)         # Set grandparent color
#
# --- Move up to grandparent node and recheck for violation
#
        mov     r6,r3                   # Set current node to grandparent
        b       .rbif10                 # Check for violation
#
.rbif30:
#
# --- Uncle is BLACK
#
        ld      rbcright(r4),r9         # Check for right child
        cmpobne r3,r9,.rbif40           # Jif not right child
#
# --- Make this node a left child and rotate tree left at parent node
#
        mov     r4,r3                   # Current node is now parent
        mov     r3,g1                   # g1 = position in tree to rotate
        call    rb$rleft                # Rotate tree left
        ld      rbparent(r3),r4         # get new parent of x
        ld      rbparent(r4),r6         # Get grandparent of x
#
# --- Now recolor and rotate
#
.rbif40:
        stob    r15,rbcolor(r4)         # Set parent color to BLACK
        stob    r14,rbcolor(r6)         # Set grandparent color to RED
        mov     r6,g1                   # Rotate grandparent
        call    rb$rright               # Rotate tree right
        b       .rbif10                 # Check for further violation
#
# --- Mirror of above code for right child
#
.rbif50:
#
# --- get uncle node
#
#       r8 = uncle pointer
#
        ldob    rbcolor(r8),r9          # Load color from node
        cmpobe  BLACK,r9,.rbif70        # jif uncle BLACK
#
# --- Set parent color to BLACK and uncle to BLACK, and grandfather to RED
#
        stob    r15,rbcolor(r8)         # Set uncle color
        stob    r15,rbcolor(r4)         # Set parent color
        stob    r14,rbcolor(r6)         # Set grandparent color
#
# --- Move up to grandparent node and recheck for violation
#
        mov     r6,r3                   # Set current node to grandparent
        b       .rbif10                 # Check for violation
#
.rbif70:
#
# --- Uncle is BLACK
#
        ld      rbcleft(r4),r9          # Check for left child
        cmpobne r3,r9,.rbif80           # Jif not left child
#
# --- Make this node a right child and rotate tree right at parent node
#
        mov     r4,r3                   # Current node is now parent
        mov     r3,g1                   # g0 = position in tree to rotate
        call    rb$rright               # Rotate tree right
        ld      rbparent(r3),r4         # get new parent of x
        ld      rbparent(r4),r6         # Get grandparent of x
#
# --- Now recolor and rotate
#
.rbif80:
        stob    r15,rbcolor(r4)         # Set parent color to BLACK
        stob    r14,rbcolor(r6)         # Set grandparent color to RED
        mov     r6,g1                   # Rotate grandparent
        call    rb$rleft                # Rotate tree left
        b       .rbif10                 # Check for further violation
#
.rbif100:
        stob    r15,rbcolor(g0)         # Set root color to BLACK
        ret                             # Done!
#
#******************************************************************************
#
#  NAME: RB$delete
#
#  PURPOSE:
#       Delete an element from a RB tree.
#
#  DESCRIPTION:
#       This routine is used to delete a node from an existing RB tree.
#       After the deletion the tree is rebalanced.
#
#  CALLING SEQUENCE:
#       call    RB$delete
#
#  INPUT:
#       g0 = address of root element of tree.
#       g1 = pointer to element to delete.
#
#  OUTPUT:
#       g0 = possible new root pointer; zero if tree empty.
#
#  REGS DESTROYED:
#       g0-g8
#
#******************************************************************************
#
RB$delete:
#
#     r15 = NIL
#     r14 = y
#     r13 = x
#     g1  = z   (node to be deleted)
#
# --- Check for right or left child = NIL
#
        ldl     rbcleft(g1),r4          # Get left and right child pointers
        lda     ._nil,r15               # Get NIL pointer
        cmpobe  r15,r4,.rbd60           # Jif left child NIL
        cmpobne r15,r5,.rbd80           # Jif right child not NIL
#
# - Set y to z, node can be spliced out.
#
.rbd60:
        mov     g1,r14                  # Set y = z
#
        b       .rbd90
#
.rbd80:
#
# --- Both children of z (node to be deleted) are present, find inorder
#     successor and assign to y.  Find successive left children of
#     original right child of z until left pointer = NIL.
#
        mov     r5,r14                  # Preserve pointer (set to y)
        ld      rbcleft(r5),r5          # Get left child pointer
        cmpobne r15,r5,.rbd80           # Jif pointer not NIL
#
.rbd90:
        ldt     rbcleft(r14),r4         # Get left child/right/parent of y
#
# --- if left(y) != NIL, set x to left(y), else x = right(y)
#
        cmpo    r4,r15                  # Check for NIL
        sele    r4,r5,r13               # set x appropriately
#
        st      r6,rbparent(r13)        # set parent of x to parent of y
#
# --- Note: x may in this case be the nil sentinel, <._nil>, and we are
#           indeed setting its parent.  This is necessary because the
#           fixup routine may be called with the nil node x as its input
#           and will need the parent to be set correctly.
#
        cmpobne 0,r6,.rbd105            # Check for parent of y == 0;   MSHDEBUG FIX #4
                                        #   if so, at root and need     MSHDEBUG FIX #4
                                        #   to set new root value       MSHDEBUG FIX #4
#                                                                       MSHDEBUG FIX #4
# --- At the root, Check to see if x == Nil which means the last node has MSHDEBUG FIX #4
#     been deleted from the tree.  If so, change the root to 0 (Null),  MSHDEBUG FIX #4
#     else root = x.                                                    MSHDEBUG FIX #4
#                                                                       MSHDEBUG FIX #4
        cmpo    r15,r13                 # Test if x == Nil              MSHDEBUG FIX #4
        sele    r13,0,g0                # If Nil set root=0, else root=xMSHDEBUG FIX #4
        be      .rbd150                 # Jif was NIL, tree empty, exit MSHDEBUG FIX #4
        b       .rbd110                 # Jif was NIL                   MSHDEBUG FIX #4
#
# --- Find whether y is left or right child of parent
#
#     r6 = parent pointer
#
.rbd105:                                #                               MSHDEBUG FIX #4
        ld      rbcleft(r6),r8          # Get left child of parent
        cmpo    r8,r14                  # Check if left child
        sele    rbcright,rbcleft,r3     # select left/right
#
# --- Want to reset the pointer from the original parent to splice out this
#     node;  x is new node to replace y.
#
        addo    r3,r6,r3                # Add to pointer to parent
        st      r13,(r3)                #  and set child pointer
#
.rbd110:
        ldob    rbcolor(r14),r11        # Get color of old node
        cmpobe  r14,g1,.rbd140          # Jif y == z; node was spliced out
                                        #  and don't need to replace with
                                        #  inorder successor
#
# --- y points to a inorder successor node.  Need to swap these nodes (change
#     pointers of y to match those of z, and parent of child nodes)
#     Done to preserve any external pointers that point to this node.
#     MSHDEBUG - This next lines wipes out the color of Y that was loaded above
# MSHDEBUG FIX #3       ldob    rbcolor(g1),r11         # Get original color of z
        ldob    rbcolor(g1),r12         # Get original color of z               MSHDEBUG FIX #3
        stob    r12,rbcolor(r14)        # and copy color to replacement node y  MSHDEBUG FIX #3
        ldt     rbcleft(g1),r4          # get left/right/parent pointers
#        ld      rbparent(r14),r12       # get original parent pointer  MSHDEBUG - Why, not used?
        stt     r4,rbcleft(r14)         # Update left/right/parent
                                        #   of node y (replacing node z)
#        cmpobe  r15,r4,.rbd120          # jif left NIL  MSHDEBUG FIX #5
        st      r14,rbparent(r4)        # update left child parent
# .rbd120:
#        cmpobe  r15,r5,.rbd130          # jif right NIL MSHDEBUG FIX #5
        st      r14,rbparent(r5)        # update right child parent
# .rbd130:
#
# --- Check for parent(x) == z; if true set parent(x) to y.
#
#     r14 = y
#     r13 = x  MSHDEBUG - ?? Not need, already corrected by previous 4 lines
#     g1  = z
#
#        ld      rbparent(r13),r8                               MSHDEBUG - ?? Not need, already corrected by previous 4 lines
#        cmpobne r8,g1,.rbd131           # Jif parent(x) != z   MSHDEBUG - ?? Not need, already corrected by previous 4 lines
#        st      r14,rbparent(r13)       # Set parent(x) to y   MSHDEBUG - ?? Not need, already corrected by previous 4 lines
#
# .rbd131:
#
# --- check for z == root; if true reset root to new node.
#
        cmpo    g0,g1
        sele    g0,r14,g0               # Reset root if necessary
#        cmpobe  0,r6,.rbd140            # Jif parent root  - MSHDEBUG, another cmp not needed, just be
        be      .rbd140                 # Jif parent root  - MSHDEBUG, replace cmpobe with be
#
# --- Find whether z was left or right child of parent
#
#     r6 = parent pointer
#
        ld      rbcleft(r6),r8          # Get left child of parent
        cmpo    r8,g1                   # Check if left child
        sele    rbcright,rbcleft,r3     # select left/right
#
# --- Want to reset the pointer from the original parent to splice out this
#     node;  x is new node to replace y.   MSHDEBUG - Comment should be y is new node to replace z.
#
        addo    r3,r6,r3                # Add to pointer to parent
        st      r14,(r3)                #  and set child pointer
# MSHDEBUG FIX #3        stob    r11,rbcolor(r14)        # Set new node to old color
#
# --- Check color of old node and if black need to adjust tree.
#
.rbd140:
#
# MSHDEBUG FIX #3       ldob    rbcolor(r14),r11        # get color of y
        cmpobne BLACK,r11,.rbd150       # Jif not black; no need to fix
                                        #  tree
#
# --- g0 = root of tree
#     g1 = node to fixup
#
        mov     r13,g1                  # Fixup at x
        b       rb$dfixup               # Readjust tree/return
#
.rbd150:
        ret                             # Done
#
#******************************************************************************
#
#  NAME: rb$dfixup
#
#  PURPOSE:
#       Maintain the tree red-black balance after a node deletion.
#
#  DESCRIPTION:
#       This routine restores the balance of a rb interval tree after
#       a node has been deleted.  It performs recoloring and rotations
#       as necessary to achieve this.  Register g0 contains a pointer
#       to the root of the tree; register g1 contains a pointer to the
#       node that was affected by the deletion.
#
#       Upon exit, register g0 may have been changed to reflect the
#       new root of the tree.
#
#       This code is based on pseudo-code in a discussion of RB trees in
#
#       _Introduction to Algorithms_, Thomas Cormen, ISBN 0-262-03141-8
#                                     (MIT Press, 1990)
#
#  CALLING SEQUENCE:
#       call    rb$dfixup
#
#  INPUT:
#       g0 = address of root of tree
#       g1 = Address of node to check tree integrity
#
#  OUTPUT:
#       g0 = root of tree; may have been changed from input value
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
rb$dfixup:
        ldconst BLACK,r15
        ldconst RED,r14
        mov     g1,r13
#
.rbdf10:
        ldob    rbcolor(r13),r3         # Get x node color
        cmpobne BLACK,r3,.rbdf100       # Jif not black; done
        cmpobe  g0,r13,.rbdf100         # Jif at root
#
        ld      rbparent(r13),r12       # Get parent of node
        ldl     rbcleft(r12),r6         # Get left/right child of parent
        cmpobne r6,r13,.rbdf50          # Jif not left child of parent
#
        ldob    rbcolor(r7),r8          # get color of right uncle
        cmpobne RED,r8,.rbdf20          # Jif not RED
#
        stob    r15,rbcolor(r7)         # set color to black
        stob    r14,rbcolor(r12)        # set parent color to red
#
# --- Rotate tree left at parent
#
        mov     r12,g1                  # g1 = node to rotate
        call    rb$rleft                # Rotate tree left around parent of x
        ld      rbparent(r13),r12       # get new x parent
        ld      rbcright(r12),r7        # get new right child of x parent
#
# --- r13 = x
#     r12 = parent of x
#     r7  = w
#
.rbdf20:
        ldl     rbcleft(r7),r8          # get w left/right child
        ldob    rbcolor(r8),r10         # get left child color
        ldob    rbcolor(r9),r11         # get right child color
#
# --- check for left and right nodes of w BLACK
#
        cmpobne r10,r11,.rbdf30         # jif not same color
        cmpobne BLACK,r10,.rbdf30       # jif not BLACK
#
# --- set color of w to RED
#
        stob    r14,rbcolor(r7)         # set color of w to RED
        mov     r12,r13                 # set x to parent of x
        b       .rbdf10                 # Iterate at new x
#
.rbdf30:
        cmpobne BLACK,r11,.rbdf40       # jif right child not BLACK
#
        stob    r15,rbcolor(r8)         # Set left child color to BLACK
        stob    r14,rbcolor(r7)         # set color of w to RED
        mov     r7,g1
        call    rb$rright               # Rotate tree right at w
#
        ld      rbparent(r13),r12       # get new x parent
        ld      rbcright(r12),r7        # get new right child of x parent
#
.rbdf40:
        ldob    rbcolor(r12),r3         # get color of parent of x
        ld      rbparent(r7),r4         # get parent of w
        ld      rbcright(r7),r5         # get right child of w
        stob    r3,rbcolor(r7)          # set color of w to color of x parent
        stob    r15,rbcolor(r4)         # set parent color to BLACK
        stob    r15,rbcolor(r5)         # set right child of w color to BLACK
        mov     r12,g1                  # Rotate left at x parent
        call    rb$rleft
        mov     g0,r13                  # set x to root
#
        b       .rbdf100                # Done!
#
# --- Mirror image of .rbdf20 - .rbdf40
#
.rbdf50:
        mov     r6,r7                   # w = r7
#
        ldob    rbcolor(r7),r8          # get color of left uncle
        cmpobne RED,r8,.rbdf60          # Jif not RED
#
        stob    r15,rbcolor(r7)         # set color to black
        stob    r14,rbcolor(r12)        # set parent color to red
#
# --- Rotate tree right at parent
#
        mov     r12,g1                  # g1 = node to rotate
        call    rb$rright               # Rotate tree right
        ld      rbparent(r13),r12       # get new x parent
        ld      rbcleft(r12),r7         # get new left child of x parent
#
# --- r13 = x
#     r12 = parent of x
#     r7  = w
#
.rbdf60:
        ldl     rbcleft(r7),r8          # get w left/right child
        ldob    rbcolor(r8),r10         # get left child color
        ldob    rbcolor(r9),r11         # get right child color
#
# --- check for left and right nodes of w BLACK
#
        cmpobne r10,r11,.rbdf70          # jif not same color
        cmpobne BLACK,r10,.rbdf70        # jif not BLACK
#
# --- set color of w to RED
#
        stob    r14,rbcolor(r7)         # set color of w to RED
        mov     r12,r13                 # set x to parent of x
        b       .rbdf10                 # Iterate at new x
#
.rbdf70:
        cmpobne BLACK,r10,.rbdf80       # jif left child not BLACK
#
        stob    r15,rbcolor(r9)         # Set right child color to BLACK
        stob    r14,rbcolor(r7)         # set color of w to RED
        mov     r7,g1
        call    rb$rleft                # Rotate tree left at w
#
        ld      rbparent(r13),r12       # get new x parent
        ld      rbcleft(r12),r7         # get new left child of x parent
#
.rbdf80:
        ldob    rbcolor(r12),r3         # get color of parent of x
        ld      rbparent(r7),r4         # get parent of w
        ld      rbcleft(r7),r5          # get left child of w
        stob    r3,rbcolor(r7)          # set color of w to color of x parent
        stob    r15,rbcolor(r4)         # set parent color to BLACK
        stob    r15,rbcolor(r5)         # set left child of w color to BLACK
        mov     r12,g1                  # Rotate right at x parent
        call    rb$rright
        mov     g0,r13                  # set x to root
#
.rbdf100:
        stob    r15,rbcolor(r13)        # set color of x to BLACK
        ret
#
#******************************************************************************
#
#  NAME: rb$rleft
#
#  PURPOSE:
#       Rotate a tree left at specified position.
#
#  DESCRIPTION:
#       This routine is used to perform a leftwise rotation at the
#       location specified in register g1.
#
#  CALLING SEQUENCE:
#       call    rb$rleft
#
#  INPUT:
#       g0 = address of root of tree
#       g1 = address of element to rotate left
#
#  OUTPUT:
#       g0 = possible new root of tree
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
rb$rleft:
        ldt     rbcleft(g1),r4          # Get pointer to left/right child
                                        #     and pointer to parent
        lda     ._nil,r15               # get NIL pointer value
        ld      rbcleft(r5),r8          # get left pointer of right child
        st      r8,rbcright(g1)         #  and store as right pointer
                                        #  in specified node
#
        cmpobe  r8,r15,.rbrl10          # Jif left pointer of right child
                                        #  NIL
#
        st      g1,rbparent(r8)         # set node as parent
.rbrl10:
        cmpobe  r5,r15,.rbrl20          # Jif right child of node is NIL
        st      r6,rbparent(r5)         # Set parent of right child
                                        #  to parent of current node
.rbrl20:
        cmpobe  0,r6,.rbrl30            # Jif parent null (root)
        ld      rbcleft(r6),r8          # get left child pointer of parent
        cmpo    r8,g1                   # check for left child
        addone  (rbcright-rbcleft),r6,r6 # set correct offset
#
        st      r5,rbcleft(r6)          # move right child to parent
        b       .rbrl40
#
# --- Parent pointer was nil; assume root; also assume r6 = 0
#
.rbrl30:
        st      r6,rbparent(r5)
        mov     r5,g0                   # Set root to right child of node
.rbrl40:
#
# --- Link the two nodes together
#
        st      g1,rbcleft(r5)
        st      r5,rbparent(g1)         # Set parent of this node
#
        ret
#
#******************************************************************************
#
#  NAME: rb$rright
#
#  PURPOSE:
#       Rotate a tree right at specified position.
#
#  DESCRIPTION:
#       This routine is used to perform a rightwise rotation at the
#       location specified in register g1.
#
#  CALLING SEQUENCE:
#       call    rb$rright
#
#  INPUT:
#       g0 = address of root of tree
#       g1 = address of element to rotate left
#
#  OUTPUT:
#       g0 = possible new root of tree
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
rb$rright:
        ldt     rbcleft(g1),r4          # get pointer to left/right child
                                        #     and pointer to parent
        lda     ._nil,r15               # get NIL pointer value
        ld      rbcright(r4),r8         # get right pointer of left child
        st      r8,rbcleft(g1)          #  and store as left pointer
                                        #  in specified node
        cmpobe  r8,r15,.rbrr10          # Jif right pointer of left child
                                        #  NIL
#
        st      g1,rbparent(r8)         # set node as parent
.rbrr10:
        cmpobe  r4,r15,.rbrr20          # Jif left child of node is NIL
        st      r6,rbparent(r4)         # Set parent of left child
                                        #  to parent of current node
.rbrr20:
        cmpobe  0,r6,.rbrr30            # Jif parent null (root)
        ld      rbcright(r6),r8         # get right child pointer of parent
        cmpo    r8,g1                   # check for right child
        addoe   (rbcright-rbcleft),r6,r6 # set correct offset
#
        st      r4,rbcleft(r6)          # move left child to parent
        b       .rbrr40
#
# --- Parent pointer was nil; assume root; also assume r6 = 0
#
.rbrr30:
        st      r6,rbparent(r4)
        mov     r4,g0                   # Set root to left child of node
.rbrr40:
#
# --- Link the two nodes together
#
        st      g1,rbcright(r4)
        st      r4,rbparent(g1)         # Set parent of this node
        ret
#
#******************************************************************************
#
#  NAME: RB$locateNext
#
#  PURPOSE:
#       Locate the successor of an element in a RB interval tree.
#
#  CALLING SEQUENCE:
#       call    RB$locateNext
#
#  INPUT:
#       g1 = address of an element of tree.
#
#  OUTPUT:
#       g1 = address of successor, FALSE (0) returned if no successor
#
#******************************************************************************
#
RB$locateNext:
#
#     r15 = NIL
#     g1  = node whose successor is to be found
#     g1  = successor
#
# --- The address of the nil node is stored in R15 for future use
#
        lda     ._nil,r15               # store address of NIL
#
# --- Check if the node has a right child.  When the node has
# --- a right child, the successor is the node with minimum
# --- key in the right sub-tree.
#
        ld      rbcright(g1),r4         # Get right child
        cmpibe  r4,r15,.rbln20          # Check if right child is NIL
#
# --- Follow the path down the left children to
# --- the node with the minimum key.
#
.rbln10:
        mov     r4,g1                   # set successor equal child
        ld      rbcleft(g1),r4          # Get left child
        cmpibne r4,r15,.rbln10          # Check if left is NIL
        ret
#
# --- When node has no right child, the successor is the lowest ancestor
# --- whose left child is also a ancestor of node.  Go up the tree until
# --- until we reach a node that is the left child of it's parent.
#
.rbln20:
        mov     g1,r4                   # save child node
        ld      rbparent(g1),g1         # get parent
        cmpobe  0,g1,.rbln30            # Check if parent is null, exit MSHDEBUG
        ld      rbcleft(g1),r8          # Get left child
        cmpobne r4,r8,.rbln20           # Check if left child of parent
.rbln30:
        ret
#
#******************************************************************************
#
#  NAME: RB$locateStart
#
#  CALLING SEQUENCE:
#       call    RB$locateStart
#
#  INPUT:
#       g0 = address of root element of tree.
#       g2 = Key LSB
#       g3 = Key MSB
#
#  OUTPUT:
#       g1 = pointer to located element; if not found returned as NIL
#
#  REGS DESTROYED:
#       g0
#
#******************************************************************************
#
RB$locateStart:
#
#     g0  = node whose successor is to be found
#     g1  = successor
#
#     r4, r5 = Key
#     r6 = root
#     r15 = NIL
#
        mov     g0,r6                   # save the root
#
# --- The address of the nil node is stored in R15 for future use
#
        lda     ._nil,r15               # store address of NIL
        mov     r15,g1                  # g1 = ???
#
# --- Check for an empty tree (the root is null)
#
        cmpibne 0,g0,.rbls20
        ret
#
# --- The input key is less than the key of the current node.
#     Save the parent of the left child and select left child
#
.rbls10:
        mov     g0,g1                   # zb = z
        ld      rbcleft(g0),g0          # get left child
        cmpibe  g0,r15,.rbls40          # Jif child is nil
#
# --- Check MSB key of current node to search key MSB
#
.rbls20:
        ld      rbkey+4(g0),r5          # get MSB key of current node
        cmpobl  g3,r5,.rbls10           # Jif key less
        cmpobg  g3,r5,.rbls30           # Jif key greater
#
# --- Check LSB key of current node to search key LSB
#
        ld      rbkey(g0),r4
        cmpobl  g2,r4,.rbls10           # Jif key less
# MSHDEBUG FIX #2       cmpobe  g2,r4,.rbls40           # Jif key is equal
        cmpobg  g2,r4,.rbls30           # Jif key greater
#
# --- The input key is equal to the key of the current node.
#     Output this node.
#
        mov     g0,g1
        ret
#
# --- The input key is greater than the key of the current node.
#     Select the right child
#
.rbls30:
        ld      rbcright(g0),g0         # get right child
        cmpibne g0,r15,.rbls20          # Jif child is not nil
#
# --- Locate is complete.  Check if the end of the tree was reached.
# --- If so, the locate wraps to the beginning of the tree.
#
.rbls40:
        cmpibne g1,r15,.rbls60          # Jif if return node is not nil
#
# --- Start with the root and follow the path down
# --- the left children to the node with the minimum key.
#
.rbls50:
        mov     r6,g1                   # set successor equal child
        ld      rbcleft(g1),r6          # Get left child
        cmpibne r6,r15,.rbls50          # Check if left is NIL

.rbls60:
        mov     g1,g0                   # MSHDEBUG, C Intf
        ret
#
#******************************************************************************
#
#  NAME: RB$foverlap
#
#  PURPOSE:
#       Locate first node (smallest key) in the RB tree that overlaps the
#       supplied key range.
#
#  DESCRIPTION:
#       Starting at the root of the given tree, this routine locates the first
#       node in the tree that overlaps the specified key range.  Register g1
#       is returned as the first node that overlaps; FALSE indicates that no
#       overlap was found.  "First" implies the lowest lower key value in the
#       tree that overlaps the supplied range. After an initial overlapping
#       node is found, the algorithm to find to first node alternates between
#       searching the left or right children until the bottom of the tree
#       (Nil pointer) is reached.  If the next node contains an overlap,
#       the search continues with the left child attempting to find an earlier
#       overlap. If the next node does not contain an overlap, the search continues
#       with the right child attempting to find another overlap again that is
#       earlier than the last one that was found.
#
#  CALLING SEQUENCE:
#       call    RB$foverlap
#
#  INPUT:
#       g0    = Address of root of interval tree
#       g2/g3 = Min Key value to detect overlap
#       g4/g5 = Max Key value for range
#
#  OUTPUT:
#       g1    = FALSE or address of located node
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
RB$foverlap:
#
# RB_NODE_PTR RB$FirstOverlap(RB_NODE_PTR node, unsigned int startLba, unsigned int endLba)
# {
#    RB_NODE_PTR     overlapNode;
# }
#
# --- Check to ensure that the tree is not empty.
#
        cmpobe  0,g0,.rbfo900           # Jif RB tree is empty (NULL)
#
# --- The RB tree is not empty.
#     Perform some initialization prior to the loop to search for
#     an overlapping node.
#       r3 =  pointer to current node to search
#       r15 = pointer to Nil node
#
        mov     g0,r3                   # r3 = pointer to current node
        lda     ._nil,r15               # load address of NIL node
#
.rbfo100:
# --- Loop to find a node with an overlap condition.  This loop will find
#     the upper most node (closest to the root) with an overlap.
#       do { } while (node != &nil)
#
# --- Load the Key, Left Child, and Right Child for the current node
#       r4 = LSB Key
#       r5 = MSB Key
#       r6 = Left Child Pointer
#       r7 = Right Child Pointer
#
        ldq     rbkey(r3),r4            # load key LSB, MSB, lchild, rchild
#
# --- If the Ending LBA is less than the the start key value for the node,
#     then an overlap does not exist with this node.  Continue to search
#     the left child sub-tree.
#        if (endLba < node->key)
#
        cmpobne g5,r5,.rbfo200          # Compare MSBs, Jif MSBs differ
        cmpo    g4,r4                   # MSBs equal, Compare LSBs
.rbfo200:
        bge     .rbfo300                # Jif endLba >= key
#
# --- Ending LBA is less than the starting key value.
#     Continue to search the left child sub-tree.
#       node = node->leftChild
#
        mov     r6,r3                   # next node is left child
        b       .rbfo600                # branch to test if Nil
#
#
.rbfo300:
# --- Else If the Starting LBA is greater than the ending key
#     value for the node, then an overlap does not exist
#     with this node.  Continue to search the right child sub-tree.
#       else if (startLba > node->endKey)
        ldl     rbkeym(r3),r8           # Load endKey from RB node
        cmpobne g3,r9,.rbfo400          # Compare MSBs, Jif MSBs differ
        cmpo    g2,r8                   # MSBs equal, Compare LSBs
.rbfo400:
        ble     .rbfo500                # Jif startLba <= endKey
#
# --- Starting LBA is greater than or equal to the ending key value.
#     Continue to search the right child sub-tree.
#       node = node->rightChild
#
        mov     r7,r3                   # next node is right child
#
.rbfo600:
# --- Continue to search for the upper most node with an overlap until
#     the bottom of the tree (Nil child pointer) is detected.
#        do { } while (node != &nil);
        cmpobne r3,r15,.rbfo100         # Jif node != Nil
#
.rbfo900:
# --- No overlapping nodes were detected, return FALSE/NULL.
#
        mov     FALSE,g1
        mov     g1,g0                   # MSHDEBUG, return g0 instead of g1
        ret
#
#
.rbfo500:
#
# --- Else an overlap condition with this node has been detected.
#     (endLba > Node's Key Start LBA) AND (startLba < Node's End Key, EndLBA)
#
#     Continue to search the left children of this node for an
#     earlier node with an overlap condition.  This search will
#     continue to search left for an earlier node with an overlap
#     until a node without an overlap is detected.  Then the search
#     direction will switch to the right children.  The search will
#     continue to search the right children until another node with
#     an overlap condition is detected.  Then the search direction
#     will switch back to the left children.  The search direction
#     will continue to alternate between searching left and right
#     children until the bottom of the tree (Nil child pointer) is
#     reached.  The last node that contained an overlap condition
#     is the earliest (smallest LBA) that will be returned.
#
#     Initialize the overlap node to this first node that contained
#     an overlap.  Set the next node to search to the left child
#     of the current node.
#       overlapNode = node
#       node = node->leftChild
#
        mov     r3,g1                   # overlapNode = node
        mov     r6,r3                   # node = node->leftChild
        b       .rbfo850                # Branch to start of while loop test
#
.rbfo550:
# --- Top of While loop to search for an earlier overlap node.
#     Test for an overlap with the next node.
#     Only need to compare the startLba to the node's endKey
#     since the endLba must be greater than the node's key
#     (because of the initial overlap detection).
#       if (startLba <= node->endKey)
#
        ldl     rbkeym(r3),r8           # Load endKey from RB node
        cmpobne g3,r9,.rbfo700          # Compare MSBs, Jif MSBs differ
        cmpo    g2,r8                   # MSBs equal, Compare LSBs
.rbfo700:
        bg      .rbfo800                # Jif startLba > endKey
#
# --- Overlap detected.
#     Update the overlap node and continue to search
#     for an earlier overlap with the left children.
#       overlapNode = node
#       node = node->leftChild
#
        mov     r3,g1                   # overlapNode = node
        ld      rbcleft(r3),r3          # node = node->leftChild
        b       .rbfo850
#
.rbfo800:                               # else
# --- Else No Overlap.
#     Try to find another overlap by searching the right children.
#       node = node->rightChild;
#
        ld      rbcright(r3),r3         # node = node->rightChild
#
.rbfo850:
# --- Continue to search for an earlier overlap node until a Nil child
#     pointer is found.
#       while (node != &nil)
#
        cmpobne r3,r15,.rbfo550         # while (node != Nil)
#
# --- Return the earliest overlapping node
#       return(overlapNode);
#
        mov     g1,g0                   # MSHDEBUG, return g0 instead of g1
        ret                             # return(g1=overlapNode)
#
#******************************************************************************
#
#  NAME: RB$noverlap
#
#  PURPOSE:
#       Locate next key range in the RB tree that overlaps the supplied key
#       range from a position in a given RB tree.
#
#  DESCRIPTION:
#       Starting at a position of the given tree, this routine locates the next
#       node in the tree that overlaps the specified key range.  Register g1
#       is returned as the next node that overlaps; FALSE indicates that no
#       overlap was found.
#
#       It is assumed that the given node was a previously overlapping node
#       and the search is to find the next node inorder that overlaps.
#
#  CALLING SEQUENCE:
#       call    RB$noverlap
#
#  INPUT:
#       g1    = Address of starting position in interval tree to locate next node
#       g4/g5 = Max Key value for range
#
#       Note:  This routine  does not need an input for the Min Key value
#              to detect an overlap since it is assumed that an overlap with the
#              input node was detected previously by RB$foverlap.  This implies
#              that the Min Key value that would have been input to RB$foverlap
#              must be less than the starting key value for the predecessor node
#              that will be checked for overlap by this routine.
#
#  OUTPUT:
#       g1    = FALSE or address of located node
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
RB$noverlap:
#
# RB_NODE_PTR RB$NextOverlap(RB_NODE_PTR node, unsigned int endLba)
# {
#    RB_NODE_PTR     overlapNode;
# }
#
# --- Find the next (successor) node following the input node in the tree.
#       nextNode = RB$locateNext(node)
#
        call    RB$locateNext
#
# --- Check for a NULL node which indicates there was no successor node
#     because the end of the tree was reached (greatest LBA).
#       if (nextNode != NULL)
#
        cmpobe  0,g1,.rbno100           # Jif NULL returned
#
# --- A successor node exists.
#     Determine if an overlap exists.
#     If the input Ending LBA is before the start key for the node,
#     then another overlap does not exists.
#       if (endLba < nextNode->key)
#
        ldl     rbkey(g1),r4            # Load starting Key value MSHDEBUG - FIX 5/26
        cmpobne g5,r5,.rbno150          # Compare MSBs, Jif MSBs differ
        cmpo    g4,r4                   # MSBs equal, Compare LSBs
.rbno150:
        bge     .rbno200                # Jif endLba >= key, return(nextNode) MSHDEBUG, FIX 6/6
.rbno100:
#
# --- The next node does not overlap the input LBA range.
#     Return a NULL/FALSE pointer to indicate no overlap.
#       return(NULL);
#
        mov     FALSE,g1                # return FALSE, no overlap
.rbno200:
        mov     g1,g0                   # MSHDEBUG, return g0 instead of g1
        ret
#
RB$locateNextCIntf:
         mov    g0,g1
         call   RB$locateNext
         mov    g1,g0
         ret
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
