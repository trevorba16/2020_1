package pset3;

import java.util.Arrays;
import java.util.Set;
import java.util.TreeSet;
public class Graph {
    // Java program for implementation of QuickSort
        /* This function takes last element as pivot,
        places the pivot element at its correct
        position in sorted array, and places all
        smaller (smaller than pivot) to left of
        pivot and all greater elements to right
        of pivot */
        int partition(int arr[], int low, int high)
        {
            int x = arr[high];
            int i = (low-1); // index of smaller element
            System.out.println(i);
            for (int j=low; j<high; j++)
            {
                // If current element is smaller than or
                // equal to pivot
                if (arr[j] <= x)
                {
                    i++;

                    // swap arr[i] and arr[j]
                    System.out.println("--------------");
                    System.out.println(i);
                    System.out.println(j);
                    exchange(arr, i, j);
//                    int temp = arr[i];
//                    arr[i] = arr[j];
//                    arr[j] = temp;
                }
            }

            // swap arr[i+1] and arr[high] (or pivot)
            exchange(arr, i+1, high);
//            int temp = arr[i+1];
//            arr[i+1] = arr[high];
//            arr[high] = temp;

            return i+1;
        }


        /* The main function that implements QuickSort()
        arr[] --> Array to be sorted,
        low --> Starting index,
        high --> Ending index */
        void sort(int arr[], int low, int high)
        {
            if (low < high)
            {
			/* pi is partitioning index, arr[pi] is
			now at right place */
                int pi = partition(arr, low, high);

                // Recursively sort elements before
                // partition and after partition
                sort(arr, low, pi-1);
                sort(arr, pi+1, high);
            }
        }

        static void exchange (int arr[], int i, int j)
        {
            int t = arr[j];
            arr[i] = t;
            arr[j] = arr[i];
        }

        /* A utility function to print array of size n */
        static void printArray(int arr[])
        {
            int n = arr.length;
            for (int i=0; i<n; ++i)
                System.out.print(arr[i]+" ");
            System.out.println();
        }

        // Driver program
        public static void main(String args[])
        {
            int arr[] = {0,2,1};
            int n = arr.length;

            Graph ob = new Graph();
            ob.sort(arr, 0, n-1);

            System.out.println("sorted array");
            printArray(arr);
        }
    /*This code is contributed by Rajat Mishra */

}