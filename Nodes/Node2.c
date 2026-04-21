/*
* Node2 - Distance Vector Routing
*
* Direct link costs from node 2:
*   to node 0 : 3
*   to node 1 : 1
*   to node 3 : 2
*   Node 2 is connected to ALL other nodes. 
*
*   Distance table layout:
*       dt2.costs[dest][via] = cost to reach dest using
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

static int lc[4] = { 3, 1, 0, 2 };

struct distance_table {
    int costs[4][4];
} dt2;

printdt2(dtptr)
  struct distance_table *dtptr;
{
  printf("                via     \n");
  printf("   D2 |    0     1    3 \n");
  printf("  ----|-----------------\n");
  printf("     0|  %3d   %3d   %3d\n",
         dtptr->costs[0][0], dtptr->costs[0][1], dtptr->costs[0][3]);
  printf("dest 1|  %3d   %3d   %3d\n",
         dtptr->costs[1][0], dtptr->costs[1][1], dtptr->costs[1][3]);
  printf("     3|  %3d   %3d   %3d\n",
         dtptr->costs[3][0], dtptr->costs[3][1], dtptr->costs[3][3]);
}
 

//Helper: compute min cost to each deset accross all via-columns
static void getmincosts(int out [4])
{
    int d, v; 
    for (d = 0; d < 4; d++) {
        out[d] = INF;
        for (v = 0; v < 4; v++) {
            if (dt2.costs[d][v] < out[d])
                out[d] = dt2.costs[d][v];
        }
    }
}

void rtinit2()
    {
        int d, v;
        int mc[4];
        struct rtpkt pkt;

        printf("\nrtinit2() called at t=%.3f\n", clocktime);

        //Initialise entire table to INF
        for (d = 0; d < 4; d++)
            for (v = 0; v < 4; v++)
                dt2.costs[d][v] = INF;
        
        //Seed direct link costs on the diagonal 
        
        dt2.costs[2][2] = 0;
        dt2.costs[0][0] = 3;
        dt2.costs[1][1] = 1;
        dt2.costs[3][3] = 2;

        printdt2(&dt2);

        //compute initial min-cost vector
        getmincosts(mc);

        //send to all direct neighbors
        pkt.sourceid = 2;
        memcpy(pkt.mincost, mc, 4 * sizeof(int));

        pkt.destid = 0;
        printf("rtinit2: sending to node 0: [%d %d %d %d]\n",
                mc[0], mc[1], mc[2], mc[3]);
        tolayer2(pkt);

        pkt.destid = 1;
        printf("rinit2: sending to node 1: [%d %d %d %d]\n",
                mc[0], mc[1], mc[2],mc[3]);
        tolayer2(pkt);

        pkt.destid = 3;
        printf("rinit2: sending to node 3: [%d %d %d %d]\n",
                mc[0], mc[1], mc[2],mc[3]);
        tolayer2(pkt);

    
    }

    void rtupdate2(rcvdpkt)
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
            if (from < 0 || from > 3 || lc[from] >= INF || from == 2) {
                printf("rtupdate2: DROP - node %d not a direct neighbor\n", from);
                return;
            }

            getmincosts(prev);

            //Bellman-For relaxation
            changed = 0;
            for (d = 0; d < 4; d++){
                int candidate;
                if (rcvdpkt->mincost[d] >= INF) continue;
                candidate = lc[from] + rcvdpkt->mincost[d];
                if (candidate < dt2.costs[d][from]) {
                    dt2.costs[d][from] = candidate;
                    changed = 1;
                }
            }

            if (changed) {
                printf("rtupdate2: table updated\n");
                printdt2(&dt2);

                //recompute mins; bnoradcast if min changed
                getmincosts(mc);
                if (mc[0] != prev[0] || mc[1] != prev[1] ||
                    mc[2] != prev[2] || mc[3] != prev[3]){

                        printf("rtupdate2: min-costs changed, broadcasting [%d %d %d %d]\n",
                                mc[0], mc[1], mc[2], mc[3]);
                        pkt.sourceid = 2;
                        memcpy(pkt.mincost, mc, 4 * sizeof(int));

                        pkt.destid = 0; tolayer2(pkt);
                        pkt.destid = 1; tolayer2(pkt);
                        pkt.destid = 3; tolayer2(pkt);
                    } else {
                        printf("rupdate2: table changed but min-costs same, no broadcast\n");
                    }
            }else {
                printf("rupdat2: table unchanged\n");
                printdt2(&dt2);
            }
        }
