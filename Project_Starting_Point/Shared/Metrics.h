/*
* This file is used for code metric subroutines to help calculate various metrics of source code
* v1.0 
*	- Timer : Use start to begin timing a function and getTime to get time elapsed in ms since the start function was called
*	- Calculation : used to get averages or other caluclations based on passed in data values
*/
#pragma once
using namespace std;
#include <chrono>
using namespace std::chrono;
#include <vector>

/// <summary>
/// Namespace is used for ease of use and quality of life calculations of code metrics
/// </summary>
namespace Metrics {
	/// <summary>
	/// used to simplify the calculation of code timings using the std::chrono library
	/// </summary>
	class Timer {
		steady_clock::time_point startTime;
	public:
		/// <summary>
		/// starts a timer
		/// </summary>
		bool start() {
			try {
				startTime = high_resolution_clock::now();
				return true;
			}
			catch (exception e) {
				return false;
			}
		}
		/// <summary>
		/// returns the time since the timer was started in milliseconds
		/// </summary>
		/// <returns></returns>
		float getTime() {
			steady_clock::time_point endTime = high_resolution_clock::now();

			return float((endTime - startTime).count())/1000000;

		}
	};
	/// <summary>
	/// helps to calculate averages of metrics quickly and easily
	/// </summary>
	class Calculations {
		float sum = 0;
		int count = 0;
	public:
		/// <summary>
		/// add a datapoint such as time at a given index t(n) add times such as 0.00734ms 
		/// </summary>
		/// <param name="point"></param>
		void addPoint(float point) {
			sum += point;
			count++;
		}
		/// <summary>
		/// get average for the points added such as times are 1s 3s, 7s therefor total averafe is 11/3
		/// </summary>
		/// <returns></returns>
		float getAverage() {
			return sum / count;
		}
		float getSum() {
			return sum;
		}
	};
}