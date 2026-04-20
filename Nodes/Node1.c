/*
* Node0 - Distance Vector Routing
*
* Direct link costs from node 0:
*   to node 0 : 1
*   to node 2 : 1
*
*   Distance table layout:
*       dt1.costs[dest][via] = cost to reach dest using
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

static int lc[4] = { 1, 0, 1, INF };

int connectcosts1[4] = { 1, 0, 1, 999 };

struct distance_table {
    int costs[4][4];
} dt1;

printdt0(dtptr)
     struct distance_table *dtptr;
{
  printf("             via   \n");
  printf("   D1 |    0     2 \n");
  printf("  ----|-----------\n");
  printf("     0|  %3d   %3d\n", dtptr->costs[0][0], dtptr->costs[0][2]);
  printf("dest 2|  %3d   %3d\n", dtptr->costs[2][0], dtptr->costs[2][2]);
  printf("     3|  %3d   %3d\n", dtptr->costs[3][0], dtptr->costs[3][2]);
}
 

//Helper: compute min cost to each deset accross all via-columns
static void getmincosts(int out [4])
{
    int d, v; 
    for (d = 0; d < 4; d++) {
        out[d] = INF;
        for (v = 0; v < 4; v++) {
            if (dt1.costs[d][v] < out[d])
                out[d] = dt1.costs[d][v];
        }
    }
}

rtinit1()
    {
        int d, v;
        int mc[4];
        struct rtpkt pkt;

        printf("\nrtinit1() called at t=%.3f\n", clocktime);

        //Initialise entire table to INF
        for (d = 0; d < 4; d++)
            for (v = 0; v < 4; v++)
                dt1.costs[d][v] = INF;
        
        //Seed direct link costs on the diagonal 
        
        dt1.costs[1][1] = 0;
        dt1.costs[0][0] = 1;
        dt1.costs[2][2] = 1;

        printdt1(&dt1);

        //compute initial min-cost vector
        getmincosts(mc);

        //send to all direct neighbors
        pkt.sourceid = 1;
        memcpy(pkt.mincost, mc, 4 * sizeof(int));

        pkt.destid = 0;
        printf("rtinit1: sending to node 0: [%d %d %d %d]\n",
                mc[0], mc[1], mc[2], mc[3]);
        tolayer2(pkt);

        pkt.destid = 2;
        printf("rinit1: sending to node 2: [%d %d %d %d]\n",
                mc[0], mc[1], mc[2],mc[3]);
        tolayer2(pkt);

    
    }

    rtupdate1(rcvdpkt)
        struct rtpkt *rcvdpkt;
        {
            int from = rcvdpkt->sourceid;
            int d, changed;
            int prev[4], mc[4];
            struct rtpkt pkt; 

            printf("\nrtupdate1() called at t=%.3f, pkt from node %d [%d %d %d %d]\n",
                    clocktime, from ,
                    rcvdpkt->mincost[0], rcvdpkt->mincost[1],
                    rcvdpkt->mincost[2], rcvdpkt->mincost[3]);

            //Reject packets from non-neighbors
            if (from < 0 || from > 3 || lc[from] >= INF || from == 1) {
                printf("rtupdate1: DROP - node %d not a direct neighbor\n", from);
                return;
            }

            getmincosts(prev);

            //Bellman-For relaxation
            changed = 0;
            for (d = 0; d < 4; d++){
                int candidate;
                if (rcvdpkt->mincost[d] >= INF) continue;
                candidate = lc[from] + rcvdpkt->mincost[d];
                if (candidate < dt1.costs[d][from]) {
                    dt1.costs[d][from] = candidate;
                    changed = 1;
                }
            }

            if (changed) {
                printf("rtupdate0: table updated\n");
                printdt1(&dt1);

                //recompute mins; bnoradcast if min changed
                getmincosts(mc);
                if (mc[0] != prev[0] || mc[1] != prev[1] ||
                    mc[2] != prev[2] || mc[3] != prev[3]){

                        printf("rtupdate1: min-costs changed, broadcasting [%d %d %d %d]\n",
                                mc[0], mc[1], mc[2], mc[3]);
                        pkt.sourceid = 0;
                        memcpy(pkt.mincost, mc, 4 * sizeof(int));

                        pkt.destid = 1; tolayer2(pkt);
                        pkt.destid = 2; tolayer2(pkt);
                        pkt.destid = 3; tolayer2(pkt);
                    } else {
                        printf("rupdate1: table changed but min-costs same, no broadcast\n");
                    }
            }else {
                printf("rupdate1: table unchanged\n");
                printdt1(&dt1);
            }
        }

        linkhandler0(linkid, newcost)
            int linkid, newcost;

            {
}
