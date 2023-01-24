#pragma once
using namespace std;
#include <chrono>
using namespace std::chrono;
#include <vector>
namespace Metrics {
	class Timer {

		steady_clock::time_point startTime;
	public:
		void start() {
			startTime = high_resolution_clock::now();
		}
		float getTime() {
			steady_clock::time_point endTime = high_resolution_clock::now();

			return float((endTime - startTime).count());

		}
	};

	class Calculations {
		float sum = 0;
		int count = 0;
	public:
		void addPoint(float point) {
			sum += point;
			count++;
		}
		float getAverage() {
			return sum / count;
		}
	};
}