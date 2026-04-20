/*
* Node0 - Distance Vector Routing
*
* Direct link costs from node 0:
*   to node 1 : 1
*   to node 2 : 3
*   to node 3 : 7
*
*   Distance table layout:
*       dt0.costs[dest][via] = cost to reach dest using
*                            direct neighbor via as first hop
*/

#include <stdio.h>
#include <string.h>

extern struct rtpkt {
    int sourceid;
    int destid;
    int mincost[4];
};

extern int TRACE;
extern int YES;
extern int NO;
extern float clocktime;

extern void tolayer2(struct rtpkt);

#define INF 9999

static int lc[4] = { 0, 1, 3, 7 };

struct distance_table {
    int costs[4][4];
} dt0;

printdt0(dtptr)
    struct distance_table *dtptr;
{
    printf("                via     \n");
    printf("   D0 |    1     2    3 \n");
    printf("  ----|-----------------\n");
    printf("     1|  %3d   %3d   %3d\n",
         dtptr->costs[1][1], dtptr->costs[1][2], dtptr->costs[1][3]);
    printf("dest 2|  %3d   %3d   %3d\n",
         dtptr->costs[2][1], dtptr->costs[2][2], dtptr->costs[2][3]);
    printf("     3|  %3d   %3d   %3d\n",
         dtptr->costs[3][1], dtptr->costs[3][2], dtptr->costs[3][3]);
}

//Helper: compute min cost to each deset accross all via-columns
static void getmincosts(int out [4])
{
    int d, v; 
    for (d = 0; d < 4; d++) {
        out[d] = INF;
        for (v = 0; v < 4; v++) {
            if (dt0.costs[d][v] < out[d])
                out[d] = dt0.costs[d][v];
        }
    }
}

void rtinit0()
    {
        int d, v;
        int mc[4];
        struct rtpkt pkt;

        printf("\nrtinit0() called at t=%.3f\n", clocktime);

        //Initialise entire table to INF
        for (d = 0; d < 4; d++)
            for (v = 0; v < 4; v++)
                dt0.costs[d][v] = INF;
        
        //Seed direct link costs on the diagonal 
        dt0.costs[0][0] = 0;
        dt0.costs[1][1] = 1;
        dt0.costs[2][2] = 3;
        dt0.costs[3][3] = 7;

        printdt0(&dt0);

        //compute initial min-cost vector
        getmincosts(mc);

        //send to all direct neighbors
        pkt.sourceid = 0;
        memcpy(pkt.mincost, mc, 4 * sizeof(int));

        pkt.destid = 1;
        printf("rtinit0: sending to node 1: [%d %d %d %d]\n",
                mc[0], mc[1], mc[2], mc[3]);
        tolayer2(pkt);

        pkt.destid = 2;
        printf("rinit0: sending to node 2: [%d %d %d %d]\n",
                mc[0], mc[1], mc[2],mc[3]);
        tolayer2(pkt);

        pkt.destid = 3;
        printf("rinit0: sending to node 3: [%d %d %d %d]\n",
                mc[0], mc[1], mc[2], mc[3]);
        tolayer2(pkt);
    }

    void rtupdate0(rcvdpkt)
        struct rtpkt *rcvdpkt;
        {
            int from = rcvdpkt->sourceid;
            int d, changed;
            int prev[4], mc[4];
            struct rtpkt pkt; 

            printf("\nrtupdate0() called at t=%.3f, pkt from node %d [%d %d %d %d]\n",
                    clocktime, from ,
                    rcvdpkt->mincost[0], rcvdpkt->mincost[1],
                    rcvdpkt->mincost[2], rcvdpkt->mincost[3]);

            //Reject packets from non-neighbors
            if (from < 0 || from > 3 || lc[from] >= INF || from == 0) {
                printf("rtupdate0: DROP - node %d not a direct neighbor\n", from);
                return;
            }

            getmincosts(prev);

            //Bellman-For relaxation
            changed = 0;
            for (d = 0; d < 4; d++){
                int candidate;
                if (rcvdpkt->mincost[d] >= INF) continue;
                candidate = lc[from] + rcvdpkt->mincost[d];
                if (candidate < dt0.costs[d][from]) {
                    dt0.costs[d][from] = candidate;
                    changed = 1;
                }
            }

            if (changed) {
                printf("rtupdate0: table updated\n");
                printdt0(&dt0);

                //recompute mins; bnoradcast if min changed
                getmincosts(mc);
                if (mc[0] != prev[0] || mc[1] != prev[1] ||
                    mc[2] != prev[2] || mc[3] != prev[3]){

                        printf("rtupdate0: min-costs changed, broadcasting [%d %d %d %d]\n",
                                mc[0], mc[1], mc[2], mc[3]);
                        pkt.sourceid = 0;
                        memcpy(pkt.mincost, mc, 4 * sizeof(int));

                        pkt.destid = 1; tolayer2(pkt);
                        pkt.destid = 2; tolayer2(pkt);
                        pkt.destid = 3; tolayer2(pkt);
                    } else {
                        printf("rupdate0: table changed but min-costs same, no broadcast\n");
                    }
            }else {
                printf("rupdate0: table unchanged\n");
                printdt0(&dt0);
            }
        }

        linkhandler0(linkid, newcost)
            int linkid, newcost;

            {
}
