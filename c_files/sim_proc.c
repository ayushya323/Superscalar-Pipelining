#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include "sim_proc.h"
#define table 67
#define clk 1
FILE *FP;
int Rename_Map_Table[table][2];

int size_ROB;
int size_IQ;
int Width;
int OPERATION_type;
int Destination;
int SRC1;
int SRC2;
char *tracefile;

typedef struct
{
    char pc[20];
    int op_type_no;
    int dest_initial;
    int order_of_instr;
    int src1;
    int src1_ready;
    int src2;
    int src2_ready;
    int latency;
    int ready;
    int valid_tag;
    int dest;
} pipeline;

typedef struct
{
    int dest;
    int src1;
    int src2;
    int Di;
    int DI_duration;
    int Is;
    int IS_duration;
    int Ex;
    int EX_duration;
    int Rn;
    int RN_duration;
    int Retire;
    int Retire_duration;
    int op_type;
    int Rr;
    int RR_duration;
    int Wb;
    int WB_duration;
    int Fe;
    int FE_duration;
    int De;
    int DE_duration;

} time_info;

pipeline DE[1000];
pipeline RN[1000];
pipeline RR[1000];
pipeline DI[1000];
pipeline IQ[1000];
pipeline EX_list[1000];
pipeline WB[1000];
pipeline ROB[1000];
time_info Time_Schedule[2000000];

void init_Rename_Map_Table()
{   int i,j;
    for (i = 0; i < table; i++)
    {
        for (j = 0; j < 2; j++)
        {
            Rename_Map_Table[i][j] = 0;
        }
    }
}


int Wakeup_Broadcast()
{   int i;
    for (i = 0; i < size_IQ; i++)
    {
        int typecast = WB_first;
        if ((IQ[i].src1 == WB[typecast].dest))
        {
            if ((WB[typecast].valid_tag == 1))
            {
                IQ[i].src1_ready = 1;
            }
        }

        if ((IQ[i].src2 == WB[typecast].dest))
        {
            if (WB[typecast].valid_tag == 1)
            {
                IQ[i].src2_ready = 1;
            }
        }
    }

    for (i = 0; i < Width; i++)
    {
        int typecast = WB_first;
        if ((DI[i].src1 == WB[typecast].dest))
        {
            if (WB[typecast].valid_tag == 1)
            {
                DI[i].src1_ready = 1;
            }
        }

        if ((DI[i].src2 == WB[typecast].dest))
        {
            if (WB[typecast].valid_tag == 1)
            {
                DI[i].src2_ready = 1;
            }
        }
    }

    for (i = 0; i < Width; i++)
    {
        int typecast = WB_first;

        if (RR[i].src2 == WB[typecast].dest)
        {
            if (WB[typecast].valid_tag == 1)
            {
                RR[i].src2_ready = 1;
            }
        }

        if (RR[i].src1 == WB[typecast].dest)
        {
            if (WB[typecast].valid_tag == 1)
            {
                RR[i].src1_ready = 1;
            }
        }
    }
    return 0;
}



int Increment_Front()
{
    FRONT_linklist++;

    if (FRONT_linklist == size_ROB)
        FRONT_linklist = 0;
    return 0;
}

int Increment_End()
{
    END_linklist++;

    if (END_linklist == size_ROB)
    {
        END_linklist = 0;

    }
    return 0;
}

int check_space_queue(pipeline reg[])
{   int i;
    for ( i = 0; i < Width; i++)
    {
        int state = reg[i].valid_tag;
        if (state == 0)
            empty++;
    }
    int track = empty;
    empty = 0;
    return track >= Width ? 1 : 0;
}

int init_time(int order)
{
    Time_Schedule[order].op_type = OPERATION_type;
    Time_Schedule[order].dest = Destination;
    Time_Schedule[order].src1 = SRC1;
    Time_Schedule[order].src2 = SRC2;
    Time_Schedule[order].Fe = Cycle;
    Time_Schedule[order].FE_duration = 1;
    Time_Schedule[order].De = Cycle + 1;
    return 0;
}
int check_OPERATION_type(int i)
{
    if (DE[i].op_type_no == 0)
    {
        DE[i].latency = 1;
        return 0;
    }
    else if (DE[i].op_type_no == 1)
    {
        DE[i].latency = 2;
        return 0;
    }
    else if (DE[i].op_type_no == 2)
    {
        DE[i].latency = 5;
        return 0;
    }
}

int init_values(int i)
{
    DE[i].op_type_no = OPERATION_type;
    DE[i].dest = Destination;
    DE[i].src1 = SRC1;
    DE[i].src2 = SRC2;
    DE[i].order_of_instr = Pipeline_stage - 1;
    DE[i].valid_tag = 1;
    DE[i].dest_initial = Destination;
    return 0;
}

int Fetch()
{   int i;
    if (Pipeline_stage < 10040)
    {
        if (check_space_queue(DE) == 1)
        {
            for (i= 0; i < Width; i++)
            {
                if (feof(FP) != 1)
                {
                    int order;
                    fscanf(FP, "%s", pc);
                    fscanf(FP, "%d", &OPERATION_type);
                    fscanf(FP, "%d", &Destination);
                    fscanf(FP, "%d", &SRC1);
                    fscanf(FP, "%d", &SRC2);
                    order = Pipeline_stage - clk;

                    init_values(i);
                    check_OPERATION_type(i);
                    init_time(order);

                    Pipeline_stage++;
                    DE_pipeline_stage++;
                }
                else
                    break;
            }
        }
    }
    else
        return 0;

    return 0;
}

int transfer_value(int i, pipeline tab1[], pipeline tab2[])
{

    tab1[i].order_of_instr = tab2[i].order_of_instr;
    tab1[i].valid_tag = tab2[i].valid_tag;
    tab1[i].dest = tab2[i].dest;
    tab1[i].src1 = tab2[i].src1;
    tab1[i].src1_ready = tab2[i].src1_ready;
    tab1[i].src2 = tab2[i].src2;
    tab1[i].src2_ready = tab2[i].src2_ready;
    tab1[i].op_type_no = tab2[i].op_type_no;
    tab1[i].latency = tab2[i].latency;
    tab1[i].ready = tab2[i].ready;
    tab1[i].dest_initial = tab2[i].dest_initial;
    return 0;
}

int Decode()
{   int i;
    int tif = 0;
    if (check_space_queue(DE) == 0)
    {
        if (check_space_queue(RN) == 1)
        {
            for (i = 0; i < DE_pipeline_stage; i++) 
            {
                tif = DE[i].order_of_instr;
                Time_Schedule[tif].DE_duration = 1 + Cycle - Time_Schedule[tif].De;

                transfer_value(i, RN, DE);
                Time_Schedule[RN[i].order_of_instr].Rn = Cycle + 1;

                DE[i].valid_tag = 0;
            }
            RN_pipeline_stage = DE_pipeline_stage;
            DE_pipeline_stage = 0;
        }
    }
    return 0;
}

int Rename()
{   int i;
    if (check_space_queue(RN) == 0)
    {
        for ( i = 0; i < size_ROB; i++)
        {
            if (ROB[i].valid_tag == 0)
                ROB_void++;
        }

        if (ROB_void >= RN_pipeline_stage)
            ROB_1 = 1;
        else
            ROB_1 = 0;

        if ((check_space_queue(RR) == 1) && (ROB_1 == 1))
        {
            for (i = 0; i < RN_pipeline_stage; i++)
            {

                ROB[END_linklist].order_of_instr = RN[i].order_of_instr;
                ROB[END_linklist].valid_tag = RN[i].valid_tag;
                ROB[END_linklist].dest = RN[i].dest;
                ROB[END_linklist].src1 = RN[i].src1;
                ROB[END_linklist].src1_ready = RN[i].src1_ready;
                ROB[END_linklist].src2 = RN[i].src2;
                ROB[END_linklist].src2_ready = RN[i].src2_ready;
                ROB[END_linklist].op_type_no = RN[i].op_type_no;
                ROB[END_linklist].latency = RN[i].latency;
                ROB[END_linklist].ready = RN[i].ready;
                ROB[END_linklist].dest_initial = RN[i].dest_initial;

                int typecast = RN[i].src2;
                if (typecast == -1)
                {
                    (RN[i].src2_ready = 1);
                }
                else
                {
                    if (Rename_Map_Table[typecast][0] == 1) 
                        RN[i].src2 = Rename_Map_Table[typecast][1];
                    else
                        RN[i].src2_ready = 1;
                }

                int flex = RN[i].src1;

                if (flex == -1) 
                    (RN[i].src1_ready = 1);
                else
                {
                    if (Rename_Map_Table[flex][0] == 1)
                        RN[i].src1 = Rename_Map_Table[flex][1];
                    else
                        RN[i].src1_ready = 1;
                }

                if (RN[i].dest != -1) 
                {
                    Rename_Map_Table[RN[i].dest][0] = 1; 
                }
                Rename_Map_Table[RN[i].dest][1] = END_linklist;
                RN[i].dest = END_linklist;
                ROB[END_linklist].dest = END_linklist;

                Increment_End();
            }

            for (i = 0; i < RN_pipeline_stage; i++) 
            {
                Time_Schedule[RN[i].order_of_instr].RN_duration = Cycle + 1 - Time_Schedule[RN[i].order_of_instr].Rn;
            }

            for ( i = 0; i < RN_pipeline_stage; i++)
            {
                transfer_value(i, RR, RN);
                RN[i].valid_tag = 0;
                Time_Schedule[RR[i].order_of_instr].Rr = Cycle + 1;
            }

          
            RR_pipeline_stage = RN_pipeline_stage;
            RN_pipeline_stage = 0;
        }
    }

    ROB_void = 0;
    return 0;
}

int Register_Read()
{   int i;
    if (check_space_queue(RR) == 0)
    {
        if (check_space_queue(DI) == 1)
        {
            for ( i = 0; i < RR_pipeline_stage; i++)
            {
                if (RR[i].src1_ready != 1)
                {
                    if (ROB[RR[i].src1].ready == 1)
                    {
                        RR[i].src1_ready = 1;
                    } 
                }
                if (RR[i].src2_ready != 1)
                {
                    if (ROB[RR[i].src2].ready == 1)
                    {
                        RR[i].src2_ready = 1;
                    }
                }
            }

            for (i = 0; i < RR_pipeline_stage; i++) 
            {
                Time_Schedule[RR[i].order_of_instr].RR_duration = Cycle + 1 - Time_Schedule[RR[i].order_of_instr].Rr;
            }

            for (i = 0; i < RR_pipeline_stage; i++)
            {
                transfer_value(i, DI, RR);
                RR[i].valid_tag = 0;
                Time_Schedule[DI[i].order_of_instr].Di = Cycle + 1;
            }
           
            DI_pipeline_stage = RR_pipeline_stage;
            RR_pipeline_stage = 0;
        }
    }
    return 0;
}

int Dispatch_List()
{   int i,q,j;
    if (check_space_queue(DI) == 0)
    {
        for (i = 0; i < size_IQ; i++)
        {
            if (IQ[i].valid_tag == 0)
                IQ_void++;
        }

        if (IQ_void >= DI_pipeline_stage)
            IQ_1 = 1;
        else
            IQ_1 = 0;

        if (IQ_1 == 1)
        {
            for (i = 0; i < DI_pipeline_stage; i++)
            {
                for (q = 0; q < size_IQ; q++) 
                {
                    if (IQ[q].valid_tag == 0)
                    {
                        IQ_firsT = q;
                        break;
                    }
                }

                for (j = 0; j < 11; j++) 
                {
                    IQ[IQ_firsT].order_of_instr = DI[i].order_of_instr;
                    IQ[IQ_firsT].valid_tag = DI[i].valid_tag;
                    IQ[IQ_firsT].dest = DI[i].dest;
                    IQ[IQ_firsT].src1 = DI[i].src1;
                    IQ[IQ_firsT].src1_ready = DI[i].src1_ready;
                    IQ[IQ_firsT].src2 = DI[i].src2;
                    IQ[IQ_firsT].src2_ready = DI[i].src2_ready;
                    IQ[IQ_firsT].op_type_no = DI[i].op_type_no;
                    IQ[IQ_firsT].latency = DI[i].latency;
                    IQ[IQ_firsT].ready = DI[i].ready;
                    IQ[IQ_firsT].dest_initial = DI[i].dest_initial;

                    Time_Schedule[DI[i].order_of_instr].DI_duration = Cycle + 1 - Time_Schedule[DI[i].order_of_instr].Di;
                    Time_Schedule[DI[i].order_of_instr].Is = Cycle + 1;
                }

                DI[i].valid_tag = 0; 
            }
            DI_pipeline_stage = 0;
        }
    }

    IQ_void = 0;
    return 0;
}

int check_ready(int g)
{
    if ((IQ[g].valid_tag == 1) && (IQ[g].src1_ready == 1) && (IQ[g].src2_ready == 1))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int Issue_Queue()
{   
    int i, g ,k,p;
    for (i = 0; i < size_IQ; i++) 
    {
        if (IQ[i].valid_tag == 0)
            IQ_zero++;
    }

    if (IQ_zero >= size_IQ)
        IQ_finish = 1;
    else
        IQ_finish = 0;

    if (IQ_finish == 0) 
    {
        for ( i = 0; i < Width; i++)
        {
            for (g = 0; g < size_IQ; g++)
            {
                int readyy = check_ready(g);
                if (readyy == 1)
                {
                    csgo = g;
                    trigger = IQ[g].order_of_instr;
                    break;
                }
            }

            for (k = 0; k < size_IQ; k++)
            {
                int readyy = check_ready(k);

                if (readyy == 1)
                {
                    if (trigger > IQ[k].order_of_instr)
                    {
                        trigger = IQ[k].order_of_instr;
                        csgo = k;
                    }
                }
            }

         
            int rest = check_ready(csgo);

            if (rest == 1)
            {
                for ( p = 0; p < Width * 5; p++)
                {
                    if (EX_list[p].valid_tag == 0)
                    {
                        EX_first = p;
                        break;
                    }
                }

                EX_list[EX_first].order_of_instr = IQ[csgo].order_of_instr;
                EX_list[EX_first].valid_tag = IQ[csgo].valid_tag;
                EX_list[EX_first].dest = IQ[csgo].dest;
                EX_list[EX_first].src1 = IQ[csgo].src1;
                EX_list[EX_first].src1_ready = IQ[csgo].src1_ready;
                EX_list[EX_first].src2 = IQ[csgo].src2;
                EX_list[EX_first].src2_ready = IQ[csgo].src2_ready;
                EX_list[EX_first].op_type_no = IQ[csgo].op_type_no;
                EX_list[EX_first].latency = IQ[csgo].latency;
                EX_list[EX_first].ready = IQ[csgo].ready;
                EX_list[EX_first].dest_initial = IQ[csgo].dest_initial;

                Time_Schedule[IQ[csgo].order_of_instr].IS_duration = Cycle + 1 - Time_Schedule[IQ[csgo].order_of_instr].Is;
             

                Time_Schedule[IQ[csgo].order_of_instr].Ex = Cycle + 1;

                IQ[csgo].valid_tag = 0; 
            }
        }
    }
    IQ_zero = 0;
    return 0;
}
int decrement(int p)
{
    EX_list[p].latency--;
    return 0;
}

int Execute_List()
{   
    int p,q;

    for ( p = 0; p < (Width * 5); p++)
    {
        if (EX_list[p].valid_tag == 1)
        {
            decrement(p);

            if (EX_list[p].latency == 0)
            {
                for (q = 0; q < Width * 5; q++) 
                {
                    if (WB[q].valid_tag == 0)
                    {
                        WB_first = q;
                        break;
                    }
                }

                WB[WB_first].order_of_instr = EX_list[p].order_of_instr;
                WB[WB_first].valid_tag = EX_list[p].valid_tag;
                WB[WB_first].dest = EX_list[p].dest;
                WB[WB_first].src1 = EX_list[p].src1;
                WB[WB_first].src1_ready = EX_list[p].src1_ready;
                WB[WB_first].src2 = EX_list[p].src2;
                WB[WB_first].src2_ready = EX_list[p].src2_ready;
                WB[WB_first].op_type_no = EX_list[p].op_type_no;
                WB[WB_first].latency = EX_list[p].latency;
                WB[WB_first].ready = EX_list[p].ready;
                WB[WB_first].dest_initial = EX_list[p].dest_initial;

                Time_Schedule[EX_list[p].order_of_instr].EX_duration = Cycle + 1 - Time_Schedule[EX_list[p].order_of_instr].Ex;
                Time_Schedule[EX_list[p].order_of_instr].Wb = Cycle + 1;

                EX_list[p].valid_tag = 0; 
            }

            Wakeup_Broadcast();
        }
    }
    return 0;
}

int Writeback()
{
    int i;

    for (i = 0; i < (Width * 5); i++) 
    {
        if (WB[i].valid_tag == 0)
            WB_void++;
    }

    if (WB_void >= (Width * 5))
        WB_zero = 1;
    else
        WB_zero = 0;

    if (WB_zero == 0) 
    {
        for (i = 0; i < (Width * 5); i++)
        {
            if (WB[i].valid_tag == 1)
            {
                int destt = WB[i].dest;

                ROB[destt].ready = 1; 

                Time_Schedule[WB[i].order_of_instr].WB_duration = Cycle - Time_Schedule[WB[i].order_of_instr].Wb + 1;
                Time_Schedule[WB[i].order_of_instr].Retire = Cycle + 1;

                WB[i].valid_tag = 0; 
            }
        }
    }
    WB_void = 0;
    return 0;
}

int Retire()
{
    int i;

    for (i = 0; i < size_ROB; i++) 
    {
        if (ROB[i].valid_tag == 0)
            ROB_zero++;
    }

    if (ROB_zero >= size_ROB)
    {
        ROB_finish = 1;
    }
    else
    {
        ROB_finish = 0;
    }

    if (ROB_finish == 0) 
    {
        for ( i = 0; i < Width; i++)
        {
            if ((ROB[FRONT_linklist].ready == 1))
            {
                if (ROB[FRONT_linklist].valid_tag == 1)
                {
                    ROB[FRONT_linklist].valid_tag = 0;
                    Retire_finish++;

                    Time_Schedule[ROB[FRONT_linklist].order_of_instr].Retire_duration = Cycle + 1 - Time_Schedule[ROB[FRONT_linklist].order_of_instr].Retire;

                    if (ROB[FRONT_linklist].dest_initial != -1)
                    {
                        if (Rename_Map_Table[ROB[FRONT_linklist].dest_initial][1] == FRONT_linklist)
                        {
                            Rename_Map_Table[ROB[FRONT_linklist].dest_initial][0] = 0;
                        }
                    }

                    Increment_Front();
                }
            }
        }
    }
    ROB_zero = 0;
    return 0;
}

int AdvanceCycle()
{
    Cycle++;

    if (Retire_finish == Pipeline_stage - 1)
    {
        fclose(FP);
        return 0;
    }
    else
        return 1;
}

int main(int argc, char *argv[])
{
    int i;
    size_ROB = atoi(argv[1]);
    size_IQ = atoi(argv[2]);
    Width = atoi(argv[3]);
    tracefile = argv[4];
    init_Rename_Map_Table();
    FP = fopen(tracefile, "r");

    do
    {
        Retire();
        Writeback();
        Execute_List();
        Issue_Queue();
        Dispatch_List();
        Register_Read();
        Rename();
        Decode();
        Fetch();
    } while (AdvanceCycle());

    for ( i = 0; i < 10000; i++)
    {

        printf("%d \t", i);
        printf(" fu{%d}\t", Time_Schedule[i].op_type);
        printf(" src{%d,%d}\t", Time_Schedule[i].src1, Time_Schedule[i].src2);
        printf("dest{%d}\t", Time_Schedule[i].dest);
        printf(" FE{%d,%d}\t", Time_Schedule[i].Fe, Time_Schedule[i].FE_duration);
        printf(" DE{%d,%d}\t", Time_Schedule[i].De, Time_Schedule[i].DE_duration);
        printf(" RN{%d,%d}\t", Time_Schedule[i].Rn, Time_Schedule[i].RN_duration);
        printf(" RR{%d,%d}\t", Time_Schedule[i].Rr, Time_Schedule[i].RR_duration);
        printf(" DI{%d,%d}\t", Time_Schedule[i].Di, Time_Schedule[i].DI_duration);
        printf(" IS{%d,%d}\t", Time_Schedule[i].Is, Time_Schedule[i].IS_duration);
        printf(" EX{%d,%d}\t", Time_Schedule[i].Ex, Time_Schedule[i].EX_duration);
        printf(" WB{%d,%d}\t", Time_Schedule[i].Wb, Time_Schedule[i].WB_duration);
        printf(" RT{%d,%d}\t", Time_Schedule[i].Retire, Time_Schedule[i].Retire_duration);
        printf("\n");
    }

    
    printf("# === Simulator Command ========= \n");
    printf("# ./sim %d %d %d %s \n",size_ROB,size_IQ, Width, tracefile);
    printf("# === Processor Configuration === \n");
    printf("# ROB_SIZE=%d \n", size_ROB);
    printf("# IQ_SIZE=%d \n", size_IQ);
    printf("# WIDTH=%d \n", Width);
    printf("# === Simulation Results ========\n");
    printf("# Dynamic Instruction Count = 10000 \n");
    printf("# Cycles = %d \n", (Time_Schedule[9999].Retire + Time_Schedule[9999].Retire_duration));
    float v = (float)(10000) / ((Time_Schedule[9999].Retire + Time_Schedule[9999].Retire_duration));
    printf("# Instructions Per Cycle (IPC) = %0.2f \n", v);

}
