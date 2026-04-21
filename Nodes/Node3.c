/*
* Node3 - Distance Vector Routing
*
* Direct link costs from node 3:
*   to node 0 : 7
*   to node 2 : 2
*   NOT connected to node 1 
*
*   Distance table layout:
*       dt3.costs[dest][via] = cost to reach dest using
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

static int lc[4] = { 7, INF, 2, 0 };

struct distance_table {
    int costs[4][4];
} dt3;

printdt3(dtptr)
  struct distance_table *dtptr;
{
  printf("             via     \n");
  printf("   D3 |    0     2 \n");
  printf("  ----|-----------\n");
  printf("     0|  %3d   %3d\n", dtptr->costs[0][0], dtptr->costs[0][2]);
  printf("dest 1|  %3d   %3d\n", dtptr->costs[1][0], dtptr->costs[1][2]);
  printf("     2|  %3d   %3d\n", dtptr->costs[2][0], dtptr->costs[2][2]);
}
 

//Helper: compute min cost to each deset accross all via-columns
static void getmincosts(int out [4])
{
    int d, v; 
    for (d = 0; d < 4; d++) {
        out[d] = INF;
        for (v = 0; v < 4; v++) {
            if (dt3.costs[d][v] < out[d])
                out[d] = dt3.costs[d][v];
        }
    }
}

void rtinit3()
    {
        int d, v;
        int mc[4];
        struct rtpkt pkt;

        printf("\nrtinit3() called at t=%.3f\n", clocktime);

        //Initialise entire table to INF
        for (d = 0; d < 4; d++)
            for (v = 0; v < 4; v++)
                dt3.costs[d][v] = INF;
        
        //Seed direct link costs on the diagonal 
        
        dt3.costs[3][3] = 0;
        dt3.costs[0][0] = 7;
        dt3.costs[2][2] = 2;

        printdt3(&dt3);

        //compute initial min-cost vector
        getmincosts(mc);

        //send to all direct neighbors
        pkt.sourceid = 3;
        memcpy(pkt.mincost, mc, 4 * sizeof(int));

        pkt.destid = 0;
        printf("rtinit3: sending to node 0: [%d %d %d %d]\n",
                mc[0], mc[1], mc[2], mc[3]);
        tolayer2(pkt);

        pkt.destid = 2;
        printf("rinit3: sending to node 2: [%d %d %d %d]\n",
                mc[0], mc[1], mc[2],mc[3]);
        tolayer2(pkt);

    
    }

    void rtupdate3(rcvdpkt)
        struct rtpkt *rcvdpkt;
        {
            int from = rcvdpkt->sourceid;
            int d, changed;
            int prev[4], mc[4];
            struct rtpkt pkt; 

            printf("\nrtupdate3() called at t=%.3f, pkt from node %d [%d %d %d %d]\n",
                    clocktime, from ,
                    rcvdpkt->mincost[0], rcvdpkt->mincost[1],
                    rcvdpkt->mincost[2], rcvdpkt->mincost[3]);

            //Reject packets from non-neighbors
            if (from < 0 || from > 3 || lc[from] >= INF || from == 3) {
                printf("rtupdate3: DROP - node %d not a direct neighbor\n", from);
                return;
            }

            getmincosts(prev);

            //Bellman-For relaxation
            changed = 0;
            for (d = 0; d < 4; d++){
                int candidate;
                if (rcvdpkt->mincost[d] >= INF) continue;
                candidate = lc[from] + rcvdpkt->mincost[d];
                if (candidate < dt3.costs[d][from]) {
                    dt3.costs[d][from] = candidate;
                    changed = 1;
                }
            }

            if (changed) {
                printf("rtupdate3: table updated\n");
                printdt3(&dt3);

                //recompute mins; bnoradcast if min changed
                getmincosts(mc);
                if (mc[0] != prev[0] || mc[1] != prev[1] ||
                    mc[2] != prev[2] || mc[3] != prev[3]){

                        printf("rtupdate3: min-costs changed, broadcasting [%d %d %d %d]\n",
                                mc[0], mc[1], mc[2], mc[3]);
                        pkt.sourceid = 3;
                        memcpy(pkt.mincost, mc, 4 * sizeof(int));

                        pkt.destid = 0; tolayer2(pkt);
                        pkt.destid = 2; tolayer2(pkt);
                    } else {
                        printf("rupdate3: table changed but min-costs same, no broadcast\n");
                    }
            }else {
                printf("rupdat2: table unchanged\n");
                printdt3(&dt3);
            }
        }
