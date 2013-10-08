package edu.calpoly.csc349.Schedule;

import edu.calpoly.csc349.Schedule.Scheduler.TimeRange;

public class MyLogger {
	public static int VERBOSE = 6;
	public static int DEBUG = 5;
	public static int INFO = 4;
	public static int WARN = 3;
	public static int ERR = 1;
	public static int OFF = 0;
	
	int debugLevel = INFO;
	
	public MyLogger(int debugLevel) {
		this.debugLevel = debugLevel;
	}
	
	public void println(String line, int lineDebugLevel) {
		if (lineDebugLevel <= debugLevel) {
			System.out.println(line);
		}
	}

	public void setDebugLevel(int debugLevel) {
		this.debugLevel = debugLevel;
	}
	public int getDebugLevel() {
		return debugLevel;
	}
	
	public static String timeRangeArrayToString(TimeRange[] timeRanges) {
		StringBuilder sb = new StringBuilder();
		
		int i = 0;
		for (TimeRange timeRange: timeRanges) {
			if (i > 0) {
				sb.append(",");
			}
			sb.append(timeRange.getStart() + "-" + timeRange.getEnd());
			i++;
		}
		
		return sb.toString();
	}
}
