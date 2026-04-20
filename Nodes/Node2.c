/*
* Node2.c - Distance Vector Routing
* Direct Links: 2-0: 3 2-1: 1 2-3: 2
*   Node connected to all others
*/

#include <stdio.h>
#include <string.h>

extern int clocktime;
extern void tolayer2(struct rtpkt);
extern struct rtpkt {
    int sourceid;
    int destid;
    int mincost[4];
};

#define NumNodes 4
#define INF 9999
#define SELF 2

static const int lc[NumNodes] = { 3, 1, 0, 2};

static const int NEIGHBORS[] = { 0, 1, 3};
#define NumNeighbors 3

struct distanceTable { int costs[NumNodes][NumNodes]; };
static struct distanceTable dt2;

void printdt2(struct distanceTable *dtptr) {
    printf("              via\n");
    printf("  D2 |   0     1     3\n");
    printf("-----------------------\n");
    for (int d = 0; d < NumNodes; d++) {
        if (d == SELF) continue;
        printf("  %3d |", d);
        for (int v = 0; v<NumNodes; v++) {
            if(v == SELF || lc[v]>= INF) continue;
            int c = dtptr->costs[d][v];
            if (c >= INF) printf("  inf");
            else          printf("  %4d", c);
        }
        printf("\n");
    }
    printf("\n");
}

static int mincosts(int out[NumNodes], const int prev[NumNodes]) {
    int changed = 0;
    for (int d = 0; d < NumNodes; d++){
        int best = INF;
        for (int v = 0; v< NumNodes; v++) 
            if (dt2.costs[d][v] < best) best = dt2.costs[d][v];
        out[d] = best;
        if (prev && best != prev[d]) changed = 1;
        
    }
    return changed;

}   

static void broadcast(const int mc[NumNodes]) {
    struct rtpkt pkt;
    pkt.sourceid = SELF;
    memcpy(pkt.mincost, mc, NumNodes * sizeof(int));
    for (int i = 0; i< NumNeighbors; i++) {
        pkt.destid = NEIGHBORS[i];
        printf("  [NODE %d -> NODE %d] sending [%d %d %d %d]\n",
        SELF, pkt.destid, mc[0], mc[1], mc[2], mc[3]);
        tolayer2(pkt);
    }
}

/* rtinit2 */
void rtinit2(void) {
    printf("\n           NODE %d  rtinit2()  t=%-4d          \n",
        SELF, clocktime);
    for (int d = 0; d < NumNodes; d++)
        for (int v = 0; v < NumNodes; v++)
            dt2.costs[d][v] = INF;
    for (int v = 0; v < NumNodes; v++)
        if (lc[v] < INF)
            dt2.costs[v][v] = lc[v];
    
    printdt2(&dt2);

    int mc[NumNodes];
    mincosts(mc, NULL);
    broadcast(mc);
}

/* rtupdate2 */
void rtupdate2(struct rtpkt *rcvdpkt) {
    int from = rcvdpkt->sourceid;
    printf("          NODE %d   rtupdate2()   t=%-4d\n"
   "  packet from node %d  ->  [%d %d %d %d]\n",
    SELF, clocktime, from,
    rcvdpkt->mincost[0], rcvdpkt->mincost[1],
    rcvdpkt->mincost[2], rcvdpkt->mincost[3]);
    
    if (lc[from] >= INF || from == SELF) {
        printf("  DROP - node %d if not a direct neighbor of node %d.\n",
            from,SELF);
        return;
    }

    int prev[NumNodes];
    mincosts(prev, NULL);

    int tableChanged = 0;
    for (int d = 0; d < NumNodes; d++) {
        if (rcvdpkt->mincost[d] >= INF) continue;
        int candidate = lc[from] + rcvdpkt->mincost[d];

        if (candidate < dt2. costs[d][from]) {
            dt2.costs[d][from] = candidate;
            tableChanged = 1;
        }
    }

    if (tableChanged) {
        printf("  Table UPDATED:\n");
        printdt2(&dt2);
        int mc[NumNodes];
        if (mincosts(mc, prev)) {
            printf("  Min-costs changed -> broadcasting to neighbors.\n");
            broadcast(mc);
        } else {
            printf("  Table changed but min-costs unchanged -> no broadcast.\n");
        }
    } else {
        printf("  Table unchanged.\n");
        printdt2(&dt2);
    }
}