package edu.calpoly.csc349.Schedule;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;

/**
 * This class provides a greedy algorithm implementation to the 
 * Scheduler interface.
 * @author asbeug@calpoly.edu
 */
public class GreedyScheduler implements Scheduler {
	@Override
	/**
	 * Create a schedule using a greedy algorithm.
	 * @param toCover - the time range to cover with employees
	 * @return employees - the employees availabilities
	 */
	public TimeRange[] makeSchedule(TimeRange toCover, TimeRange[] employees) {
		if (toCover == null) {
			throw new IllegalArgumentException(
				"The argument 'toCover' cannot be null.");
		} else if (employees == null) {
			throw new IllegalArgumentException(
				"The argument 'employees' cannot be null.");
		}

		if (employees.length == 0) {
			return null;			
		}

		ArrayList<TimeRange> scheduleList = new ArrayList<TimeRange>();
		Arrays.sort(employees, new StartSpanComparator());

		int currentEndTime = toCover.getStart();

		// Evaluate each employee's schedule
		for (TimeRange currentEmployee: employees) {
			// Check to see if we're already done
			if (scheduleList.size() > 0 
					&& scheduleList.get(0).getStart() <= toCover.getStart() 
					&& scheduleList.get(scheduleList.size()-1).getEnd() >= toCover.getEnd()) {
				break;
			}
			
			if (currentEmployee.getStart() <= currentEndTime) {
				if (currentEmployee.getStart() <= toCover.getStart()  
						&& scheduleList.size() > 0) {
					if (currentEmployee.getEnd() >= scheduleList.get(0).getEnd()) {
						scheduleList.set(0, currentEmployee);
						currentEndTime = currentEmployee.getEnd();
					}
				} else if (currentEmployee.getEnd() > currentEndTime) {
					scheduleList.add(currentEmployee);
					currentEndTime = currentEmployee.getEnd();
				}
			}
		}

		if (scheduleList.size() > 0 
				&& scheduleList.get(scheduleList.size()-1).getEnd() 
				>= toCover.getEnd()) {
			return scheduleList.toArray(new TimeRange[] {});
		} else {
			return null;
		}
	}

	/**
	 * Sort first by start time (ascending) then by span (descending).
	 */
	public class StartSpanComparator implements Comparator<TimeRange> {
		@Override
		public int compare(TimeRange o1, TimeRange o2) {
			if (o1.getStart() == o2.getStart()) {
				int span1 = o1.getEnd() - o1.getStart();
				int span2 = o2.getEnd() - o2.getStart();
				return Integer.valueOf(span2).compareTo(Integer.valueOf(span1));
			}

			return Integer.valueOf(o1.getStart()).compareTo(
					Integer.valueOf(o2.getStart()));
		}
	}
}