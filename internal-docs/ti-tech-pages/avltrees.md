# AVL trees

This page describes an assembly-language implementation of indexed AVL
trees.

[Introduction
](#Intro)[Rotations
](#Rotations)[Inserting nodes
](#Insertion)[Deleting nodes](#Deletion)

[Threaded trees
](#Threaded%20trees)[Indexed trees](#Indexed%20trees)

[Testing](#Test)

[References](#Refs)

## Introduction

Binary trees are very usefull to perform ultra-fast dictionnary-type
searches. The principle is very simple: the data are arranged in a
balanced, binary tree in which each node contains two pointers, to a
larger and a smaller value. Here is an example:

            "DETERMINE"
           /           \
        "BULL"        "FROG"
       /      \       /    \
    "ADAM" "CLOUD"       "GUTS"

To search for a string, start from the top and compare the target string
with that in the node. If it is smaller take the left link, if it is
larger follow the right link. In this way, you can search through a
thousand words with 10 comparisons, through a million with 20, a billion
with 30, etc.

This is fine for sets of data that can be arranged in advance: start
with the middle value and put it at the top. On level two, put the
values that are at 1/4 and 3/4, then those at 1/8, 3/8, 5/8 and 7/8,
etc.

The problems begin when the user is allowed to enter new values, or to
delete existing ones. Suppose the user enters values in alphabetical
order, if we just follow a "go left / go right" strategy, we'll end up
with:

    "ADAM"
    /    \
        "BULL"
        /    \
            "CLOUD"
            /    \
                "ERIC"

Which defeats the purpose of a binary tree.

AVL trees were created by G.M. Adel'son-Velskii and E.M. Landis to
overcome this problem. The idea is very elegant: each node contains an
additional value, the balance, that indicates which child subtree is
heaviest (i.e. the depth in levels of the right subtree is substracted
from that of the left subtree). A value of 0 indicates that this node is
perfectly balanced. Values of +1 and -1 are unavoidable, but anything
beyond that is not tolerated and will cause the tree to rearrange.

But first, let's see how we can search a binary tree:

* --------------
*      DATA key               Key after which nodes are sorted (or ptr to it)
*      DATA left-link         Points to smaller node or &gt;0000 if none
*      DATA right-link        Points to greater node or &gt;0000 if none
*      BYTE balance           Balance factor (-1 / 0 / +1)
*      BYTE lcount            Number of items in the left branch (optional)
*
KEY    EQU  0
LEFT   EQU  2
RIGHT  EQU  4
BAL    EQU  6
LCNT   EQU  7
NSIZE  EQU  8                 node size`
*----------------------------------------------------------
* Find greatest node in a (sub)tree, keep track of our path
* Starting from node in R5, return ptr in R5
*----------------------------------------------------------
MAX    MOV  5,*8+             save node ptr in stack
       MOVB @BAL(5),*8+       and its balance
       MOVB @H00,*8+          no link yet
       MOV  @RIGHT(5),0       always go to the right
       JEQ  SK7               no more: we are done
       MOVB @H01,@-1(8)       flag: we went to the right
       MOV  0,5               keep going
       JMP  MAX
SK7    B    *11               found
*
*----------------------------------------------------------
* Find smallest node in a (sub)tree, keep track of our path
* Starting from node in R5, return ptr in R5
*----------------------------------------------------------
MIN    MOV  5,*8+             save node ptr in stack
       MOVB @BAL(5),*8+       and its balance
       MOVB @H00,*8+          no link yet
       MOV  @LEFT(5),0        always go to the left
       JEQ  SK7               no more: we are done
       MOVB @HFF,@-1(8)       flag: we went to the left
       MOV  0,5               keep going
       JMP  MIN
*
*----------------------------------------------------------
* Search for a key, keep track of our route
* Value to find (or ptr to it) in R4
* Return ptr to node into R5, skip JMP if found
*----------------------------------------------------------
FIND   MOV  11,10
       LI   8,STACK+4         memorize our route on a stack
       MOV  @ROOT+4,5         start from top of tree
 LP2    JEQ  SK6               not found
       MOV  5,*8+             save node ptr
       MOV  @BAL(5),*8+       save balance
        BL   @COMP             compare current node with wanted value
       JGT  SK4               value is greater
       JLT  SK5               value is smaller
       INCT 10                we  found it, skip jump
SK6    B    *10
 SK4    MOVB @H01,@-1(8)       leave flag: we went to the right
       MOV  @RIGHT(5),5       go right
       JMP  LP2
 SK5    MOVB @HFF,@-1(8)       leave flag: we went to the left
       MOV  @LEFT(5),5        go left
       JMP  LP2`
*----------------------------------------------------------
* User-defined routine.
* Compare two keys, return with result in status
* Values in R4 and *R5. ( or pointers to values).
*----------------------------------------------------------
COMP   C    4,*5              here: let&#39;s compare values
       B    *11`
*----------------------------------------------------------
* Data area
*----------------------------------------------------------
00    BYTE &gt;00               constants
H01    BYTE &gt;01
HFF    BYTE &gt;FF
       EVEN
*
ROOT   DATA 0,0,0,0           root of the tree
 WREGS  DATA 0,1,2,3,4,5,6,7,8 our workspace
       DATA 0,10,11,MEM,13,14,15
 STACK  DATA ROOT,&gt;0001        trace-back stack
       BSS  80                20-level deep: 1 million nodes

You may wonder what's with the stack pointed by R8. I'm using it to save
the route from the root of the tree to the current node: at each step I
save a pointer to the node and a flag saying whether we followed the
left link or the right link. This is not strictly required for the above
routines, but it will be essential for inserting and deleting nodes.
Rather than writing two versions of FIND, one with and one without route
tracking, I decided to implement it anyhow.

##  Rotations

A tree can be rearranged to restore balance by just rotating a node and
its child:

     A        balance = +2     ----LL------>       B       balance = 0
    / \                         rotate A-B        / \
      B       balance = +1       to the          A   C     balance = 0 0
     / \                          left          / \ / \
        C     balance = 0

Now we should check what the effect on the balance will be. There are
two possibilities, as node B can be balanced or not (B could be balanced
if the rotation was caused by the deletion of a node at the left of A,
for instance).

Old balance
New balance
Tree depth


In fact, one can generalize the above situation and consider the cases
when A and B are not terminal nodes (aka "leaves") but have subtrees
themselves. In the following sheme, subtrees are denoted by lower-case
letters, while individual nodes are represented by upper-case letters.

      A          ----LL------>       B
     / \           rotate           / \
    a1  B          to the          A   b2
       / \          left          / \
      b1  b2                     a1  b1
Obviously, A must have a balance of +2 to cause a rotation, but B could
be balanced or not. If this situation was cause by an insertion (in one
of the B subtrees) , then B cannot be balanced.

#### Tree depth

What will be the effect of such a rotation on the overall tree depth? It
depends on the balance of B and on the operation that caused
imbalancing: deletion or insertion. An insertion will cause subtree b2
to become longer (insetions in b1 require another type of rotation, see
below). As we are moving b2 one level up, it will still reach the same
depth. But what about a1? We know that a1 must be exactly one node
shorter than b2 before the insertion: if it were 2 nodes shorter a
rotation would have occured before, it it were the same length (or
larger) no rotation would be required. We are now moving a1 one level
down, which means that it will reach the same depth than b2 prior to the
insertion. In conclusion, the tree depth is not affected by a rotation
caused by an insertion.

Things are trickier with a deletion since B could be imbalanced prior to
the deletion: b2 could be larger than b1 (the opposite is also possible
but calls for another type of rotation). After the deletion, the
rotation moved b2 one level higher, which shortened the overall depth of
the tree (of course a1 went down, but remember we deleted a node in it,
so it reaches the same depth as before). If B was balanced before the
deletion, the overall tree depth remains unchanged because b1 did not
move and reaches the same depth as before.

Why are we so concerned about tree depth? Because we must consider the
possibility that our "tree" could in fact be a subtree of a larger tree.
In which case, if the depth changes, we must step one level up and see
what will be the effect it has on the balance of the parent node. But
this will be discussed later.

#### Other rotations

For now, I'd like to discuss the other types of rotations: firstly, we
have the mirror-image of the above situation, which can be solved by a
rotation to the right:

        B        ----RR------>       A
       / \          rotate          / \
      A   b         to the         a1  B
     / \            right             / \
    a1 a2                            a2  b

Old balance
New balance
Tree depth


This situation is a bit trickier and requires a double rotation, first
right between B and C, then left between A and B:

      A                      A      --L-->     B
     / \                    / \               / \
    a1  C       --R-->     a1  B             A   C
       / \                    / \           / \ / \
      B  c1                  b1 C         a1 b1 b2 c1
     / \                       / \
    b1 b2                     b2 c1

There are three possible cases, depending on the balance of B (in the
case of an insertion, there are only two cases, as B cannot be balanced
after insertion).

Old balance
New balance
Tree depth


In the case of an insertion, in either b1 or b2, the overall tree length
will not be affected: the expanded subtree is brought one level up, so
it will reach the same depth as before. Subtree c1 does not move so we
don't need to worry about it. And subtree a1 must be exactly one node
shorter than b1 and b2, prior to the insertion (otherwise a rotation
would have occured before, or would not be necessary now). The rotation
brings a1 down by one level, which means that it now reached the same
depth than b1 and b2 prior to the rotation. In conclusion: a
double-rotation caused by an insertion does not change the depth of the
tree.

What about deletions? If a1 is trucated, and b1 and b2 are brought one
level up, the only subtree that could maintain the depth of the tree
would be c1. But we know that c1 does not reach as deep as b1/b2,
otherwise a double-rotation wouldn't be necessary: we could just rotate
A and C. So double-rotations caused be a deletion always change the
depth of the tree.
Finally, the mirror-image of the above situation requires a left-right
double rotation:

        C                    C      --R-->     B
       / \                  / \               / \
      A   c1    --L-->     B  c1             A   C
     / \                  / \               / \ / \
    a1  B                A   b2           a1 b1 b2 c1
       / \              / \
      b1 b2            a1 b1

Did you notice? We ended up with the very same situation than in the
case of a right-left rotation. Now isn't that convenient? It means that
the effects on the balance and tree depth will be the same.

Old balance
New balance
Tree depth


#### Code

Here are the four assembly routines that perform these rotations.
NB. Don't worry about the "left count" lines, this is a feature of
indexed trees, it will be discussed [later](#Indexed%20trees).

* Rotate left once. Parent in R5, child in R6                   / \  ==&gt;   / \
*--------------------------------------------                  a   B      A  b2
ROTLL  MOV  @LEFT(6),0        memorize left grand-child           / \    / \
       MOV  5,@LEFT(6)        put parent at left of child        b1 b2  a  b1
       MOV  0,@RIGHT(5)       and grand-child at right of parent
        AB   @LCNT(5),@LCNT(6) adjust left count in child
       AB   @H01,@LCNT(6)     now becomes parent
        LI   0,&gt;FF00           new balance will shift to the left
SK15   AB   @BAL(6),0         adjust balances after single rotation
       MOVB 0,@BAL(6)         child decremented or incremented
       NEG  0
       MOVB 0,@BAL(5)         parent is invert of child
       MOV  6,7               (grand-)child is now at top
 SK16   MOV  @-4(8),1          get ancestor
       MOVB @-1(8),0          which side did we go ?
       JLT  SK14
       MOV  7,@RIGHT(1)       update link to this level
       JMP  SKE
SK14   MOV  7,@LEFT(1)        ditto, if we went on the other side
SKE    MOVB @BAL(6),0         test balance of top node
       B    *11
*
*---------------------------------------------                    B        A
* Rotate right once. Parent in R5, child in R6                   / \  ==&gt; / \
*---------------------------------------------                  A   b    a1  B
ROTRR  MOV  @RIGHT(6),0       memorize right grand-child       / \          / \
       MOV  5,@RIGHT(6)       put parent at right of child   a1  a2        a2  b
       MOV  0,@LEFT(5)        and grand-child at left of parent
        SB   @LCNT(6),@LCNT(5) adjust left count in parent
       SB   @H01,@LCNT(5)     now becomes child
        LI   0,&gt;0100           balance will shift to the right
       JMP  SK15
*
*---------------------------------------------                 A          B
* Rotate left-right. Parent in R5, child in R6                / \   ==&gt;  / \
*---------------------------------------------               a   C      A    C
ROTLR  MOV  @LEFT(6),7         get left grand-child             / \    / \  / \
       MOV  @LEFT(7),@RIGHT(5) put its left at right of parent B   c  a  b1 b2 c
       MOV  @RIGHT(7),@LEFT(6) its right at left of child     / \
       MOV  5,@LEFT(7)         parent will be at its left    b1  b2
       MOV  6,@RIGHT(7)        child at its right
        SB   @LCNT(7),@LCNT(6)  adjust left count in child
       SB   @H01,@LCNT(6)      now lost the grand-child
       AB   @LCNT(5),@LCNT(7)  adjust left count in grand-child
       AB   @H01,@LCNT(7)      now becomes parent
 SK12   MOVB @H00,@BAL(6)       adjust balances after dobble rotation
       MOVB @BAL(7),0          get grand-child balance
       NEG  0
       JGT  SKF
       JLT  SK10
       MOVB @H00,@BAL(5)       grand-child was balanced: all 3 are now
       MOVB @H00,@BAL(7)
       JMP  SK16               update link in ancestor
 SKF    MOVB 0,@BAL(7)          grand-child balance was -1, now it&#39;s +1
       MOVB @H00,@BAL(5)       parent is now balanced
       JMP  SK16               update link in ancestor
 SK10   MOVB @H00,@BAL(7)       grand-child balance was +1, now it&#39;s 0
       MOVB 0,@BAL(5)          but parent becomes -1
       JMP  SK16               update link in ancestor
*
*---------------------------------------------                      C  ==&gt; same
* Rotate right-left. Parent in R5, child in R6                     / \     as
*---------------------------------------------                    A   c   above
ROTRL  MOV  @RIGHT(6),7        get right grand-child             / \
       MOV  @LEFT(7),@RIGHT(6) put its left at right of child   a   B
       MOV  @RIGHT(7),@LEFT(7) its right at left of parent         / \
       MOV  5,@RIGHT(7)        parent will be at its right        b1  b2
       MOV  6,@LEFT(7)         child at its left
        AB   @LCNT(6),@LCNT(7)  adjust left count for grand-child
       AB   @H01,@LCNT(7)      now becomes parent
       SB   @LCNT(7),@LCNT(5)  adjust left count in parent
       SB   @H01,@LCNT(5)      now becomes child
        JMP  SK12               update balance (same as above)

##  Insertion

Now that we know how to rearrange a tree by rotating a node, we can
insert new elements without imbalancing the tree. First, we insert the
element where it belongs. Then we check the effect on its parent node's
balance. Two cases are possible

     E  --> E                 E ---> E
    / \    / \               / \    / \
              N             D      D   N
    E was balanced          E was imbalanced
    now it's not            now it's balanced

In both cases, the balance of E remains in the acceptable range.
However, in the first case we have increased the depth of the 'E'
subtree by one level. This may result in imbalancing a node above the
level of E. We must thus go one level up, and repeat our checks. This
time, a third situation may occur, when the node was already imbalanced
and is now even more tilted. This will require a rotation.

       B          --->        B           --LL-->         E
      / \                    / \                         / \
         E                      E                       B   N
        / \                    / \                     / \ / \
                                  N
    B was imbalanced     Now it's not aceptable     We must rotate
    but acceptable       (balance B = 2)            to rebalance this level

Theoretically we should check what effect the rotation has on the upper
levels, but rotations caused be insertions never change the depth of the
tree, so we can dispense with it.

The insertion algorithm thus become:

The node was imbalanced, now it is balanced ==\> stop here

The node was balanced, now it's imbalanced ==\> check the effect at the
upper level (if any)

The node was imbalanced, now it's not acceptable ==\> rotate, then stop

* Insert a node into the tree.
* New key value in R4. Ptr to new node into R5
*----------------------------------------------------------
INSERT MOV  11,9
       BL   @FIND             look for value in existing nodes
       JMP  SK8
       B    *9                we found it: error (optional)
 SK8    LI   0,NSIZE           node size
       BL   @NEW              get memory space for node
       MOV  1,2               save ptr
       MOV  4,*1+             save data
       CLR  *1+               clear links
       CLR  *1+
       CLR  *1+               reset balance + count
       BL   @INCNT            ajust counts upstream
        AI   8,-4
       MOV  *8,5              get parent ptr
       MOVB @3(8),0           where did we go ?
       JLT  SK9
       MOV  2,@RIGHT(5)       insert at right of parent
       JMP  SKA
SK9    MOV  2,@LEFT(5)        insert at left of parent
SKA    AB   @BAL(5),0         update parent balance
       JMP  SK23              balanced (one leaf on each side): done
 LP3    CI   8,STACK+4         did we reach top of tree?
       JLE  SK17              yes: done
       AI   8,-4              back up one level
       MOV  *8,5              get node ptr
       MOV  @2(8),0           and its balance
       AB   @3(8),0           tree became heavier on the side we went
 SK23   MOVB 0,@BAL(5)         update balance
       JLT  SKB               tilted on the left
       JGT  SKC               tilted on the right
SK17   B    *9                balanced: done
 SKC    CI   0,&gt;01FF           node is heavy on the right
       JLE  LP3               still ok, but see what will happen upstream
       MOV  @RIGHT(5),6       get heavy child
       MOV  @BAL(6),0         check its balance
       JLT  SKD               it&#39;s heavy on the other side: rotate twice
       BL   @ROTLL            rotate left once
       B    *9                and we are done
 SKD    BL   @ROTLR            rotate left, then right
       B    *9
 SKB    CI   0,&gt;FF00           node is heavy on the left
       JHE  LP3               still ok, but  see what happens above
       MOV  @LEFT(5),6        get heavy child
       MOV  @BAL(6),0         check its balance
       JGT  SK11              it&#39;s heavy on the other side: rotate twice
       BL   @ROTRR            rotate right once
       B    *9
 SK11   BL   @ROTRL            rotate rigth, then left
       B    *9`
*---------------------------------
* User-defined routine.
* Allocate space for a node
* Size in R0, address into R1.
*---------------------------------
NEW    MOV  12,1              here: from a stack, ptr in R12
       A    0,12              update ptr (needs check for overflow)
       B    *11
*
*---------------------------------
* User-defined routine.
* Reclaim space after deletion
* Size in R0, address into R1.
*---------------------------------
FREE   B    *11               don&#39;t bother for this demo`
*
MEM    BSS  whatever          buffer where we put the nodes

##  Deletion

Deletions are more complicated than insertions, because the target node
is not necessarily at the end of a branch. In fact, tree cases are
possible:

       B   --->    B                 B   --->   B                  B  --->   ???
      / \         / \               / \        / \                / \
         R                             R          T                  R
        / \                           / \        / \                / \
                                         T                         M   T
    R is a leaf (no children)    R is a branch (1 child)       R is a subtree (2 children)
    Remove it                    Connect child + parent        We are stuck!

The last case is a bummer, because there is no way we can connect both
children to the parent node. We must use a little trick here:

Find the next in-order node after node R (i.e. the smallest item in its
right subtree).

Install it in place of R. It belongs here since all the other nodes in
the subtree are greater than it.

Now perform the deletion from where that node was. It is always a leaf
or a branch, because it cannot have a left child (otherwise, this child
would be the next in-order node after R).

       B          --->             B           --->            B
      / \                         / \                         / \
         R                           S                           S
        / \                         / \                         / \
       M   T                       M   T                       M   T
          / \                         / \                           \
         S   X                       S   X                           X
    We want to remove R       Install S in place of R      Now delete from were S was
    Next node in order is S

When deleting a node, we must determine the effect it has on its parent
node. Three cases are possible:

The node was balanced, now it's imbalanced (tree depth did not change)
==\> stop here.

The node was imbalanced, now its balanced (we trimmed the longest
subtree) ==\> check the effect at upper levels

The node was imbalanced, now its not acceptable ==\> rotate and check
the effect at upper levels (if tree depth changed)

        C   --->   C                 C   --->   C                C   --->   C
       / \        / \               / \        / \              / \        / \
      A   R      A                     R                       A   R      A
                                                              / \        / \
                                                                 B          B
    Balanced   Imbalanced       Imbalanced   Balanced      Imbalanced   Not acceptable
            Depth unchanged              Tree shortened                 We must rotate

* Remove a node from tree. Key value in R4
*----------------------------------------------------------
REMOVE MOV  11,9
       BL   @FIND             look for value in existing nodes
       B    *9                not found (optionally: issue error)
       MOV  5,1
       BL   @FREE             delete it from memory
       MOV  5,1               save node ptr
       AI   8,-4              ignore this node, in stack (deleted)
 LP5    AI   8,-4              move one step up
       MOV  *8,5              get parent
       MOV  @LEFT(1),6        get left child
       JNE  SK20
       MOV  @RIGHT(1),6       none: get right child, if none node is a leaf
SK21   BL   @DECNT            update counts upstream
       CLR  0
       MOVB @3(8),0           where did we go ?
       NEG  0
       JGT  SK18              left
       MOV  6,@RIGHT(5)       replace in parent link (or clear it)
       JMP  SK24              check effect upstream
SK18   MOV  6,@LEFT(5)        ditto, if we went to the left
SK24   AB   @BAL(5),0         update parent balance
       JMP  SK1A
 SK20   MOV  @RIGHT(1),6       child found at left, get other side
       JEQ  SK21              node is a branch
        MOV  5,7               node is a tree (has two children)
       MOV  @RIGHT(1),5       means we can&#39;t delete it like that
       C    *8+,*8+           keep parent in traceback stack
       MOV  8,2               remember this position, we&#39;ll modify it later
       INCT 8                 don&#39;t know yet what it will be
       MOVB @H01,@-1(8)       we&#39;ll go to the right to find successor
       BL   @MIN              find node successor (smallest item on its right)
       MOV  5,*2              put its pointer in stack
       MOV  @LEFT(1),@LEFT(5) swap data with it (or: just swap keys)
       CLR  @LEFT(1)          successor cannot have smaller child
       MOV  @RIGHT(5),0
       MOV  @RIGHT(1),@RIGHT(5)
       MOV  0,@RIGHT(1)
       MOV  @BAL(5),0
       MOV  @BAL(1),@BAL(5)
       MOV  0,@BAL(1)
       C    1,@RIGHT(7)       check were we went from parent
       JNE  SK22
       MOV  5,@RIGHT(7)       substitute successor in parent link
       JMP  LP5               now remove node from successor&#39;s position
SK22   MOV  5,@LEFT(7)        ditto, if we went left
       JMP  LP5
 LP4    CI   8,STACK+4         did we reach top of tree?
       JLE  SK1F              yes: done
       AI   8,-4              back up one level
       MOV  *8,5              get node ptr
       MOV  @2(8),0           and its balance
       SB   @3(8),0           tree became lighter on the side we went
 SK1A   MOVB 0,@BAL(5)         update balance
       JLT  SK1B              tilted on the left
       JGT  SK1C              tilted on the right
       JMP  LP4               balanced: see effects above (tree shortened)
 SK1C   CI   0,&gt;01FF           node is heavy on the right
       JLE  SK1F              still ok, done (one branch shortened, were equal)
       MOV  @RIGHT(5),6       get heavy child
       MOV  @BAL(6),0         check its balance
       JLT  SK1D              it&#39;s heavy on the other side: rotate twice
       BL   @ROTLL            rotate left once
       JEQ  LP4               if balanced: continue upstream
SK1F   B    *9                else we are done
 SK1D   BL   @ROTLR            rotate left, then right
       JMP  LP4               check effect upstream
 SK1B   CI   0,&gt;FF00           node is heavy on the left
       JHE  SK1F              still ok, done
       MOV  @LEFT(5),6        get heavy child
       MOV  @BAL(6),0         check its balance
       JGT  SK1E              it&#39;s heavy on the other side: rotate twice
       BL   @ROTRR            rotate right once
       JEQ  LP4               if balanced: one level up
       B    *9                else we are done
 SK1E   BL   @ROTRL            rotate rigth, then left
       JMP  LP4               see what it does at upper levels

A faster way to install the next in-order node in place of the current
one would be to just swap keys between the nodes. This however may be
dangerous: if the calling program memorized the location of the nodes,
swapping them in memory could create havoc. Thus, it is safer to swap
pointers but to leave the nodes where they are.

##  Threaded trees

You may have noted in the above drawings that when a node has less than
two children, the remaing links are empty. It seems a pity not to make
use of these. And that's precisely what threaded trees are for.

In a threaded tree, when a node has no right child, this pointer is
replaced with a pointer to the next in-order node (wherever in the tree
this node may be). Similarly, when there is no left child, the left link
is replaced with a pointer to the previous node in order. This requires
some extra code to maintain the pointers, but it allows for faster
travel whithin the tree: finding the next or previous in-order node does
not require tracing back our route to previous levels. As a compromise,
there are right-threaded trees that only have pointers to the next
in-order, and left-threaded trees that only have pointers to the
previous in-order node.

Of course this requires using two extra bits to tell whether a link
points to a child node or to an in-order successor, My suggestion would
be to use the least significant bit: since all addresses are even on the
TI-99/4A, an odd address will indicate an in-order pointer (that last
bit will be cleared when using this pointer).

Here is an exemple of a threaded tree (lower-case letters denote
in-order pointers):

            D                                 D                           D
        /        \                        /         \                /         \
       B         F                       B           F              B           F
      / \      /   \                   /  \       /    \           /  \       /    \
     A   C    E     H                 A    C     E     H          A    C     E     H
    / \ / \  / \   / \               / \  / \   / \   /  \       / \  / \   / \   /  \
                  G   I                b  b d  d  f  G    I         b   d     f  G    I
                 / \ / \                            / \  / \                    / \  / \
                                                   f  h g                          h
    Not threaded                      Fully threaded                Right-threaded

Note how easy it is to walk the treaded tree:

1.  Start from the top of the tree, or from a given node.
2.  From the current node, go all the way to the left, to find the
    smallest node and return it.
3.  Get its right link. If it's an in-order pointer, return this node.
    Then continue at 2.
4.  If it's a regular child pointer, go to 2.

By contrast, here are the routines required to walk a non-threaded tree.
As you see, we must walk our way up the calling stack each time the node
does not have a child on the proper side.

* Find next inorder node (called after FIND, MIN or MAX)
* Current node in R5, route in stack, stack ptr in R8
* Ptr to node into R5, skip JMP if found
*-----------------------------------------------------------
NEXT   MOV  @RIGHT(5),0       get current node&#39;s right child
       JEQ  LP6               none: up one level
       MOVB @H01,@-1(8)       change flag: we are going to the right
       INCT 11                we found one
       MOV  0,5               return this node or ...
       B    @MIN              the smallest item in its right branch
 LP6    AI   8,-4              one level up
       CI   8,STACK+4         root  reached?
       JLE  SK2               yes: no more found
       MOV  @-4(8),5          get last item on stack
       CB   @-1(8),@H01       did we go right from it ?
       JEQ  LP6               yes: then we already returned it
       INCT 11                no: return this node
SK2    B    *11
*
*-----------------------------------------------------------
* Find previous inorder node (called after FIND, MIN or MAX)
* Current node in R5, route in stack, stack ptr in R8
* Ptr to node into R5, skip JMP if found
*-----------------------------------------------------------
PREV   MOV  @LEFT(5),0        get current node&#39;s left child
       JEQ  LP7               none: up one level
       MOVB @HFF,@-1(8)       change flag: we are going to the left
       INCT 11                we found one
       MOV  0,5               return this node or ...
       B    @MAX              the largest item in its left branch
 LP7    AI   8,-4              one level up
       CI   8,STACK+4         root  reached?
       JLE  SK2               yes: no more found
       MOV  @-4(8),5          get last item on stack
       CB   @-1(8),@HFF       did we go left from it ?
       JEQ  LP7               yes: then we already returned it
       INCT 11                no: return this node
       B    *11

## Indexed trees

The problem with a tree structure is that it is quite difficult to
answer questions like: "Which is the 87th node in the tree?", or "What
is the rank of this node whitin the in-order list?". Arrays are much
better at this, but then again arrays are slow to search. Well, maybe we
can have the best of two worlds? All we need to do is to add an extra
variable in each node: the number of nodes in the left subtree. This
will require some overhead code to maintain the count, but it allows us
very fast indexing strategies.

To find the node at position x:

- Start from the top of the tree.
- Compare the target value with its left-count.
  - If it's equal we found it. Return this node
  - If it's lower take the left link and try again
  - If it's higher take the right link, after substracting the
    left-count (+1 for the current node) from the target value.

To find the rank of a node:

1.  Initialize a counter as 0.
2.  Add the left-count of the current node to the counter
3.  Go up one level. Was the node a left-child or a right-child?
    - If it was a left child, ignore the current node and go to 3.
    - If it was a right-child, increment the count and go to 2.
    - If there is no parent node, we are done.

* Find a node from its inorder rank (in R0)
* Return node ptr in R5, skip JMP if found
*----------------------------------------------------------
INDEX  MOV  @ROOT+4,5         start from top
       SLA  0,8               only 256 nodes in this demo
       LI   8,STACK+4         memorize our route on a stack
 LP1    MOV  5,*8+             save node ptr
       MOV  @BAL(5),*8+       save balance
       CB   0,@LCNT(5)        compare to count
       JL   SK25
       JH   SK26
       INCT 11                we found it (index start from 0)
       B    *11               skip jump on return
 SK25   MOVB @HFF,@-1(8)       we must go to the left
       MOV  @LEFT(5),5        get node ptr
       JNE  LP1
       B    *11               no link (should never happen)
 SK26   MOVB @H01,@-1(8)       we must go to the right
       SB   @LCNT(5),0        we leave all these behind us: decount them
       SB   @H01,0            plus current node
       MOV  @RIGHT(5),5       get node ptr
       JNE  LP1
       B    *11               no more: number too big
 *----------------------------------------------------------
* Return inorder rank for node in R5, into R0
* Route in stack, stack ptr in R8
*----------------------------------------------------------
ANK   CLR  0                 init counter
       MOV  @ROOT+4,5         start from top of tree
 LP8    MOVB @LCNT(5),1        get # of items in left subtree
       SRL  1,8               max 255 in this demo
       A    1,0               add to total
 LPA    AI   8,-4              one level up
       CI   8,STACK+4         root  reached?
       JLE  SK27              yes: we are done
       MOV  @-4(8),5          get previous item on stack
       CB   @-1(8),@H01       did we go to the right from it?
       JNE  LPA               no: ignore this node
       INC  0                 yes: then we must count this node
       JMP  LP8               and its left subtree
 SK27   B    *11
*
*-------------------------------------------------------------
* Update left-count after insertion/deletion
* Route in stack, stack ptr in R8 (called by INSERT and REMOVE)
*-------------------------------------------------------------
INCNT  LI   0,&gt;0100           increment count after insertion
       JMP  SK29
 DECNT  LI   0,&gt;FFFF           decrement count after deletion
SK29   MOV  8,1               we want to keep R8 intact
 LP9    AI   1,-4              one level up
       CI   1,STACK+4         root  reached?
       JLE  SK27              yes: we are done
       CB   @-1(1),@HFF       did we go left from there?
       JNE  LP9               no: count unchanged in this node
       MOV  @-4(1),2          get node ptr
       AB   0,@LCNT(2)        update its count
       JMP  LP9               keep going

##  Testing

Here is a sample test program. You can also download the whole file from
[here](avl_s.txt).

* Example of a test program using the above routines
*-----------------------------------------------------------
       DEF  TEST
 TEST   LWPI WREGS              load our workspace
       LI   4,&#39;A &#39;             create a tree
       BL   @INSERT
       LI   4,&#39;B &#39;
       BL   @INSERT
       LI   4,&#39;C &#39;
       BL   @INSERT
       LI   4,&#39;D &#39;
       BL   @INSERT
       LI   4,&#39;E &#39;
       BL   @INSERT
       LI   4,&#39;F &#39;
       BL   @INSERT
       LI   4,&#39;G &#39;
       BL   @INSERT
       LI   4,&#39;H &#39;
       BL   @INSERT
       LI   4,&#39;I &#39;
       BL   @INSERT
       LI   4,&#39;G &#39;            delete node &quot;G &quot;
       BL   @REMOVE
        LI   0,4               find the fifth node (index starts at 0)
       BL   @INDEX
       JMP  ERR               returns here if not found
       LI   4,&#39;I &#39;            find node &quot;I &quot;
       BL   @FIND
       JMP  ERR
       BL   @RANK             return its rank (should be 7)
 ERR    LWPI &gt;20BA             caller&#39;s workspace
       B    *11               exit program
*
       END

## References

The AVL Page. On Ben Pfaff website at
 Comprises many usefull links.

Brad Appleton's AVL library in C language. The doc includes an excellent
explanation of AVL trees.

Revision 1. 6/17/00. Ok to release.
Revision 2. 9/3/00. Discussed tree depth, subtrees. Corrected bugs in
code.
[Back to the TI-99/4A Tech Pages](titechpages.md)
