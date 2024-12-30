# $Id: rbi.as 159300 2012-06-16 04:47:51Z m4 $
#******************************************************************************
#
#  NAME: rbi.as
#
#  PURPOSE:
#
#       To provide red-black interval tree related functions.
#
#       Red-Black trees are a self balancing binary search tree where
#       insert, delete and locations are inherently of order log2N where
#       N is the size in elements of the tree.  This tree can also
#       be used to determine if a specified interval overlaps any
#       other intervals stored in the tree.
#
#       NOTE:  This tree is to be used to store overlapping intervals.
#
#       The interval is specified by the key, <rbkey> as the lower bound with
#       the upper bound specified by the <rbkeym> field in the node.
#
#       The elements (nodes) are maintained with ascending order of the
#       64 bit key stored in each node.  If a node is inserted with
#       a duplicate key value the node to be inserted is chained to
#       a doubly-linked list with the existing node.
#
#       Note that support for deletion of duplicate nodes in a linked list
#       is not provided by this code.
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
#
#       RBI$insert      - Add element to tree.
#       RBI$delete      - Delete node from tree.
#       RBI$foverlap    - Locate first overlapping interval in given tree.
#       RBI$noverlap    - Locate next overlapping interval in tree.
#
#  Copyright (c) 1996 - 2008 Xiotech Corporation.  All rights reserved.
#
#******************************************************************************
#
# --- Executable code ---------------------------------------------------------
        .text
#
.if rbidebug
#******************************************************************************
#
#  NAME: RB$test
#
#  PURPOSE:
#       Test RB tree operations.
#
#  DESCRIPTION:
#       For testing write cache and RB tree functions.
#
#  CALLING SEQUENCE:
#       Forked as separate task.
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#******************************************************************************
#
RB$test:
        ldconst 0x0400,r15              # Number of VRP/ILTs to allocate
        ldconst 0x3000,r14              # Starting disk address for request
        ldconst 0x1,r13                 # Sizeof request
.rb10:
c       g0 = s_MallocW(vrpsiz, __FILE__, __LINE__);
        ldconst vroutput,r3             # Set write request
        stos    r3,vr_func(g0)
        mov     r14,r4
        ldconst 0,r5                    # Set starting disk address
        stl     r4,vr_vsda(g0)
        st      r13,vr_vlen(g0)         # Set sizeof request
#
        mov     g0,r12
c       g0 = s_MallocW(iltsiz, __FILE__, __LINE__);
        st      r12,il_w0(g0)
        mov     g0,g1                   # g1 = ILT to submit
        ldconst 0,g0                    # g0 = VID
#
        call    WC$coverlap
        addo    2,r13,r13               # Adjust sizeof request
        subo    1,r14,r14
        subo    2,r15,r15
        cmpibl  0,r15,.rb10             # Loop for next request
        bl      .rb10                   # Submit next VRP
#
.rb20:
        mov     r3,r3
        b       RB$test
.endif  # rbidebug
#
#******************************************************************************
#
#  NAME: RBI$insert
#
#  PURPOSE:
#       Insert an element into a RB interval tree.
#
#  DESCRIPTION:
#       This routine is used to insert a node into an existing RB tree.
#       The node is inserted in the tree in ascending order according to
#       the 64-bit key in each element.  After the insertion the tree
#       is rebalanced.
#
#       The node to be inserted must have the <rbkey> field set to the
#       desired lower bound and the <rbkeym> value set to
#       the endpoint of the range.  The <rbnodem> value is computed and
#       propagated through the tree as necessary.
#
#       In the case of a duplicate key the node is linked onto the existing
#       node(s) in order of insertion.  The node maximum is then updated
#       to reflect the added node.
#
#       The thread pointers <rbfthd> and <rbbthd> are used to link the node
#       onto the doubly-linked list.
#
#       Note that this code is intended to be used for Write Cache purposes
#       as the <rbkeym> field of a node may be altered when additional duplicate
#       nodes are linked onto it.  In this case the key value will have to
#       be stored elsewhere or, as in the case of I/Os stored in the tree
#       for the Write Cache purposes, the <rbdpoint> field points to an ILT
#       that in turn points to a VRP which contains the necessary data to
#       reconstruct the <rbkeym> value.
#
#  CALLING SEQUENCE:
#       call    RBI$insert
#
#  INPUT:
#       g0 = address of root element of tree.
#       g1 = element to insert in tree.
#
#  OUTPUT:
#       g0 = possible new root of tree
#
#******************************************************************************
#
RBI$insert:
.if rbidebug_NodeMax
        PushRegs(r3)
        call    CheckNodeMaximum        # Check the NodeMax values in the tree
        PopRegsVoid(r3)
.endif  # rbidebug_NodeMax
        ldl     rbkey(g1),r12           # Get key value
        ldl     rbkeym(g1),r10          # Get key maximum
        lda     ._nil,r14               # get NIL value
        stl     r10,rbnodem(g1)         # Preset node maximum value
        mov     0,r15                   # Preset parent value
        mov     0,r4                    # Preset r4 to zero, in case only one node in tree.
#
# --- Locate future parent
#
        cmpo    0,g0                    # Check for null root (empty tree)
        mov     g0,r3                   # r3 = current node
        sele    g0,g1,g0                # Select current node if no root
        be      .rbii30                 # Jif only one node in tree
#
.rbii10:
        ldq     rbkey(r3),r4            # Get key value
                                        #     and left/right child pointers
        ldl     rbnodem(r3),r8          # Get node maximum
        mov     r3,r15                  # Setup parent node pointer
#
# --- Update node maximum if necessary
#
        cmpobne r9,r11,.rbii14          # Jif MSBs differ
        cmpo    r8,r10                  # MSBs differ; check LSBs
#
.rbii14:
        bge     .rbii15                 # Jif node max > key max
        stl     r10,rbnodem(r3)         # Update node maximum
.rbii15:
#
# --- Check MSB of current to insertion node
#
        cmpobne r13,r5,.rbii20          # Jif MSBs differ
#
# --- Check for duplicate key condition
#
        cmpobe  r12,r4,.rbii100         # Jif duplicate key
#
# --- Select right child if insertion key  > node key; otherwise left child
#
.rbii20:
        selg    r6,r7,r3                # Select proper child pointer
        selg    rbcleft,rbcright,r4     # Get child pointer offset
        cmpobne r14,r3,.rbii10          # Jif pointer not NIL
#
# --- Insertion point found.  Insert node.
#
.rbii30:
        mov     r15,r10                 # Setup parent pointer
        ldconst RED,r11                 # Preset RED color
        addo    r4,r15,r4               # Formulate pointer to parent's child
                                        #  for this node
        mov     r14,r15                 # Set up for long store
        stl     r14,rbcleft(g1)         # Set left child to NIL
                                        #     right child to NIL
        stl     r10,rbparent(g1)        #     parent value
                                        #     color to RED
        mov     0,r6
        mov     0,r7                    # Clear forward and backward pointers
        stl     r6,rbfthd(g1)
#
        cmpobe  g1,g0,.rbii40           # Jif at root node
        st      g1,(r4)                 # Set correct parent child pointer to
                                        #  point to this new node
#
# --- Node is inserted into tree.  Now rebalance.
#
.rbii40:
        call    rbi$ifixup              # Rebalance tree if necessary
        b       .rbi110
#
.rbii100:
        ldl     rbkeym(r3),r8           # Get current node key maximum
#
# --- Duplicate node.  Set key maximum for head node if this node is >
#
        cmpobne r9,r11,.rbi105          # Jif MSBs differ
        cmpo    r8,r10                  # Check LSBs
#
.rbi105:
        bg      .rbi106                 # Jif original key > new
#
        stl     r10,rbkeym(r3)          # Set key max for head node
#
.rbi106:
        st      r3,rbparent(g1)         # Set parent of new node to
                                        #  head of list
#
# --- Link onto doubly-linked list.
#
        ld      rbfthd(r3),r4           # Get forward pointer
        mov     r3,r5                   # Setup for double store
#
        stl     r4,rbfthd(g1)           # Set forward pointer of current node/
                                        #     backward pointer of current node
        st      g1,rbfthd(r3)           # Setup forward pointer of head node
#
        cmpobe  0,r4,.rbi110            # Jif thread did not exist
#
# --- Set backward pointer of next node to point to this node.
#
        st      g1,rbbthd(r4)
#
.rbi110:
.if rbidebug_NodeMax
        PushRegs(r3)
        call    CheckNodeMaximum        # Check the NodeMax values in the tree
        PopRegsVoid(r3)
.endif  # rbidebug_NodeMax
        ret
#
#******************************************************************************
#
#  NAME: rbi$ifixup
#
#  PURPOSE:
#       Rebalance a RB interval tree if necessary after a node insertion.
#
#  DESCRIPTION:
#       This routine is used to rebalance a tree according to the
#       Red-Black rules after a node has been inserted into a tree.
#       This operation may recolor nodes and may cause rotations.
#       This operation completes in order log2N time where N
#       is the size of the tree in elements.
#
#  CALLING SEQUENCE:
#       call    rbi$ifixup
#
#  INPUT:
#       g0 = root address of tree
#       g1 = address of newly inserted element in tree.
#
#  OUTPUT:
#       g0 = Possible new root of tree
#
#  REGS DESTROYED:
#       g1
#
#******************************************************************************
#
rbi$ifixup:
        ldconst BLACK,r15
        ldconst RED,r14
        mov     g1,r3                   # r3 = current node
#
.rbiif10:

## V2: In this file, the code is okay. But in
## rb.as, the code is wrong. I put this part in to keep code
## same in both the files.
        cmpobe  g0,r3,.rbiif100         # Jif at root node (done)
        ld      rbparent(r3),r4         # Get parent pointer
        ldob    rbcolor(r4),r5          # Get parent color
        cmpobne RED,r5,.rbiif100        # Jif no red-red violation
        ld      rbparent(r4),r6         # Get grandparent's pointer
        ldl     rbcleft(r6),r8          # Get grandparent's left/right child pointer
#
# --- Red-Red violation exists.  Check uncle node for color.
#
        cmpobne r4,r8,.rbiif50          # Jif parent is not g'parent's
                                        #  left child
#
# --- get uncle node
#
#       r9 = right uncle pointer
#
        ldob    rbcolor(r9),r8          # Load color from node
        cmpobe  BLACK,r8,.rbiif30       # jif uncle BLACK
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
        b       .rbiif10                # Check for violation
#
.rbiif30:
#
# --- Uncle is BLACK
#
        ld      rbcright(r4),r9         # Check for right child
        cmpobne r3,r9,.rbiif40          # Jif not right child
#
# --- Make this node a left child and rotate tree left at parent node
#
        mov     r4,r3                   # Current node is now parent
        mov     r3,g1                   # g1 = position in tree to rotate
        call    rbi$rleft               # Rotate tree left
        ld      rbparent(r3),r4         # get new parent of x
        ld      rbparent(r4),r6         # Get grandparent of x
#
# --- Now recolor and rotate
#
.rbiif40:
        stob    r15,rbcolor(r4)         # Set parent color to BLACK
        stob    r14,rbcolor(r6)         # Set grandparent color to RED
        mov     r6,g1                   # Rotate grandparent
        call    rbi$rright              # Rotate tree right
        b       .rbiif10                # Check for further violation
#
# --- Mirror of above code for right child
#
.rbiif50:
#
# --- get uncle node
#
#       r8 = uncle pointer
#
        ldob    rbcolor(r8),r9          # Load color from node
        cmpobe  BLACK,r9,.rbiif70       # jif uncle BLACK
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
        b       .rbiif10                # Check for violation
#
.rbiif70:
#
# --- Uncle is BLACK
#
        ld      rbcleft(r4),r9          # Check for left child
        cmpobne r3,r9,.rbiif80          # Jif not left child
#
# --- Make this node a right child and rotate tree right at parent node
#
        mov     r4,r3                   # Current node is now parent
        mov     r3,g1                   # g0 = position in tree to rotate
        call    rbi$rright              # Rotate tree right
        ld      rbparent(r3),r4         # get new parent of x
        ld      rbparent(r4),r6         # Get grandparent of x
#
# --- Now recolor and rotate
#
.rbiif80:
        stob    r15,rbcolor(r4)         # Set parent color to BLACK
        stob    r14,rbcolor(r6)         # Set grandparent color to RED
        mov     r6,g1                   # Rotate grandparent
        call    rbi$rleft               # Rotate tree left
        b       .rbiif10                # Check for further violation
#
.rbiif100:
        stob    r15,rbcolor(g0)         # Set root color to BLACK
.if rbidebug_NodeMax
        PushRegs(r3)
        call    CheckNodeMaximum        # Check the NodeMax values in the tree
        PopRegsVoid(r3)
.endif  # rbidebug_NodeMax
        ret                             # Done!
#
#******************************************************************************
#
#  NAME: RBI$delete
#
#  PURPOSE:
#       Delete an element from a RB tree.
#
#  DESCRIPTION:
#       This routine is used to delete a node from an existing RB tree.
#       The node (subtree) maximum is recalculated and the tree is rebalanced.
#
#  CALLING SEQUENCE:
#       call    RBI$delete
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
RBI$delete:
#
.if rbidebug_NodeMax
        PushRegs(r3)
        call    CheckNodeMaximum        # Check the NodeMax values in the tree
        PopRegsVoid(r3)
.endif  # rbidebug_NodeMax
        mov     0,r4
        lda     0,r5
        stl     r4,rbkeym(g1)           # Clear node max key value
        call    rbi$maxprop             # Propagate maximum up tree until
                                        #  node g0 (root) reached
.if rbidebug_NodeMax
        PushRegs(r3)
        call    CheckNodeMaximum        # Check the NodeMax values in the tree
        PopRegsVoid(r3)
.endif  # rbidebug_NodeMax
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
        cmpobe  r15,r4,.rbid60          # Jif left child NIL
        cmpobne r15,r5,.rbid80          # Jif right child not NIL
#
# - Set y to z, node can be spliced out.
#
.rbid60:
        mov     g1,r14                  # Set y = z
#
        b       .rbid90
#
.rbid80:
#
# --- Both children of z (node to be deleted) are present, find inorder
#     successor and assign to y.  Find successive left children of
#     original right child of z until left pointer = NIL.
#
        mov     r5,r14                  # Preserve pointer (set to y)
        ld      rbcleft(r5),r5          # Get left child pointer
        cmpobne r15,r5,.rbid80          # Jif pointer not NIL
#
# --- Need to update maximum node value upward to node being deleted from this
#      node
#
        mov     g0,r4                   # Preserve g0
        mov     g1,r5                   #  and g1
        mov     g1,g0                   # Node to stop maxprop at
        ld      rbparent(r14),g1        # Node to start maxprop
        cmpobe  g1,g0,.rbid85           #  Jif nodes same
#
#   If there is a right child, need to pick up it's nodeMax instead of the
#   node being moved to propagate upward.  If there is no right child, zero
#   the node max of the node being moved (which will force the parent to pick
#   up its nodem or the right child of the parents nodem.
#
        ld      rbcright(r14),r6        # Get the moving nodes Right Child
        cmpobe  r15,r6,.rbid82          # Jif there is no Right Child
        ldl     rbnodem(r6),r6          # Get the Right Childs Node Max
        b       .rbid84                 # Go Store the new node max and prop.
#
.rbid82:
        mov     0,r6                    # Set node max
        lda     0,r7                    #  to zero
.rbid84:
        stl     r6,rbnodem(r14)         # Clear or set to Right Childs node max
        call    rbi$maxprop             # Propagate maximum from
                                        #  inorder successor to
                                        #  node being deleted
.rbid85:
        mov     r4,g0                   # Restore g0
        mov     r5,g1                   #  and g1
.rbid90:
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
        cmpobne 0,r6,.rbid105           # Check for parent of y==0
                                        #  if so, at root and need to set
                                        #  a new root value
#
# --- At the root, check to see if x == Nil which means the last node has
#   been deleted from the tree.  If so, change the root to 0 (Null),
#   else root = x
#
        cmpo    r15,r13                 # Test if x == Nil
        sele    r13,0,g0                # If Nil, set root = 0, else root = x
        be      .rbid150                # Jif was Nil, tree empty, exit
        b       .rbid110                # Jif was Nil
#
# --- Find whether y is left or right child of parent
#
#     r6 = parent pointer
#
.rbid105:
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
.rbid110:
        ldob    rbcolor(r14),r11        # Get color of old node
        cmpobe  r14,g1,.rbid140         # Jif y == z; node was spliced out
                                        #  and don't need to replace with
                                        #  inorder successor
#
# --- y points to a inorder successor node.  Need to swap these nodes (change
#     pointers of y to match those of z, and parent of child nodes)
#     Done to preserve any external pointers that point to this node.
#
        ldob    rbcolor(g1),r12         # Get original color of z
        stob    r12,rbcolor(r14)        # Set new node to old color
        ldt     rbcleft(g1),r4          # get left/right/parent pointers
        stt     r4,rbcleft(r14)         # Update left/right/parent
                                        #   of node y (replacing node z)
        st      r14,rbparent(r4)        # update left child parent
        st      r14,rbparent(r5)        # update right child parent
#
# --- Update maximum for replacement node.
#
        mov     g0,r4                   # Preserve g0
        mov     g1,r8                   # Preserve g1
        mov     r14,g0                  # g0 = node to calc max of
        call    rbi$maxcalc             # calc max for node
        mov     r4,g0                   # Restore g0
        mov     r8,g1                   # Restore g1
#
# --- Check for parent(x) == z; if true set parent(x) to y.
#
#     r14 = y
#     r13 = x
#     g1  = z
#
# --- check for z == root; if true reset root to new node.
#
        cmpo    g0,g1
        sele    g0,r14,g0               # Reset root if necessary
        be      .rbid140                # Jif parent root
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
#     node;  x is new node to replace y.
#
        addo    r3,r6,r3                # Add to pointer to parent
        st      r14,(r3)                #  and set child pointer
#
# --- Check color of old node and if black need to adjust tree.
#
.rbid140:
#
        cmpobne BLACK,r11,.rbid150      # Jif not black; no need to fix
                                        #  tree
#
# --- g0 = root of tree
#     g1 = node to fixup
#
        mov     r13,g1                  # Fixup at x
        b       rbi$dfixup              # Readjust tree/return
#
.rbid150:
.if rbidebug_NodeMax
        PushRegs(r3)
        call    CheckNodeMaximum        # Check the NodeMax values in the tree
        PopRegsVoid(r3)
.endif  # rbidebug_NodeMax
        ret                             # Done
#
#******************************************************************************
#
#  NAME: rbi$dfixup
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
#       call    rbi$dfixup
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
rbi$dfixup:
        ldconst BLACK,r15
        ldconst RED,r14
        mov     g1,r13
#
.rbidf10:
        ldob    rbcolor(r13),r3         # Get x node color
        cmpobne BLACK,r3,.rbidf100      # Jif not black; done                 Line 1
        cmpobe  g0,r13,.rbidf100        # Jif at root                         Line 1
#
        ld      rbparent(r13),r12       # Get parent of node
        ldl     rbcleft(r12),r6         # Get left/right child of parent      Line 3
        cmpobne r6,r13,.rbidf50         # Jif not left child of parent        Line 2
#
        ldob    rbcolor(r7),r8          # get color of right uncle
        cmpobne RED,r8,.rbidf20         # Jif not RED                         Line 4
#
        stob    r15,rbcolor(r7)         # set color to black                  Line 5
        stob    r14,rbcolor(r12)        # set parent color to red             Line 6
#
# --- Rotate tree left at parent
#
        mov     r12,g1                  # g1 = node to rotate                 Line 7
        call    rbi$rleft               # Rotate tree left around parent of x Line 7
        ld      rbparent(r13),r12       # get new x parent                    Line 8
        ld      rbcright(r12),r7        # get new right child of x parent     Line 8
#
# --- r13 = x
#     r12 = parent of x
#     r7  = w
#
.rbidf20:
        ldl     rbcleft(r7),r8          # get w left/right child
        ldob    rbcolor(r8),r10         # get left child color
        ldob    rbcolor(r9),r11         # get right child color
#
# --- check for left and right nodes of w BLACK
#
        cmpobne r10,r11,.rbidf30        # jif not same color                  Line 9
        cmpobne BLACK,r10,.rbidf30      # jif not BLACK                       Line 9
#
# --- set color of w to RED
#
        stob    r14,rbcolor(r7)         # set color of w to RED               Line 10
        mov     r12,r13                 # set x to parent of x                Line 11
        b       .rbidf10                # Iterate at new x
#
.rbidf30:
        cmpobne BLACK,r11,.rbidf40      # jif right child not BLACK           Line 12
        stob    r15,rbcolor(r8)         # Set left child color to BLACK       Line 13
        stob    r14,rbcolor(r7)         # set color of w to RED               Line 14
        mov     r7,g1
        call    rbi$rright              # Rotate tree right at w              Line 15
#
        ld      rbparent(r13),r12       # get new x parent                    Line 16
        ld      rbcright(r12),r7        # get new right child of x parent     Line 16
#
.rbidf40:
        ldob    rbcolor(r12),r3         # get color of parent of x            Line 17
        ld      rbparent(r7),r4         # get parent of w                     ?? Line 18 (delete, use r12 below)
        ld      rbcright(r7),r5         # get right child of w                Line 19
        stob    r3,rbcolor(r7)          # set color of w to color of x parent Line 17
        stob    r15,rbcolor(r4)         # set parent color to BLACK           ?? Line 18 (use r12 ??)
        stob    r15,rbcolor(r5)         # set right child of w color to BLACK Line 19
        mov     r12,g1                  # Rotate left at x parent             Line 20
        call    rbi$rleft               #                                     Line 20
        mov     g0,r13                  # set x to root                       Line 21
#
        b       .rbidf100               # Done!
#
# --- Mirror image of .rbidf20 - .rbidf40
#
.rbidf50:
        mov     r6,r7                   # w = r7
#
        ldob    rbcolor(r7),r8          # get color of left uncle
        cmpobne RED,r8,.rbidf60         # Jif not RED
#
        stob    r15,rbcolor(r7)         # set color to black
        stob    r14,rbcolor(r12)        # set parent color to red
#
# --- Rotate tree right at parent
#
        mov     r12,g1                  # g1 = node to rotate
        call    rbi$rright              # Rotate tree right
        ld      rbparent(r13),r12       # get new x parent
        ld      rbcleft(r12),r7         # get new left child of x parent
#
# --- r13 = x
#     r12 = parent of x
#     r7  = w
#
.rbidf60:
        ldl     rbcleft(r7),r8          # get w left/right child
        ldob    rbcolor(r8),r10         # get left child color
        ldob    rbcolor(r9),r11         # get right child color
#
# --- check for left and right nodes of w BLACK
#
        cmpobne r10,r11,.rbidf70        # jif not same color
        cmpobne BLACK,r10,.rbidf70      # jif not BLACK
#
# --- set color of w to RED
#
        stob    r14,rbcolor(r7)         # set color of w to RED
        mov     r12,r13                 # set x to parent of x
        b       .rbidf10                # Iterate at new x
#
.rbidf70:
        cmpobne BLACK,r10,.rbidf80      # jif left child not BLACK
        stob    r15,rbcolor(r9)         # Set right child color to BLACK
        stob    r14,rbcolor(r7)         # set color of w to RED
        mov     r7,g1
        call    rbi$rleft               # Rotate tree left at w
        ld      rbparent(r13),r12       # get new x parent
        ld      rbcleft(r12),r7         # get new left child of x parent
#
.rbidf80:
        ldob    rbcolor(r12),r3         # get color of parent of x
        ld      rbparent(r7),r4         # get parent of w
        ld      rbcleft(r7),r5          # get left child of w
        stob    r3,rbcolor(r7)          # set color of w to color of x parent
        stob    r15,rbcolor(r4)         # set parent color to BLACK
        stob    r15,rbcolor(r5)         # set left child of w color to BLACK
        mov     r12,g1                  # Rotate right at x parent
        call    rbi$rright
        mov     g0,r13                  # set x to root
#
.rbidf100:
        stob    r15,rbcolor(r13)        # set color of x to BLACK
.if rbidebug_NodeMax
        PushRegs(r3)
        call    CheckNodeMaximum        # Check the NodeMax values in the tree
        PopRegsVoid(r3)
.endif  # rbidebug_NodeMax
        ret
#
#******************************************************************************
#
#  NAME: rbi$rleft
#
#  PURPOSE:
#       Rotate a tree left at specified position.
#
#  DESCRIPTION:
#       This routine is used to perform a leftwise rotation at the
#       location specified in register g1.
#
#  CALLING SEQUENCE:
#       call    rbi$rleft
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
rbi$rleft:
        mov     g0,r14                  # Preserve root pointer
        ldt     rbcleft(g1),r4          # Get pointer to left/right child
                                        #     and pointer to parent
        lda     ._nil,r15               # get NIL pointer value
        ldl     rbnodem(g1),r10         # get node max value
        ld      rbcleft(r5),r8          # get left pointer of right child
        st      r8,rbcright(g1)         #  and store as right pointer
                                        #  in specified node
        stl     r10,rbnodem(r5)         # store node max value in new
                                        #  subtree root
#
        cmpobe  r8,r15,.rbirl10         # Jif left pointer of right child
                                        #  NIL
#
        st      g1,rbparent(r8)         # set node as parent
.rbirl10:
        st      r6,rbparent(r5)         # Set parent of right child
                                        #  to parent of current node
        cmpobe  0,r6,.rbirl30           # Jif parent null (root)
        ld      rbcleft(r6),r8          # get left child pointer of parent
        cmpo    r8,g1                   # check for left child
        addone  (rbcright-rbcleft),r6,r6 # set correct offset
#
        st      r5,rbcleft(r6)          # move right child to parent
        b       .rbirl40
#
# --- Parent pointer was nil; assume root
#
.rbirl30:
        mov     r5,r14                  # Set root to right child of node
.rbirl40:
        mov     g1,g0                   # Calculate new node max for
                                        #   old subtree root
        call    rbi$maxcalc             # calculate/update maximum
#
# --- Link the two nodes together
#
        mov     g0,g1                   # Restore g1
        mov     r14,g0                  # Restore g0
        st      g1,rbcleft(r5)
        st      r5,rbparent(g1)         # Set parent of this node
#
.if rbidebug_NodeMax
        PushRegs(r3)
        call    CheckNodeMaximum        # Check the NodeMax values in the tree
        PopRegsVoid(r3)
.endif  # rbidebug_NodeMax
        ret
#
#******************************************************************************
#
#  NAME: rbi$rright
#
#  PURPOSE:
#       Rotate a tree right at specified position.
#
#  DESCRIPTION:
#       This routine is used to perform a rightwise rotation at the
#       location specified in register g1.
#
#  CALLING SEQUENCE:
#       call    rbi$rright
#
#  INPUT:
#       g0 = address of root of tree
#       g1 = address of element to rotate right
#
#  OUTPUT:
#       g0 = possible new root of tree
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
rbi$rright:
        mov     g0,r14                  # Preserve root pointer
        ldt     rbcleft(g1),r4          # Get pointer to left/right child
                                        #     and pointer to parent
        lda     ._nil,r15               # get NIL pointer value
        ldl     rbnodem(g1),r10         # get node max value
        ld      rbcright(r4),r8         # get right pointer of left child
        st      r8,rbcleft(g1)          #  and store as left pointer
                                        #  in specified node
        stl     r10,rbnodem(r4)         # store node max value in new
                                        #  subtree root
#
        cmpobe  r8,r15,.rbirr10         # Jif right pointer of left child
                                        #  NIL
#
        st      g1,rbparent(r8)         # set node as parent
.rbirr10:
        st      r6,rbparent(r4)         # Set parent of left child
                                        #  to parent of current node
        cmpobe  0,r6,.rbirr30           # Jif parent null (root)
        ld      rbcright(r6),r8         # get right child pointer of parent
        cmpo    r8,g1                   # check for right child
        addoe   (rbcright-rbcleft),r6,r6 # set correct offset
#
        st      r4,rbcleft(r6)          # move left child to parent
        b       .rbirr40
#
# --- Parent pointer was nil; assume root
#
.rbirr30:
        mov     r4,r14                  # Set root to left child of node
.rbirr40:
        mov     g1,g0                   # Calculate new node max for
                                        #   old subtree root
        call    rbi$maxcalc             # calculate/update maximum
        mov     g0,g1                   # Restore g1
        mov     r14,g0                  # Restore g0
#
# --- Link the two nodes together
#
        st      g1,rbcright(r4)
        st      r4,rbparent(g1)         # Set parent of this node
#
.if rbidebug_NodeMax
        PushRegs(r3)
        call    CheckNodeMaximum        # Check the NodeMax values in the tree
        PopRegsVoid(r3)
.endif  # rbidebug_NodeMax
        ret
#
#******************************************************************************
#
#  NAME: rbi$maxcalc
#
#  PURPOSE:
#       Calculate the node maximum for a given node pointer.
#
#  DESCRIPTION:
#       This routine looks up the key maximum values for each child of
#       the specified node, compares it to the maximum value for the node,
#       and places the greater of the three into the node maximum value
#       for the specified node.
#
#       If a child pointer points to the NIL node, <._nil>, the value used
#       for the maximum value of that child is zero.
#
#       Register g1 is returned as TRUE if a new maximum was calculated
#       for this node.  FALSE indicates that the maximum was calculated
#       to be the same as already stored in the node.
#
#  CALLING SEQUENCE:
#       call    rbi$maxcalc
#
#  INPUT:
#       g0 = address of indicated node.
#
#  OUTPUT:
#       g1 = TRUE/FALSE; TRUE indicates computed maximum is different than
#                        old maximum stored into node.
#
#  REGS DESTROYED:
#       g1
#
#******************************************************************************
#
rbi$maxcalc:
        lda     ._nil,r10               # get NIL pointer
        ldl     rbcleft(g0),r4          # get left/right child pointers
        ldq     rbkeym(g0),r12          # get key max
                                        #  and node max values
        cmpobe  r10,r4,.rbim10          # Jif left pointer NIL
#
        ldl     rbnodem(r4),r6          # get left child node maximum
#
.rbim10:
        sele    r6,0,r6                 # Set to zero
        sele    r7,0,r7                 #    if NIL left child
#
        cmpobe  r10,r5,.rbim20          # Jif right pointer NIL
#
        ldl     rbnodem(r5),r8          # get right child node maximum
#
.rbim20:
        sele    r8,0,r8                 # Set to zero
        sele    r9,0,r9                 #    if NIL right child
#
# --- Determine "winner" of left and right children
#
        cmpobne r7,r9,.rbim30           # Jif MSB left child >  or < right
        cmpobe  r6,r8,.rbim40           # Jif MSBs and LSBs equal
#
.rbim30:
        sell    r7,r9,r7                # Select right child MSB
                                        #  if right > left
        sell    r6,r8,r6                # Select right child LSB
                                        #  if right > left
#
.rbim40:
#
# --- Select "winner" with current node key maximum value
#
        cmpobne r7,r13,.rbim50          # Jif MSB "winner" > or < node
        cmpobe  r6,r12,.rbim60          # Jif MSBs and LSBs equal
#
.rbim50:
        sell    r7,r13,r7               # Select node value MSB
                                        #   if node > "winner"
        sell    r6,r12,r6               # Select node value LSB
                                        #   if node > "winner"
#
.rbim60:
        cmpobne r7,r15,.rbim70          # Jif computed max MSB != old max MSB
        cmpobe  0,r12,.rbim70           # Jif old LSB zero (MDO Bugfix)
        cmpobe  r6,r14,.rbim80          # Jif max not changed
#
.rbim70:
        ldconst TRUE,g1                 # Set max altered
        stl     r6,rbnodem(g0)          # Set node maximum
        ret
.rbim80:
        ldconst FALSE,g1                # Set max not altered
        ret
#
#******************************************************************************
#
#  NAME: rbi$maxprop
#
#  PURPOSE:
#       Calculate the node maximums starting at the node specified
#       by register g1 and propagate upward until the node specified
#       by g0 is hit or root is reached. The propagation process is also
#       terminated if, after the node maximum is calculated, the new value
#       does not differ from the old value stored in the node.
#
#  DESCRIPTION:
#       This routine looks up the node maximum values for each child of
#       the specified node, compares it to the maximum value for the node,
#       and places the greater of the three into the node maximum value
#       for the specified node.  This process is repeated from each node
#       up the tree from node g1 to node g0 (or root).
#
#       If a child pointer points to the NIL node, <._nil>, the value used
#       for the maximum value of that child is zero.
#
#  CALLING SEQUENCE:
#       call    rbi$maxprop
#
#  INPUT:
#       g0 = address of indicated node to stop propagation.
#       g1 = address of node to start propagation.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
rbi$maxprop:
        mov     g0,r3                   # Preserve g0
        mov     g1,r4                   # Preserve g1
        mov     g1,g0                   # g0 = node to calc max of
#
.rbimp10:
        call    rbi$maxcalc
        cmpobe  g0,r3,.rbimp20          # Jif done
#
        ld      rbparent(g0),g0         # Move up to parent node
        cmpobe  FALSE,g1,.rbimp20       # Jif max not changed for this node
        cmpobne 0,g0,.rbimp10           # Jif not at root
#
.rbimp20:
        mov     r3,g0                   # Restore g0
        mov     r4,g1                   # Restore g1
        ret                             # Done!
#
#******************************************************************************
#
#  NAME: RBI$foverlap
#
#  PURPOSE:
#       Locate first key range in the interval tree that overlaps the supplied key
#       range from a given interval tree.
#
#  DESCRIPTION:
#       Starting at the root of the given tree, this routine locates the first
#       node in the tree that overlaps the specified key range.  Register g1
#       is returned as the next node that overlaps; FALSE indicates that no
#       overlap was found.  "First" implies the lowest lower key value in the
#       tree that overlaps the supplied range.
#
#  CALLING SEQUENCE:
#       call    RBI$foverlap
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
RBI$foverlap:
        lda     ._nil,r15
        mov     g0,g1                   # Setup starting node to search
#
.rbifo10:
        ldq     rbkey(g1),r4            # Get key, left, right values
#
# --- Check for left child NIL
#
        cmpobe  r6,r15,.rbifo30         # Jif no left child
#
# --- Check left child node max for >= Min Key value
#
        ldl     rbnodem(r6),r10         # Get left child node max
#
        cmpobne r11,g3,.rbifo20         # Jif MSBs differ
        cmpo    r10,g2                  # Check LSBs
.rbifo20:
        bl      .rbifo30                # Jif no interval in left subtree
                                        #   overlaps
        mov     r6,g1                   # Select left subtree
        b       .rbifo10                # Loop for next node
#
.rbifo30:
        ldl     rbkeym(g1),r10          # get current node maximum
#
# --- Check current node for overlap; if so then done. This will be the
#     lowest key value in the tree that overlaps.
#
#     Otherwise if possible select the right child.
#
# --- Check key value of current node for > Max Key Value
#
        cmpobne r5,g5,.rbifo40          # Jif MSBs differ
        cmpo    r4,g4                   # Check LSBs
.rbifo40:
        bg      .rbifo100               # Jif search unsuccessful
#
# --- Check for max of current node >= Min Key value
#
        cmpobne r11,g3,.rbifo50         # Jif MSBs differ
        cmpo    r10,g2                  # Check LSBs
.rbifo50:
        bge     .rbifo110               # Jif overlap; node found
#
# --- If no right child end search; unsuccessful.
#
        mov     r7,g1                   # Select right child
        cmpobne r7,r15,.rbifo10         # Not NIL; resume search
#
.rbifo100:
        ldconst FALSE,g1                # Set no node found
.rbifo110:
        ret
#
#******************************************************************************
#
#  NAME: RBI$noverlap
#
#  PURPOSE:
#       Locate next key range in the interval tree that overlaps the supplied key
#       range from a position in a given interval tree.
#
#  DESCRIPTION:
#       Starting at a position of the given tree, this routine locates the next
#       node in the tree that overlaps the specified key range.  Register g1
#       is returned as the next node that overlaps; FALSE indicates that no
#       overlap was found.
#
#       It is assumed that the given node was a previously overlapping interval
#       and the search is to find the next node inorder that overlaps.
#
#       It is also assumed that the left child (or subtree) of this node has
#       already been searched; searching will continue up the tree or to the
#       right of the supplied node address.
#
#  CALLING SEQUENCE:
#       call    RBI$noverlap
#
#  INPUT:
#       g1    = Address of starting position in interval tree to locate next node
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
RBI$noverlap:
        mov     g0,r14                  # Preserve g0
        lda     ._nil,r15
#
# --- Search sequence:
#
#       1). Check right child of node for overlap possibilities; if tree max
#           (and node value) in range find lowest matching key in subtree.
#
#       2). Move up tree to parent node and check if overlap; if not try right
#           child.
#
#       Terminate search if:
#
#       1). Overlap found.
#       2). min key range > max key value
#
# --- Start with right child of node (if not nil).
#
        ld      rbcright(g1),r7         # Get right child pointer
        ld      rbparent(g1),r8         # Get parent of node
#
.rbin25:
        cmpobe  r15,r7,.rbin30          # Jif right child NIL
#
# --- Locate first overlap in subtree rooted by right child.
#
        mov     r7,g0                   # Setup root pointer to right child
        mov     g1,r9                   # Preserve current node pointer
        call    RBI$foverlap            # Find lowest overlapped interval
                                        #  in subtree rooted by right child
        cmpobne FALSE,g1,.rbin60        # Jif done
#
        mov     r9,g1                   # Restore g1
#
# --- No overlapped interval found in subtree.  Try next node up tree.
#
.rbin30:
        cmpobe  0,r8,.rbin80            # Jif at root; unsuccessful
#
# --- Determine if parent node overlaps.
#
        mov     g1,r12                  # Preserve child
        mov     r8,g1                   # Set current node to parent
        ldq     rbkey(r8),r4            # get key value/left/right pointers
        ld      rbparent(g1),r8         # Get parent pointer
        ldl     rbkeym(g1),r10          # get key max value
#
# --- If came from right child, move up to next node; need to move up tree until
#     we are left child of parent or search ends.
#
        cmpobe  r7,r12,.rbin30          # Jif was right child of parent; move
                                        #  up to grandparent
#
# --- Check for key <= Max Key value
#           and max >= Min Key value
#
        cmpobne r5,g5,.rbin40           # Jif MSBs differ
        cmpo    r4,g4                   # Check LSBs
.rbin40:
        bg      .rbin80                 # Jif done with search,
                                        #  key > Max Key Value
#
# --- Check for overlap.
#
        cmpobne r11,g3,.rbin50          # Jif MSBs differ
        cmpo    r10,g2                  # Check LSBs
.rbin50:
        bl      .rbin25                 # Jif node not overlap
#
# --- This node is the next overlapped node; return
#
.rbin60:
        mov     r14,g0                  # Restore g0
        ret
#
# --- Set unsuccessful search
#
.rbin80:
        mov     r14,g0                  # Restore g0
        mov     FALSE,g1                # Set unsuccessful search
        ret
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
