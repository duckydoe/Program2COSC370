/*
* Node0 - Distance Vector Routing
*
* Distance Table Layout:
*   dt[dest][via] = cost to reach 'dest' going first
*                   through direct naighbor via unused
*                   via-entries stay at INF throughout.
* INF (9999) = unreachable / unknown
*/

#include <stdio.h>
#include <string.h>

/* simulator interface */

extern float clocktime;
extern struct rtpkt {
    int sourceid;
    int destid; 
    int mincost[4];
};
extern void tolayer2(struct rtpkt);

#define NumNodes 4
#define INF 9999
#define SELF 0


/* direct link-cost table 
* lc[v] = cost from SELF to node v */
static const int lc[NumNodes] = {0, 1, 3, 7};

static const int NEIGHBORS[] = { 1, 2, 3};
#define NumNeighbors 3


struct distanceTable { int costs[NumNodes][NumNodes]; };
static struct distanceTable dt0;

/* printdt0 - print node 0's distance table. */
void printdt0(struct distanceTable *dtptr) {
    printf("                 via\n");
    printf("  DO |    1      2      3\n");
    printf("  ---+--------------------\n");
    for (int d = 1; d < NumNodes; d++) {
        printf("  %3d  |", d);
        for (int v = 1; v < NumNodes; v++) {
            int c = dtptr->costs[d][v];
            if (c >= INF) printf("   inf");
            else          printf("  %4d", c);
        }
        printf("\n");
    }
    printf("\n");
}


/* 
* mincosts - sweep the distance table and write the per-dest
* minimum into out[0..NumNodes-1].
*
* Returns 1 if anyout[d] differs from prev[d] (pass NULL
* for prev to skip the comparison).
*/
static int mincosts(int out[NumNodes], const int prev[NumNodes]) {
    int changed = 0;
    for (int d = 0; d < NumNodes; d++) {
        int best = INF;
        for (int v = 0; v < NumNodes; v++)
            if (dt0.costs[d][v] < best) best = dt0.costs[d][v];
        out[d] = best;
        if (prev && best != prev[d]) changed = 1;
    }
    return changed;
}

/* broadcast - send mc[] to every direct neighbor. */
static void broadcast(const int mc[NumNodes]) {
    struct rtpkt pkt;
    pkt.sourceid = SELF;
    memcpy(pkt.mincost, mc, NumNodes * sizeof(int));

    for (int i = 0; i < NumNeighbors; i++) {
        pkt.destid = NEIGHBORS[i];
        printf(" [NODE %d -> NODE %d] sending [%d %d %d %d]\n",
                SELF, pkt.destid, mc[0], mc[1], mc[2], mc[3]);
                tolayer2(pkt);
    }
}

/* rtinit0 - Called once at t=0
*
* Seeds dt with direct link costs; broadcasts initial vector. 
*/
void rtinit0(void) {
    printf("\n          NODE %d  rtinit0()  t=%-4d          \n",
            SELF, );
    
    /* initialises entire table to INF */
    for (int d = 0; d < NumNodes; d++) 
        for (int v = 0; v < NumNodes; v++)
            dt0.costs[d][v] = INF;
    
    /* Seed direct-neighbor costs on the diagonal: dt[v][v] = lc[v] */
    for (int v = 0; v < NumNodes; v++) 
        if (lc[v] < INF)
            dt0.costs[v][v] = lc[v];
    
    printdt0(&dt0);

    int mc[NumNodes];
    mincosts(mc, NULL);
    broadcast(mc);
}


/* rtupdate 0 - called when routing packet arrives
* DV update formula (Bellman-Ford relaxation):
*   dt[d][from] <- min( dt[d][from],
*                       lc[from] + rcvd.mincost[d])
* If any table entry tightens AND a route minimum changes,
* rebroadcast the new min-cost vector to all neighbors.
*/

void rtupdate0(struct rtpkt *rcvdpkt) {
    int from = rcvdpkt->sourceid;

    printf("\n          NODE %d   rtupdate0()   t=%-4d          \n"
        " packet from node %d ->  [%d %d %d %d]\n",
        SELF, clocktime, from, 
        rcvdpkt->mincost[0], rcvdpkt->mincost[1],
        rcvdpkt->mincost[2], rcvdpkt->mincost[3]);

    /* reject packets from non-neighbors */
    if (lc[from] >= INF || from == SELF) {
        printf("  DROP - node %d is not a direct neighbor of node %d.\n",
            from, SELF);
        return;
    }

    /* snapshot current minimums before modifications */
    int prev[NumNodes];
    mincosts(prev, NULL);

    /* Bellman-Ford relaxation */
    int tableChanged = 0;
    for (int d = 0; d < NumNodes; d++) {

        if (rcvdpkt->mincost[d] >= INF) continue;
        int candidate = lc[from] + rcvdpkt->mincost[d];
        if (candidate < dt0.costs[d][from]) {
            dt0.costs[d][from] = candidate;
            tableChanged = 1;
        }
    }

    if (tableChanged) {
        printf("  Table UPDATED:\n");
        printdt0(&dt0);

        int mc[NumNodes];
        if (mincosts(mc, prev)) {
            printf(" Min-costs changed -> broadcasting to neighbors.\n");
            broadcast(mc);
        }else {
            printf("  Table changed but min-costs unchaged -> no broadcast.\n");
        }
    }else {
        printf("  Table unchanged.\n");
        printdt0(&dt0);
    }
}
