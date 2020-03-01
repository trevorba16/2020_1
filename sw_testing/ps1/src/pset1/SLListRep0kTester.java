package pset1;

import static org.junit.Assert.*;
import org.junit.Test;
import pset1.SLList.Node;

public class SLListRep0kTester {

    @Test public void t0()
    {
        SLList l = new SLList();
        assertTrue(l.repOk());
    }
    @Test public void t1()
    {
        SLList l = new SLList();
        l.header = new Node();
        Node n = new Node();
        Node n0 = new Node();
        Node n1 = new Node();
        Node n2 = new Node();

        n1.next = n2;
        n0.next = n1;
        n.next = n0;
        l.header.next = n;

        assertTrue(l.repOk());

    }

    @Test public void t2()
    {
        SLList l = new SLList();
        l.header = new Node();
        Node n = new Node();
        Node n0 = new Node();
        Node n1 = new Node();
        Node n2 = new Node();

        n1.next = n2;
        n0.next = n1;
        n.next = n0;
        l.header.next = n;

        n2.next = n0;

        assertFalse(l.repOk());

    }
}
