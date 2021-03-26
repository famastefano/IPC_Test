#include <boost/interprocess/windows_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <cstring>
#include <cstdlib>
#include <thread>

struct IPC_Sync
{
    boost::interprocess::interprocess_condition cnd;
    boost::interprocess::interprocess_mutex mux;
};

int main(int argc, char* argv[])
{
    using namespace boost::interprocess;

    windows_shared_memory shm(create_only, "MySharedMemory", read_write, 1000);
    mapped_region region(shm, read_write);

    IPC_Sync* sync = static_cast<IPC_Sync*>(new(region.get_address()) IPC_Sync);
    
    if(!sync)
    {
        printf("Error, couldn't create IPC_Sync!\n");
        return -1;
    }
     
    {
        printf("Waiting for client...\n");
        scoped_lock lock{ sync->mux };
        sync->cnd.wait(lock);
    }

    printf("Client said: %s\n", static_cast<char const*>(region.get_address()) + sizeof(IPC_Sync));

    return 0;
}