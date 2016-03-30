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

	void addFunctionAndWakeup(RunloopFuncSP runloopFuncSP);
	void notifyWork();
	static void addFunctionAndWakeup(std::function<void(void)> Func, RunloopNum rlNum);

	runLoop() :workVectorSP_(new std::vector<RunloopFuncSP>()){
	}

private:
	/*
	static RunloopSP getRunloop(RunloopNum rlNum) {
		std::lock_guard<std::mutex> l(mutexRunloopMap_);
		return runloopMap_.at(rlNum);
	}
	*/

	WorkVectorSP getVectorSP() {
		std::lock_guard<std::mutex> l(mutexWorkVectorSP_);
		return workVectorSP_;
	}

private:

	bool isNeedRun = true;
	std::mutex mutexRequestStop_;

	WorkVectorSP workVectorSP_;
	bool isWait_ = true;
	std::mutex mutexWorkVectorSP_;
	std::condition_variable workVectorCondVar_;

	static std::map<RunloopNum, RunloopSP> runloopMap_;
	static std::mutex mutexRunloopMap_;

};

std::map<runLoop::RunloopNum, runLoop::RunloopSP> runLoop::runloopMap_;
std::mutex runLoop::mutexRunloopMap_;

void runLoop::run() {
	while (isNeedRun) {
		auto workVectorSP = getVectorSP();
		if (NULL == workVectorSP || workVectorSP->empty()) {
			std::unique_lock<std::mutex> ul(mutexWorkVectorSP_);
			isWait_ = true;
			workVectorCondVar_.wait(ul, [&]{
					return !isWait_;
					});
		}
		if (NULL != workVectorSP)
		for (const auto& funcP: *workVectorSP) {
			(*funcP)();
		}
		workVectorSP->clear();
	}
}

void runLoop::addFunctionAndWakeup(runLoop::RunloopFuncSP runloopFuncSP) {
	// get runloop workvector add function
	//std::shared_ptr<runLoop> runloopSP(this);
	std::lock_guard<std::mutex> l(this->mutexWorkVectorSP_);
	if (!this->workVectorSP_.unique()) {
		WorkVectorSP workVectorSP(new std::vector<RunloopFuncSP>());
		this->workVectorSP_.swap(workVectorSP);
	}
	this->workVectorSP_->push_back(std::move(runloopFuncSP));
	// notify thread work
	notifyWork();
}

void runLoop::notifyWork() {
	// notify thread work
	if (this->isWait_) {
		this->isWait_ = false;
		this->workVectorCondVar_.notify_one();
	}
}

void runLoop::addFunctionAndWakeup(std::function<void(void)> Func, RunloopNum rlNum) {
	// get runloop
	std::unique_lock<std::mutex> ul(mutexRunloopMap_);
	if (runloopMap_.find(rlNum) == runloopMap_.end())
		runloopMap_[rlNum] = std::make_shared<runLoop>();
	auto& runloopSP = runloopMap_.at(rlNum);
	ul.unlock();
	// get runloop workvector add function
	RunloopFuncSP runloopFuncSP(new RunloopFunc(Func));
	runloopSP->addFunctionAndWakeup(runloopFuncSP);
}


}

#endif
