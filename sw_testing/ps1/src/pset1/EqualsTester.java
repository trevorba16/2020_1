package pset1;

import static org.junit.Assert.*;
import org.junit.Test;

public class EqualsTester
{ /* * P1: For any non-null reference value x, x.equals(null) should return false. */

    @Test public void t0()
    {
        assertFalse(new Object().equals(null));
    }
    // your test methods for P1 go here
    @Test public void t1()
    {
        assertFalse(new C(0).equals(null));
    }
    @Test public void t2()
    {
        assertFalse(new D(0,0).equals(null));
    }

/* P2: It is reflexive: for any non-null reference value x, x.equals(x)
 * should return true.
 */

    @Test public void p2_t0()
    {
        Object o = new Object();
        assertTrue(o.equals(o));
    }

    @Test public void p2_t1()
    {
        C c = new C(0);
        assertTrue(c.equals(c));
    }

    @Test public void p2_t2()
    {
        D d = new D(0,0);
        assertTrue(d.equals(d));
    }

/*
 * P3: It is symmetric: for any non-null reference values x and y, x.equals(y)
 * should return true if and only if y.equals(x) returns true.
 */

    @Test public void p3_t0()
    {
        Object o = new Object();
        Object o1 = new Object();

        assertTrue(o.equals(o1) == o1.equals(o));
    }
    @Test public void p3_t1()
    {
        Object o = new Object();
        C c1 = new C(0);

        assertTrue(o.equals(c1) == c1.equals(o));
    }
    // your test methods for P1 go here
    @Test public void p3_t2()
    {
        Object o = new Object();
        D d1 = new D(0,0);

        assertTrue(o.equals(d1) == d1.equals(o));
    }


    @Test public void p3_t3()
    {
        C c = new C(0);
        C c1 = new C(0);

        assertTrue(c.equals(c1) == c1.equals(c));
    }

    @Test public void p3_t4()
    {
        C c = new C(0);
        Object o1 = new Object();

        assertTrue(c.equals(o1) == o1.equals(c));
    }

    @Test public void p3_t5()
    {
        C c = new C(0);
        D d1 = new D(0,0);

        assertTrue(c.equals(d1) == d1.equals(c));
    }

    @Test public void p3_t6()
    {
        D d = new D(0,0);
        D d1 = new D(0,0);

        assertTrue(d.equals(d1) == d1.equals(d));
    }

    @Test public void p3_t7()
    {
        D d = new D(0,0);
        Object o1 = new Object();

        assertTrue(d.equals(o1) == o1.equals(d));
    }
    @Test public void p3_t8()
    {
        D d = new D(0, 0);
        C c1 = new C(0);

        assertTrue(d.equals(c1) == c1.equals(d));
    }



    /*  P4: It is transitive: for any non-null reference values x, y, and z,
*  if x.equals(y) returns true and y.equals(z) returns true, then
*  x.equals(z) should return true. */
// you do not need to write tests for P4
}