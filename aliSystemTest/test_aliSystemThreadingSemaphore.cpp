#include "gtest/gtest.h"
#include <aliSystem.hpp>

namespace {
  using Semaphore = aliSystem::Threading::Semaphore;
  using Time      = aliSystem::Time;
  
}

TEST(aliSystemSemaphore, DoNothing) {
  Semaphore                 sem;
  Time::TP                  start;
  Time::TP                  end;
  std::chrono::milliseconds ms(1);
  std::chrono::seconds      sec(1);
  sem.Post();
  start = Time::Now();
  sem.TimedWait(start+sec);
  end   = Time::Now();
  EXPECT_NEAR(Time::ToSeconds(end-start),
	      0,
	      Time::ToSeconds(ms)) << "Confirming timed wait with cnt=1";
  start = Time::Now();
  sem.TimedWait(start + sec);
  end   = aliSystem::Time::Now();
  EXPECT_NEAR(Time::ToSeconds(end-start),
	      Time::ToSeconds(sec),
	      Time::ToSeconds(ms)) << "Confirming timed wait's expiration";
  sem.Post();
  start = end;
  sem.Wait();
  end   = Time::Now();
  EXPECT_NEAR(Time::ToSeconds(end-start),
	      0,
	      Time::ToSeconds(ms)) << "Confirming wait";
}

