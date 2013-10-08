package edu.calpoly.csc349.Schedule;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Random;

import edu.calpoly.csc349.Schedule.Scheduler.TimeRange;

/**
 * This class provides the test harness for the GreedyScheduler implementation.
 * @author asbeug
 */
public class ScheduleTester {

	private static final int NUM_TEST_CASES = 1000;
	private static final int MAX_TIME_RANGE = 20;
	private static final int MAX_NUM_EMPLOYEES = 100;

	private GreedyScheduler greedyScheduler;

	public static void main(String [ ] args)
	{
		ScheduleTester myTester = new ScheduleTester();
		myTester.runTests();
	}

	/**
	 * This method creates tests and runs each individually.
	 */
	private void runTests() {
		greedyScheduler = new GreedyScheduler();

		// Setup test cases
		ArrayList<TestCase> testCases = new ArrayList<TestCase>();
		testCases.addAll(inputTestCases("Resources/testCases.txt"));
		testCases.addAll(createTestCases());
		
//		for (int j=0; j < NUM_TEST_CASES; j++) {
//			testCases.add(createTestCaseProgram	atically());
//		}
		
		for (TestCase testCase : testCases) {
			TimeRange[] scheduleSolution = greedyScheduler.makeSchedule(testCase.getToCover(), 
						testCase.getEmployees());
			testCase.setFoundSolution(scheduleSolution);
			if (scheduleSolution != null ) {
				System.out.println(timeRangeArrayToString(scheduleSolution));
			} else {
				System.out.println("none");
			}
		}
		
	}
	
	/**
	 * Create a test case by randomly generating values.
	 * 
	 * @return TestCase
	 */
	TestCase createTestCaseProgramatically() {
		TestCase newCase = new TestCase();
		Random myRand = new Random();

		int tempIntA = myRand.nextInt(MAX_TIME_RANGE);
		int tempIntB = myRand.nextInt(MAX_TIME_RANGE);
		if (tempIntA == tempIntB) {
			tempIntB = myRand.nextInt(MAX_TIME_RANGE);
		}
		
		int start = Math.min(tempIntA, tempIntB);
		int end = Math.max(tempIntA, tempIntB);
		
		newCase.setToCover(new TimeRange(start, end));
		
		int numEmployees = myRand.nextInt(MAX_NUM_EMPLOYEES);
		ArrayList<TimeRange> employees = new ArrayList<TimeRange>();
		for (int i = 0; i < numEmployees; i++) {
			tempIntA = myRand.nextInt(MAX_TIME_RANGE);
			tempIntB = myRand.nextInt(MAX_TIME_RANGE);
			if (tempIntA == tempIntB) {
				tempIntB = myRand.nextInt(MAX_TIME_RANGE);
			}
			
			start = Math.min(tempIntA, tempIntB);
			end = Math.max(tempIntA, tempIntB);

			employees.add(new TimeRange(start,end));
		}
		newCase.setEmployees(employees.toArray(new TimeRange[] {}));
					
		return newCase;
	}
	
	/**
	 * Create test case objects by hand.
	 * @return ArrayList<TestCase> hard-coded test cases
	 */
	private ArrayList<TestCase> createTestCases() {
		ArrayList<TestCase> testCases = new ArrayList<TestCase>();
		
		
		// 1-8;1-4,2-5,5-7,5-9;1-4,2-5,5-9
		testCases.add(new TestCase(new TimeRange(1,8),
				new TimeRange[] {new TimeRange(1,4), new TimeRange(2,5),
					new TimeRange(5,7), new TimeRange(5,9),},
				new TimeRange[] {new TimeRange(1,4), new TimeRange(2,5),
					new TimeRange(5,9)}));
		// 1-3;;
		testCases.add(new TestCase(new TimeRange(1,3),
				new TimeRange[] {}, new TimeRange[] {}));
		// 3-4;1-2,6-10;
		testCases.add(new TestCase(new TimeRange(1,10),
				new TimeRange[] {new TimeRange(1,10)},
				new TimeRange[] {new TimeRange(1,10)}));
		// 3-4;6-7,7-8;
		testCases.add(new TestCase(new TimeRange(3,4),
				new TimeRange[] {new TimeRange(6,8), new TimeRange(7,8)},
				new TimeRange[] {}));
		// 6-9;1-2,2-4,3-6;
		testCases.add(new TestCase(new TimeRange(6,9),
				new TimeRange[] {new TimeRange(1,2), new TimeRange(2,4),
					new TimeRange(3,6)},
				new TimeRange[] {}));
		// 1-2;1-2;1-2
		testCases.add(new TestCase(new TimeRange(6,9),
				new TimeRange[] {new TimeRange(6,9)},
				new TimeRange[] {new TimeRange(6,9)}));
		testCases.add(new TestCase(new TimeRange(6,9),
				new TimeRange[] {new TimeRange(6,9), new TimeRange(6,9), new TimeRange(6,9)},
				new TimeRange[] {new TimeRange(6,9)}));

		return testCases;
	}
	
	
	/**
	 * Parses test cases from text file.
	 * @return ArrayList<TestCase> test cases from file
	 */
	private ArrayList<TestCase> inputTestCases(String filePath) {
		ArrayList<TestCase> testCases = new ArrayList<TestCase>();

		String line;
		// Read in from file
		try {
			BufferedReader reader = new BufferedReader(new FileReader(filePath));
			while ((line = reader.readLine()) != null) {
				if (!line.startsWith("//")) {
					// split around a ;
					String[] parts = line.split(";");

					String toCoverString = parts[0];
					String[] toCoverParts = toCoverString.split("-");
					if (toCoverParts.length != 2) {
						//printDebug("Could not parse line (malformed time range): " + line);
						continue;
					}

					TimeRange toCover;
					try {
						toCover = new TimeRange(Integer.parseInt(toCoverParts[0].trim()), Integer.parseInt(toCoverParts[1].trim()));
					} catch (NumberFormatException nfe) {
						//printDebug("Could not parse line (malformed time range-can't parse number): " + line);
						continue;
					}

					ArrayList<TimeRange> employeeTimeRanges = new ArrayList<TimeRange>();
					if (parts.length >= 2 && parts[1] != null && !parts[1].trim().isEmpty()) {
						String employeesString = parts[1];
						String[] employeeParts = employeesString.split(",");
						for (String employeePart: employeeParts) {
							String[] employeeTimeRangePart = employeePart.split("-");
							if (employeeTimeRangePart.length == 2) {
								employeeTimeRanges.add(new TimeRange(Integer.parseInt(employeeTimeRangePart[0]), 
										Integer.parseInt(employeeTimeRangePart[1])));
							}
						}
					}

					ArrayList<TimeRange> solutionTimeRanges = new ArrayList<TimeRange>();
					if (parts.length >= 3 && parts[2] != null && !parts[2].trim().isEmpty()) {
						String solutionString = parts[2];
						String[] solutionParts = solutionString.split(",");
						for (String solutionPart: solutionParts) {
							String[] solutionTimeRangePart = solutionPart.split("-");
							if (solutionTimeRangePart.length == 2) {
								solutionTimeRanges.add(new TimeRange(Integer.parseInt(solutionTimeRangePart[0]), 
										Integer.parseInt(solutionTimeRangePart[1])));
							}
						}
					}
					
					testCases.add(new TestCase(toCover, employeeTimeRanges.toArray(new TimeRange[]{})
							, solutionTimeRanges.toArray(new TimeRange[]{})));

				} // else it's a comment line in the file

			}

		} catch (FileNotFoundException fnfe) {
			System.err.println("inputTestCases(): Unable to find test cases file: " 
					+ filePath + ".");
			return null;
		} catch (IOException ioe) {
			System.err.println("ScheduleTester.inputTestCases(): Unable to read from test cases file: " 
					+ filePath + ".");
			return null;
		}

		return testCases;
	}
	
	/**
	 * Make a string out of an array of time ranges, with commans in
	 * between. For use in debugging printing.
	 * @param timeRanges
	 * @return String
	 */
	private String timeRangeArrayToString(TimeRange[] timeRanges) {
		StringBuilder sb = new StringBuilder();
		
		int i = 0;
		for (TimeRange timeRange: timeRanges) {
			if (i > 0) {
				sb.append(",");
			}
			sb.append(timeRange.getStart() + "-" 
				+ timeRange.getEnd());
			i++;
		}
		
		return sb.toString();
	}
	
	/**
	 * Inner class for information about individual test cases.
	 */
	private class TestCase {
		private TimeRange toCover;
		private TimeRange[] employees;
		private TimeRange[] expectedSolution;
		private TimeRange[] foundSolution;

		private TestCase() {
		}
		
		private TestCase(TimeRange toCover, TimeRange[] employees) {
			this.setToCover(toCover);
			this.setEmployees(employees);
		}

		private TestCase(TimeRange toCover, TimeRange[] employees, TimeRange[] solution) {
			this(toCover, employees);
			this.setExpectedSolution(solution);
		}
		
		@Override
		public String toString() {
			StringBuilder sb = new StringBuilder();
			
			sb.append(toCover.getStart() + "-" + toCover.getEnd() + ";");
			
			if (employees != null && employees.length > 0) {
				sb.append(timeRangeArrayToString(getEmployees()));
			}
			sb.append(";");
			
			if (expectedSolution != null && expectedSolution.length > 0) {
				sb.append(timeRangeArrayToString(getExpectedSolution()));
			}
						
			return sb.toString();
		}

		private TimeRange getToCover() {
			return toCover;
		}
		private void setToCover(TimeRange toCover) {
			this.toCover = toCover;
		}
		private TimeRange[] getEmployees() {
			return employees;
		}
		private void setEmployees(TimeRange[] employees) {
			this.employees = employees;
		}
		private TimeRange[] getExpectedSolution() {
			return expectedSolution;
		}
		private void setExpectedSolution(TimeRange[] solution) {
			this.expectedSolution = solution;
		}
		private TimeRange[] getFoundSolution() {
			return foundSolution;
		}
		private void setFoundSolution(TimeRange[] foundSolution) {
			this.foundSolution = foundSolution;
		}
	}

}


