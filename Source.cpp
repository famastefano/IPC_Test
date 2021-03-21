#define BOOST_DATE_TIME_NO_LIB

#include <boost/interprocess/managed_shared_memory.hpp>
#include <cstddef>
#include <utility>
#include <thread>
#include <cstdio>

int main(int argc, char** argv) 
{
    using namespace boost::interprocess;
    typedef int MyType;

    if (argc == 1)
    {  
        printf("Parent process spawned...");

        //Parent process
//Remove shared memory on construction and destruction
        struct shm_remove
        {
            shm_remove() { shared_memory_object::remove("MySharedMemory"); }
            ~shm_remove() { shared_memory_object::remove("MySharedMemory"); }
        } remover;

        //Construct managed shared memory
        managed_shared_memory segment(create_only, "MySharedMemory", 65536);

        //Create an object of MyType initialized to {42}
        MyType* instance = segment.construct<MyType>
            ("MyType instance")  //name of the object
            (42);            //ctor first argument

         //Create an array of 10 elements of MyType initialized to {99}
        MyType* array = segment.construct<MyType>
            ("MyType array")     //name of the object
            [10]                 //number of elements
        (99);            //Same two ctor arguments for all objects

     //Create an array of 3 elements of MyType initializing each one
     //to a different value {0}, {1}, {2}
        int   int_initializer[3] = { 0, 1, 2 };

        MyType* array_it = segment.construct_it<MyType>
            ("MyType array from it")   //name of the object
            [3]                        //number of elements
        (&int_initializer[0]);    //Iterator for the 1st ctor argument

         //Check child has destroyed all objects
        while (segment.find<MyType>("MyType array").first ||
            segment.find<MyType>("MyType instance").first ||
            segment.find<MyType>("MyType array from it").first)
        {
            printf("Some objects still alive...");

            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1s);
        }

        printf("Parent process exited.");
    }
    else
    {
        printf("Child process spawned...");

   //Open managed shared memory
        managed_shared_memory segment(open_only, "MySharedMemory");

        std::pair<MyType*, managed_shared_memory::size_type> res;

        //Find the array
        res = segment.find<MyType>("MyType array");
        //Length should be 10
        if (res.second != 10)
        {
            printf("MyType array Length is different than 10! -> %d", (int)res.second);
        }
        else
        {
            printf("\t[MyType array]");
            for (int i = 0; i < res.second; ++i)
                printf("[%d]: %d", i, res.first[i]);
        }

        //Find the object
        res = segment.find<MyType>("MyType instance");
        //Length should be 1
        if (res.second != 1)
        {
            printf("MyType instance Length is different than 1! -> %d", (int)res.second);
        }
        else
        {
            printf("\t[MyType instance]\n%d", *res.first);
        }

        //Find the array constructed from iterators
        res = segment.find<MyType>("MyType array from it");
        //Length should be 3
        if (res.second != 3)
        {
            printf("MyType array from it Length is different than 3! -> %d", (int)res.second);
        }
        else
        {
            printf("\t[MyType array]");
            for (int i = 0; i < res.second; ++i)
                printf("[%d]: %d", i, res.first[i]);
        }

        //We're done, delete all the objects
        segment.destroy<MyType>("MyType array");
        segment.destroy<MyType>("MyType instance");
        segment.destroy<MyType>("MyType array from it");

        printf("Parent objects destroyed.");
    }

    return 0;
}