package pset1;
import static org.junit.Assert.*;
import org.junit.Test;
public class HashCodeTester {
    /* * P5: If two objects are equal according to the equals(Object)
    * method, then calling the hashCode method on each of
    * the two objects must produce the same integer result. */

    @Test public void t0()
    {
        Object o = new Object();
        Object o1 = new Object();

        assertTrue(o.hashCode() == o1.hashCode());
    }
    @Test public void t1()
    {
        Object o = new Object();
        C c1 = new C(0);

        assertTrue(o.hashCode() == c1.hashCode());
    }

    @Test public void t2()
    {
        Object o = new Object();
        D d1 = new D(0,0);

        assertTrue(o.hashCode() == d1.hashCode());
    }


    @Test public void t3()
    {
        C c = new C(0);
        C c1 = new C(0);

        assertTrue(c.hashCode() == c1.hashCode());
    }

    @Test public void t4()
    {
        C c = new C(0);
        Object o1 = new Object();

        assertTrue(c.hashCode() == o1.hashCode());
    }

    @Test public void t5()
    {
        C c = new C(0);
        D d1 = new D(0,0);

        assertTrue(c.hashCode() == d1.hashCode());
    }

    @Test public void t6()
    {
        D d = new D(0,0);
        D d1 = new D(0,0);

        assertTrue(d.hashCode() == d1.hashCode());
    }

    @Test public void t7()
    {
        D d = new D(0,0);
        Object o1 = new Object();

        assertTrue(d.hashCode() == o1.hashCode());
    }
    @Test public void t8()
    {
        D d = new D(0, 0);
        C c1 = new C(0);

        assertTrue(d.hashCode() == c1.hashCode());
    }

}