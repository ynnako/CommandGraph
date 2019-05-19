/* 046267 Computer Architecture - Spring 2019 - HW #3 */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"

#define MAX_REG 32

/*this struct holds all the information for each command:
 * the dependency fields are obviously the indicator for each dependency of the command.
 * latency is the latency of each command as in the opsLatency table.
 * startLatency is the time the command will start running.
 * totalLatency is the time the command will finish its execute.
 * */
typedef struct {
	int dependency1 = -1;
	int dependency2 = -1;
	int latency;
	int startLatency = 0;
	int totalLatency = 0;
} command;
typedef command* pCommand;


/*
 * this struct is the DB of the program that's being analyzed.
 * progGraph is a dynamically allocated table of the program's commands, the table is set in a sequential order
 *  - the same order as they appear in the trace file.
 *
 *  progLength is the length of the trace.
 *  falseDepsArray is an array that holds the amount of false dependencies for each one of the 32 reg's*/
typedef struct {
	command *progGraph;
	unsigned int progLength = 0;
	unsigned int falseDepsArray[MAX_REG] = {0};
} prog;
typedef prog* pProg;




ProgCtx analyzeProg(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts) {
	if(!opsLatency || !progTrace || numOfInsts == 0) return PROG_CTX_NULL;
	pProg progHandle = new prog;
	progHandle->progGraph = new command[numOfInsts ];
	progHandle->progLength = numOfInsts;
	int dep1 , dep2 , tmpLatency1 , tmpLatency2;
	int dstCurrentCommand;

	// next loop is for finding dependencies between commands.
	for (unsigned int i = 0 ; i < numOfInsts  ; ++i) { //go over all commands in the trace
		dstCurrentCommand = progTrace[i].dstIdx; // this is just for convenience purposes.

		progHandle->progGraph[i].latency = opsLatency[progTrace[i].opcode]; //the latency of each command
		for (unsigned int j = 1 ; j < numOfInsts ; ++j) { // find all commands that depend on progTrace[i]
			if(j + i < numOfInsts) {
				//check if RAW
				if (dstCurrentCommand == progTrace[j + i].src1Idx){
					progHandle->progGraph[j + i].dependency1 = i;
				}
				if(dstCurrentCommand == progTrace[j + i].src2Idx) {
					progHandle->progGraph[j + i].dependency2 = i;
				}
				if(j == 1) { //we check for false dependencies only one command after progTrace[i]
					//check for WAW
					if (dstCurrentCommand == progTrace[j + i].dstIdx) {
						progHandle->falseDepsArray[dstCurrentCommand]++;
					}
					//check for WAR
					if (progTrace[j + i].dstIdx != dstCurrentCommand && // we only count the amount of times a register was in a false dependency and not how many dependencies were there
						(progTrace[i].src1Idx == progTrace[j + i].dstIdx ||
						 progTrace[i].src2Idx == progTrace[j + i].dstIdx)) {

						progHandle->falseDepsArray[progTrace[j + i].dstIdx]++;
					}
				}
			}
			else break;
		}
	}
	// next loop is to determine depth's of each command
	for (unsigned int k = 0; k < numOfInsts ; ++k) {
		tmpLatency1 = -1;
		tmpLatency2 = -1;
		if(progHandle->progGraph[k].dependency1 == -1 && progHandle->progGraph[k].dependency2 == -1) { 	// this means
																											// this program
																											// has no dependency
			progHandle->progGraph[k].startLatency = 0;
			progHandle->progGraph[k].totalLatency = progHandle->progGraph[k].latency;
		}
		else{ // the command does depend on at least one other command
			dep1 = progHandle->progGraph[k].dependency1;
			dep2 = progHandle->progGraph[k].dependency2;
			if(dep1 != -1){
				tmpLatency1 = progHandle->progGraph[dep1].totalLatency; // go to the first command the is being depended on and get its total latency
			}
			if(dep2 != -1){
				tmpLatency2 = progHandle->progGraph[dep2].totalLatency; // go to the second command the is being depended on and get its total latency
			}
			progHandle->progGraph[k].startLatency = tmpLatency1 > tmpLatency2 ? tmpLatency1 : tmpLatency2 ; //set the start latency to be the latest execution finish time of the commands that the current command depends on
			progHandle->progGraph[k].totalLatency = progHandle->progGraph[k].startLatency + progHandle->progGraph[k].latency; // just add the command's latency to get the time the command will finish its execution.
		}
	}

	return progHandle;
}

void freeProgCtx(ProgCtx ctx) {
	if(!ctx) return;
 	pProg tmp = static_cast<pProg> (ctx);
 	delete [] tmp->progGraph;
 	delete tmp;
}

int getInstDepth(ProgCtx ctx, unsigned int theInst) {
	if(!ctx ) return -1;
	pProg tmp = static_cast<pProg> (ctx);
	if(tmp->progLength <= theInst) return -1;
	return tmp->progGraph[theInst].startLatency;

}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst) {
	pProg tmp = static_cast<pProg> (ctx);
	if(theInst >= tmp->progLength) return -1;
	*src1DepInst = tmp->progGraph[theInst].dependency1;
	*src2DepInst = tmp->progGraph[theInst].dependency2;
	return 0;
}

int getRegfalseDeps(ProgCtx ctx, unsigned int reg){
	if(!ctx || reg > MAX_REG) return -1;
	pProg tmp = static_cast<pProg> (ctx);
	return tmp->falseDepsArray[reg];
}

int getProgDepth(ProgCtx ctx) {
	pProg tmp = static_cast<pProg> (ctx);
	int tmpDepth = -1;
	for (unsigned int i = 0; i < tmp->progLength; ++i) {
		tmpDepth = tmpDepth > tmp->progGraph[i].totalLatency ? tmpDepth : tmp->progGraph[i].totalLatency;
	}
	return tmpDepth;
}


