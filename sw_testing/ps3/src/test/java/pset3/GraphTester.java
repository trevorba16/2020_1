package pset3;

import static org.junit.Assert.assertEquals;

import java.util.TreeSet;
import java.util.Set;
import org.junit.Test;
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

//    @Test
//    public void test1() {
//        Graph g = new Graph(3);
//        g.addEdge(1, 2);
//        assertEquals(g.toString(), "numNodes: 3\nedges: [[false, false, false], [false, false, true], [false, false, false]]");
//    }
}