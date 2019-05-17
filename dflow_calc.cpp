/* 046267 Computer Architecture - Spring 2019 - HW #3 */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"
#include <cstdlib>
#include <iostream>
using std::cout;
using std::endl;


#define MAX_REG 32





typedef struct {

	//InstInfo cmdInfo;
	bool visited = false;
	int dependency1 = -1;
	int dependency2 = -1;
	int distFromEntry = -1;
} command;
typedef command* pCommand;

class CmdDepsGraph{
private:

	int numOfInsts_;
	int tableSize_;
	int falseDepsArray_[MAX_REG];
	int **depsMatrix_;
public:
	explicit CmdDepsGraph(unsigned int numOfInsts);
	~CmdDepsGraph();
	pCommand  commandTable_;
	void addMatrixDependency(int commandIdx, int depIdx, int dependencyReg);
	void setFalseDeps(int reg);
	int getFalseDeps(unsigned int reg);
	bool getDeps(unsigned int theInst, int *src1DepInst, int *src2DepInst);
	void printMatrix();



};

CmdDepsGraph::CmdDepsGraph(unsigned int numOfInsts){
	numOfInsts_ = numOfInsts;
	tableSize_ = 0;
	commandTable_ = new command[numOfInsts - 1];
	for (int i = 0; i < numOfInsts; ++i) {
		commandTable_[i].distFromEntry = -1;
		commandTable_[i].dependency1 = -1;
		commandTable_[i].dependency2 = -1;
	}
	for (int l = 0; l < MAX_REG; ++l) {
		falseDepsArray_[l] = -1;
	}
	depsMatrix_ = new int* [numOfInsts];
	for (int i = 0; i < numOfInsts ; ++i) {
		depsMatrix_[i] = new int[numOfInsts];
	}
	for (int j = 0; j < numOfInsts; ++j) {
		for (int k = 0; k < numOfInsts; ++k) {
			depsMatrix_[j][k] = -1;
		}
	}
//	commandTable_[0]->dependency1 = -1;
//	commandTable_[0]->dependency2 = -1;
//	commandTable_[0]->cmdSerialNum = -1;
//	commandTable_[0]->distFromEntry = -1;



}

CmdDepsGraph::~CmdDepsGraph() {
	delete [] commandTable_;
	for (int i = 0; i < numOfInsts_ ; ++i) {
		delete[] depsMatrix_[i];
	}
	delete[] depsMatrix_;
}

void CmdDepsGraph::addMatrixDependency(int commandIdx, int depIdx, int dependencyReg) {
	depsMatrix_[commandIdx][depIdx] = dependencyReg;
}

void CmdDepsGraph::printMatrix() {
	for (int i = 0; i < numOfInsts_; ++i) {
		for (int j = 0; j < numOfInsts_; ++j) {
			cout << depsMatrix_[i][j] << " ";
		}
		cout << endl;
	}
}

void CmdDepsGraph::setFalseDeps(int reg) {
	falseDepsArray_[reg]++;
}

int CmdDepsGraph::getFalseDeps(unsigned int reg) {
	return falseDepsArray_[reg];
}

bool CmdDepsGraph::getDeps(unsigned int theInst, int *src1DepInst, int *src2DepInst) {
	int dep[2] = {-1 ,-1} , idx =0;
	for (int i = numOfInsts_ - 1; i >= 0; --i) {

		if(depsMatrix_[theInst][i] != -1){
			if(idx == 0){
				*src1DepInst = depsMatrix_[theInst][i];
				idx ++;
			}
			else if(idx == 1){
				*src2DepInst = depsMatrix_[theInst][i];
			}
		}
		if(idx >= 2) return true;
	}
}


ProgCtx analyzeProg(const unsigned int opsLatency[], InstInfo progTrace[], unsigned int numOfInsts) {
	CmdDepsGraph *handle = new CmdDepsGraph(numOfInsts);

	for (int i = 0 ; i < numOfInsts ; ++i) {
		for (int j = 1 ; j <= opsLatency[progTrace[i].opcode] ; ++j) {
			if(j + i < numOfInsts) {
				//check if RAW
				if (progTrace[i].dstIdx == progTrace[j + i].src1Idx){
					handle->addMatrixDependency(j + i, i, progTrace[i].dstIdx);
					handle->commandTable_->dependency1 = progTrace[i].dstIdx;
				}
				if(progTrace[i].dstIdx == progTrace[j + i].src2Idx) {
					handle->addMatrixDependency(j + i, i, progTrace[i].dstIdx);
					handle->commandTable_->dependency2 = progTrace[i].dstIdx;
				}
				//check for WAW
				if (progTrace[i].dstIdx == progTrace[j + i].dstIdx) {
					handle->setFalseDeps(progTrace[i].dstIdx);
				}
				//check for WAR
				if(progTrace[i].src1Idx == progTrace[j + i].dstIdx || progTrace[i].src2Idx == progTrace[j + i].dstIdx){
					handle->setFalseDeps(progTrace[j + i].dstIdx);
				}
			}
			else break;
		}
	}
	handle->printMatrix();
	return handle;
}

void freeProgCtx(ProgCtx ctx) {
	CmdDepsGraph *tmp = static_cast<CmdDepsGraph *> (ctx);
	delete tmp;
}

int getInstDepth(ProgCtx ctx, unsigned int theInst) {
	return -1;
}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst) {
	return -1;
}

int getRegfalseDeps(ProgCtx ctx, unsigned int reg){
	CmdDepsGraph *tmp = static_cast<CmdDepsGraph *> (ctx);
	return tmp->getFalseDeps(reg);
}

int getProgDepth(ProgCtx ctx) {
	return 0;
}


