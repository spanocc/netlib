#ifndef NETLIB_NET_EVENT_LOOP_H
#define NETLIB_NET_EVENT_LOOP_H

#include <atomic>
#include <vector>
#include <memory>
#include "base/thread.h"
#include "base/current_thread.h"
#include "net/timer_container.h"

class Epoller;
class Channel;

// 栈对象
class EventLoop : private NonCopyable {
 public:
  EventLoop();
  ~EventLoop();
  void Loop();
  void Quit();
  void Wakeup();
  void RunInLoop(const std::function<void ()>& cb); 
  // 如果在不同线程，使用队列可以方便线程间调用函数
  // 如果在同一线程，可以将无法在channel的HandleEvent过程中调用的函数放在queue中延迟调用
  void QueueInLoop(const std::function<void ()>& cb); 
  void HandleWakeup();
  int CreateWakeupFd();

  TimerWeakPtr RunAt(Timestamp time, const TimerCallBack& cb) {
    return timer_container_->AddTimer(cb, time, 0.0);
  }
  TimerWeakPtr RunAfter(double delay, const TimerCallBack& cb) { // 以秒为单位
    Timestamp time(Timestamp::Now() + Timestamp(static_cast<int64_t>(delay * 1000000)));
    return RunAt(time, cb);
  }
  TimerWeakPtr RunEvery(double interval, const TimerCallBack& cb) {
    Timestamp time(Timestamp::Now() + Timestamp(static_cast<int64_t>(interval * 1000000)));
    return timer_container_->AddTimer(cb, time, interval);
  }
  void RemoveTimer(TimerWeakPtr timer) {
    timer_container_->RemoveTimer(timer);
  }

  bool IsInLoopThread() {
    return thread_id_ == CurrentThread::gettid();
  }
  void UpdateChannel(Channel *channel);
  void RemoveChannel(Channel *channel);
 private:
  typedef std::vector<Channel *> ChannelList;
  
  void RunPendingFunctors();
  
  std::atomic<bool> looping_;  
  const pid_t thread_id_; // thread_id_一经初始化就不可以改变，所以可以不上锁
  std::unique_ptr<Epoller> epoller_;
  ChannelList active_channels_;
  std::unique_ptr<TimerContainer> timer_container_;

  bool calling_pending_functors_; // 仅在本线程循环中使用
  int wakeup_fd_;
  std::unique_ptr<Channel> wakeup_channel_;
  Mutex mutex_;
  std::vector<std::function<void()>> pending_functors_; // GUARDED_BY(mutex_); 
};

#endif