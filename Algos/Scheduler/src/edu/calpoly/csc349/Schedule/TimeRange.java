package edu.calpoly.csc349.Schedule;

/**
 * A time range represents either a range of times to cover or an
 * employee's range of available time.
 * @auther asbeug
 **/
public class TimeRange 
{
    private int start;
    private int end;

    public int getStart() 
    {
        return start;
    }
    
    public int getEnd() 
    {
        return end;
    }

    public TimeRange(int start, int end) 
    {
        this.start = start;
        this.end = end;
    }
        
    @Override
    public String toString() 
    {
        return new String("[TimeRange@" + System.identityHashCode(this) + "] " + getStart() + " -- " + getEnd());
    }
}
