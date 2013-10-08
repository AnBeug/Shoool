package edu.calpoly.csc349.Schedule;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;

/**
 * This class provides a greedy algorithm implementation to the Scheduler interface.
 * @author asbeug@calpoly.edu
 */
public class GreedyScheduler implements Scheduler {

	private static MyLogger myLogger = new MyLogger(MyLogger.INFO);

	@Override
	public TimeRange[] makeSchedule(TimeRange toCover, TimeRange[] employees) {
		if (toCover == null) {
			throw new IllegalArgumentException("The argument 'toCover' cannot be null.");
		} else if (employees == null) {
			throw new IllegalArgumentException("The argument 'employees' cannot be null.");
		}

		myLogger.println("GreedyScheduler.makeSchedule():", MyLogger.DEBUG);
		myLogger.println("\t toCover = " + toCover.toString(), MyLogger.DEBUG);

		if (employees.length == 0) {
			return null;			
		}

		ArrayList<TimeRange> scheduleList = new ArrayList<TimeRange>();

		Arrays.sort(employees, new StartSpanComparator());

		myLogger.println("\t employees (sorted) = ", MyLogger.DEBUG);
		for (TimeRange empTimeRange: employees) {
			myLogger.println("\t\t" + empTimeRange.toString(), MyLogger.DEBUG);
		}

		int currentEndTime = toCover.getStart();

		// Evaluate each employee's schedule
		for (TimeRange currentEmployee: employees) {
			myLogger.println("\tEvaluating " + currentEmployee + "; currentEndTime = " + currentEndTime, MyLogger.DEBUG);

			if (currentEmployee.getStart() <= currentEndTime && currentEmployee.getEnd() > currentEndTime) {
				currentEndTime = currentEmployee.getEnd();
				myLogger.println("\t\t* Using " + currentEmployee, MyLogger.DEBUG);
				scheduleList.add(currentEmployee);
			}
		}


		myLogger.println("\t schedule = ", MyLogger.DEBUG);
		if (scheduleList.size() > 0) {
			for (TimeRange schedule: scheduleList) {
				myLogger.println(schedule.toString(), MyLogger.DEBUG);
			}
		} else {
			myLogger.println("No solution found", MyLogger.DEBUG);
		}

		return scheduleList.toArray(new TimeRange[] {});
	}

	/**
	 * Sort first by start time (ascending) then by span (descending).
	 */
	public class StartSpanComparator implements Comparator<TimeRange> {
		@Override
		public int compare(TimeRange o1, TimeRange o2) {
			myLogger.println("** Comparing " + o1 + " to " + o2, MyLogger.VERBOSE);

			if (o1.getStart() == o2.getStart()) {
				int span1 = o1.getEnd() - o1.getStart();
				int span2 = o2.getEnd() - o2.getStart();
				myLogger.println("span 1 = " + span1 + "; span 2 = " + span2, MyLogger.VERBOSE);

				return Integer.valueOf(span2).compareTo(Integer.valueOf(span1));
			}

			return Integer.valueOf(o1.getStart()).compareTo(Integer.valueOf(o2.getStart()));
		}
	}
}