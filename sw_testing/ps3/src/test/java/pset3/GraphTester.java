package pset3;

import java.util.TreeSet;
import java.util.Set;
import org.junit.Test;

import static org.junit.Assert.*;

public class GraphTester {
    // tests for method "addEdge" in class "Graph"
    @Test
    public void tae0() {
        Graph g = new Graph(2);
        g.addEdge(0, 1);
        System.out.println(g);
        assertEquals(g.toString(), "numNodes: 2\nedges: [[false, true], [false, false]]");
    }
    // your tests for method "addEdge" in class "Graph" go here
    // you must provide at least 4 test methods;
    // each test method has at least 1 invocation of addEdge;
    // each test method creates exactly 1 graph
    // each test method creates a unique graph w.r.t. "equals" method
    // each test method has at least 1 test assertion;
    // your test methods provide full statement coverage of your
    // implementation of addEdge and any helper methods
    // no test method directly invokes any method that is not
    // declared in the Graph class as given in this homework 2

    @Test
    public void test1() {
        Graph g = new Graph(3);
        g.addEdge(1, 2);
        assertEquals(g.toString(), "numNodes: 3\nedges: [[false, false, false], [false, false, true], [false, false, false]]");
    }

    @Test
    public void test2() {
        Graph g = new Graph(3);
        g.addEdge(1, 3);
        assertEquals(g.toString(), "numNodes: 3\nedges: [[false, false, false], [false, false, false], [false, false, false]]");
    }

    @Test
    public void test3() {
        Graph g = new Graph(4);
        g.addEdge(1, 0);
        g.addEdge(0, 1);
        g.addEdge(0, 2);
        g.addEdge(3, 2);

        Set<Integer> sources = new TreeSet<Integer>();
        sources.add(1);
        sources.add(3);

        Set<Integer> targets = new TreeSet<Integer>();
        targets.add(1);
        targets.add(2);

        assertTrue(g.reachable(sources, targets));
    }

    @Test
    public void test4() {
        Graph g = new Graph(4);
        g.addEdge(1, 0);
        g.addEdge(0, 1);
        g.addEdge(2, 1);
        g.addEdge(3, 2);

        Set<Integer> sources = new TreeSet<Integer>();
        sources.add(1);
        sources.add(3);

        Set<Integer> targets = new TreeSet<Integer>();
        targets.add(1);
        targets.add(3);

        assertFalse(g.reachable(sources, targets));
    }
}

interface J
{

}
interface I extends J
{

}
class D implements I
{

}
class C extends D implements J
{

}