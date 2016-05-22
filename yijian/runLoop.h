// author jiwei.wang

#ifndef __YIJIAN_RUNLOOP_H__
#define __YIJIAN_RUNLOOP_H__

#include "macro.h"
#include <functional>
#include <memory>
#include <map>
#include <vector>
#include <pthread.h>
#include <mutex>
#include <condition_variable>

namespace yijian {

	namespace currentThread {
		__thread unsigned int runloopNum = 0;
	}
	class runLoop;

class runLoop:public noncopyable {
public:
	typedef std::function<void(void)> RunloopFunc;
	typedef std::shared_ptr<RunloopFunc> RunloopFuncSP;
	typedef std::shared_ptr<runLoop> RunloopSP;
	typedef unsigned int RunloopNum;
	typedef std::shared_ptr<std::vector<RunloopFuncSP>> WorkVectorSP;


	// getting a runloop
	static RunloopSP& currentRunloop(RunloopNum rlNum) {
		if (likely(0 == currentThread::runloopNum)) {
			currentThread::runloopNum = rlNum;
			std::lock_guard<std::mutex> l(mutexRunloopMap_);
			if (runloopMap_.find(rlNum) == runloopMap_.end())
				runloopMap_[rlNum] = std::make_shared<runLoop>();
			return runloopMap_.at(rlNum);
		} else {
			throw std::runtime_error("currentRunloop is setup already");
		}

	}

	static RunloopSP& currentRunloop() {
		if (unlikely(0 == currentThread::runloopNum))
			throw std::runtime_error("currentRunloop is null");
		std::lock_guard<std::mutex> l(mutexRunloopMap_);
		return runloopMap_.at(currentThread::runloopNum);
	}

	void run();
	// managing Sources

	void addFunctionAndWakeup(RunloopFunc && func);
	static void addFunctionAndWakeup(RunloopFunc &&func, RunloopNum rlNum);

	runLoop() :workVectorSP_(new std::vector<RunloopFuncSP>()){
	}

private:
	/*
	static RunloopSP getRunloop(RunloopNum rlNum) {
		std::lock_guard<std::mutex> l(mutexRunloopMap_);
		return runloopMap_.at(rlNum);
	}
	*/
// must lock mutexWorkVectorSP_
	void notifyWorkNolock();
	void notifyWorklock();

	WorkVectorSP getVectorSP() {
		std::lock_guard<std::mutex> l(mutexWorkVectorSP_);
		return workVectorSP_;
	}

private:

	bool isNeedRun_ = true;
	std::mutex mutexRequestStop_;

	WorkVectorSP workVectorSP_;
	std::mutex mutexWorkVectorSP_;
	std::condition_variable workVectorCondVar_;

	static std::map<RunloopNum, RunloopSP> runloopMap_;
	static std::mutex mutexRunloopMap_;

};

std::map<runLoop::RunloopNum, runLoop::RunloopSP> runLoop::runloopMap_;
std::mutex runLoop::mutexRunloopMap_;

void runLoop::run() {
	while (isNeedRun_) {
		auto workVectorSP = getVectorSP();
//		printf("before vector size %lu\n",workVectorSP->size());
		if (NULL != workVectorSP && !workVectorSP->empty()) {
//			printf("vector size %lu\n",workVectorSP->size());
			for (const auto& funcP: *workVectorSP) {
				(*funcP)();
			}
			workVectorSP->clear();
		}else{
			std::unique_lock<std::mutex> ul(mutexWorkVectorSP_);
//			printf("wait\n");
			workVectorCondVar_.wait(ul, [&]{
//					printf("wake\n %lu", workVectorSP->size());
					return !this->workVectorSP_->empty();
					});
		}
	}
}

void runLoop::addFunctionAndWakeup(RunloopFunc && func) {
	// get runloop workvector add function
	//std::shared_ptr<runLoop> runloopSP(this);
	std::lock_guard<std::mutex> l(this->mutexWorkVectorSP_);
	if (!this->workVectorSP_.unique()) {
		this->workVectorSP_.reset(new std::vector<RunloopFuncSP>());
	}
	this->workVectorSP_->push_back(RunloopFuncSP(new RunloopFunc(func)));

	//printf("add vector size %lu\n",this->workVectorSP_->size());
	// notify thread work
	notifyWorkNolock();
}

void runLoop::notifyWorkNolock() {
	// notify thread work
	this->workVectorCondVar_.notify_one();
}

void runLoop::notifyWorklock() {
	std::lock_guard<std::mutex> l(this->mutexWorkVectorSP_);
	this->workVectorCondVar_.notify_one();
}

void runLoop::addFunctionAndWakeup(RunloopFunc && func, RunloopNum rlNum) {
	// get runloop
	std::unique_lock<std::mutex> ul(mutexRunloopMap_);
	if (runloopMap_.find(rlNum) == runloopMap_.end())
		runloopMap_[rlNum] = std::make_shared<runLoop>();
	auto& runloopSP = runloopMap_.at(rlNum);
	ul.unlock();
	// get runloop workvector add function
	runloopSP->addFunctionAndWakeup(std::forward<RunloopFunc>(func));
}


}

#endif
