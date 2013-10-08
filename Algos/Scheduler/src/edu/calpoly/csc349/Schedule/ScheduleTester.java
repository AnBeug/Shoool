package edu.calpoly.csc349.Schedule;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.text.ParseException;
import java.util.ArrayList;

import edu.calpoly.csc349.Schedule.Scheduler.TimeRange;

/**
 * This class provides the test harness for the GreedyScheduler implementation.
 * @author asbeug
 *
 */
public class ScheduleTester {

	private GreedyScheduler greedyScheduler;
	private static MyLogger myLogger = new MyLogger(MyLogger.INFO);

	public static void main(String [ ] args)
	{
		myLogger.println("ScheduleTester.main(): start", MyLogger.VERBOSE);

		// Run tests
		ScheduleTester myTester = new ScheduleTester();
		myTester.runTests();

		myLogger.println("ScheduleTester.main(): finish", MyLogger.VERBOSE);
	}

	/*
	 * 
	 */
	private void runTests() {
		myLogger.println("ScheduleTester.performTest()", MyLogger.DEBUG);

		// Setup test cases
		ArrayList<TestCase> testCases = this.inputTestCases();

		if (testCases == null) {
			System.err.println("ScheduleTester.performTest(): unable to perform tests");
		}

		greedyScheduler = new GreedyScheduler();

		for (TestCase testCase : testCases) {
			this.runTest(testCase);
		}
	}

	/*
	 * 
	 */
	private ArrayList<TestCase> inputTestCases() {
		ArrayList<TestCase> testCases = new ArrayList<TestCase>();

		String filePath = "Resources/testCases.txt";
		String line;
		// Read in from file
		try {
			BufferedReader reader = new BufferedReader(new FileReader(filePath));
			int numTestCases = 0;

			while ((line = reader.readLine()) != null) {
				myLogger.println("ScheduleTester.inputTestCases(): " + line, MyLogger.VERBOSE);

				if (!line.startsWith("//")) {
					// split around a ;
					String[] parts = line.split(";");

					String toCoverString = parts[0];
					String[] toCoverParts = toCoverString.split("-");
					if (toCoverParts.length != 2) {
						myLogger.println("Could not parse line (malformed time range): " + line, MyLogger.ERR);
						continue;
					}

					TimeRange toCover;
					try {
						toCover = new TimeRange(Integer.parseInt(toCoverParts[0].trim()), Integer.parseInt(toCoverParts[1].trim()));
					} catch (NumberFormatException nfe) {
						myLogger.println("Could not parse line (malformed time range-can't parse number): " + line, MyLogger.ERR);
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
					
					testCases.add(new TestCase(numTestCases, toCover, employeeTimeRanges.toArray(new TimeRange[]{})
							, solutionTimeRanges.toArray(new TimeRange[]{})));
					numTestCases++;

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

		myLogger.println("ScheduleTester.inputTestCases(): found " 
				+ testCases.size() + " test cases.", MyLogger.VERBOSE);

		return testCases;
	}

	/***
	 * 
	 */
	private TimeRange[] runTest(TestCase testCase) {

		TimeRange[] scheduleSolution = greedyScheduler.makeSchedule(testCase.getToCover(), testCase.getEmployees());

		myLogger.println("---------------------", MyLogger.INFO);
		myLogger.println(testCase.toString(), MyLogger.INFO);
		if (scheduleSolution != null && scheduleSolution.length > 0) {
			myLogger.println("Solution: " + MyLogger.timeRangeArrayToString(scheduleSolution), MyLogger.INFO);
		} else {
			myLogger.println("Solution: none", MyLogger.INFO);
		}

		return null;
	}

	/**
	 * Inner class for information about individual test cases.
	 */
	private class TestCase {
		int id;
		private TimeRange toCover;
		private TimeRange[] employees;
		private TimeRange[] solution;

		private TestCase(int id, TimeRange toCover, TimeRange[] employees) {
			this.setId(id);
			this.setToCover(toCover);
			this.setEmployees(employees);
		}

		private TestCase(int id, TimeRange toCover, TimeRange[] employees, TimeRange[] solution) {
			this(id, toCover, employees);
			this.setSolution(solution);
		}
		
		@Override
		public String toString() {
			StringBuilder sb = new StringBuilder("Test Case # " + getId() + "\n");

			sb.append("\ttoCover: " + toCover.getStart() + "-" + toCover.getEnd() + "\n");
			
			sb.append("\temployees: ");
			if (employees != null && employees.length > 0) {
				sb.append(MyLogger.timeRangeArrayToString(getEmployees()));
			} else {
				sb.append("none");
			}
			sb.append("\n");
			
			sb.append("\texpected solution: ");
			if (solution != null && solution.length > 0) {
				sb.append(MyLogger.timeRangeArrayToString(getSolution()));
			} else {
				sb.append("none");
			}
			
			
			return sb.toString();
		}

		//------- GETTERS & SETTERS -------//
		private int getId() {
			return id;
		}
		private void setId(int id){
			this.id = id;
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
		private TimeRange[] getSolution() {
			return solution;
		}
		private void setSolution(TimeRange[] solution) {
			this.solution = solution;
		}
	}
}


