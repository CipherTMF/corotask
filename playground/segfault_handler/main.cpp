#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <cstddef>
#include <unistd.h>
#include <chrono>

/*
 * This test creates two regions in memory:
 * The "ORIGINAL" region where we can read/write and the
 * "COPY" region where any access (read or write) results in
 * a SIGBUS error. This demo program writes data to the "ORIGINAL"
 * region and tries to read from the "COPY" region. Upon reading
 * from the "COPY" region the SIGBUS error will be handled in a way
 * that all contents from the "ORIGINAL" region are mirrored to the
 * "COPY" region. This results in reading the originally written data
 * without the users active intervention.
 *
 * setup:
 *     ORIGINAL    : [0,0,0,0,0,0,0,0,0,0]
 *     COPY        : [0,0,0,0,0,0,0,0,0,0]
 * writing to "ORIGINAL":
 *     ORIGINAL    : [0,1,2,3,4,5,6,7,8,9]
 *     COPY        : [0,0,0,0,0,0,0,0,0,0]
 * accessing "COPY" results in error, memory is mirrored:
 *     ORIGINAL    : [0,1,2,3,4,5,6,7,8,9]
 *     COPY        : [0,1,2,3,4,5,6,7,8,9]
 * access continues without errors
 */

auto ORIGINAL = reinterpret_cast<int*>(0x10000000000);
auto COPY = reinterpret_cast<int*>(0x20000000000);

auto PAGE_SIZE = sysconf(_SC_PAGESIZE);

std::byte* page_start(void* addr) {
   uintptr_t page_base = (uintptr_t) addr & ~((uintptr_t) PAGE_SIZE - 1);
   return reinterpret_cast<std::byte*>(page_base);
}

std::byte* page_end(void* addr) {
   uintptr_t page_base = ((uintptr_t) addr + PAGE_SIZE - 1) & ~((uintptr_t) PAGE_SIZE - 1);
   return reinterpret_cast<std::byte*>(page_base);
}

// reserves a memory space where we are allowed to read and write
void reserve_local(void* ptr, size_t size) {
   const auto begin = page_start((std::byte*) ptr);
   const auto end = page_end((std::byte*) ptr + size);
   const auto sz = end - begin;
   if (mmap(begin, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED | MAP_NORESERVE, -1,
            0) == MAP_FAILED) {
      std::exit(24);
   }
}

// reserves a memory space where we are not allowed to read nor write
// this is to identify access to this region to be loaded from remote resources
void reserve_remote(void* ptr, size_t size) {
   const auto begin = page_start(ptr);
   const auto end = page_end((std::byte*) ptr + size);
   const auto sz = end - begin;
   if (mmap(begin, sz, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED | MAP_NORESERVE, -1,
            0) == MAP_FAILED) {
      std::exit(24);
   }
}

// releases the physical backing memory of the virtual addresses
void release(void* ptr, size_t size) {
   const auto begin = page_start(ptr);
   const auto end = page_end((std::byte*) ptr + size);
   const auto sz = end - begin;
   madvise(begin, sz, MADV_DONTNEED);
}

// releases the physical backing memory and resets access rights
// so that any access results in a SIGBUS to catch again later
void reset_remote(void* ptr, size_t size) {
   release(ptr, size);
   reserve_remote(ptr, size);
}

// detect read access from remote resources and loading them in time when needed
void pagefault_handler(int, siginfo_t* info, void*) {
   auto* fault_address = static_cast<std::byte*>(info->si_addr);

   // check if that's a remote memory region
   if (fault_address >= (std::byte*) COPY && fault_address < ((std::byte*) COPY) + 40) {
      // remapping remote-associated page to be able to read/write
      reserve_local(page_start(fault_address), PAGE_SIZE);

      // copy from remote resources
      const auto offset = fault_address - reinterpret_cast<std::byte*>(COPY);
      const auto original = reinterpret_cast<std::byte*>(ORIGINAL) + offset;
      memcpy(page_start(fault_address), page_start(original), PAGE_SIZE);

      std::cout << "LOADED ";
   } else {
      std::cerr << "outside region! ABORTING" << std::endl;
      std::exit(1);
   }
}

void setup() {
   // pagefault handler
   struct sigaction sa = {};
   sa.sa_flags = SA_ONSTACK | SA_SIGINFO | SA_NODEFER | SA_NOCLDWAIT;
   sa.sa_sigaction = pagefault_handler;
   sigemptyset(&sa.sa_mask);
   if (sigaction(SIGBUS, &sa, nullptr) == -1) {
      perror("sigaction");
      exit(EXIT_FAILURE);
   }
   if (sigaction(SIGSEGV, &sa, nullptr) == -1) {
      perror("sigaction");
      exit(EXIT_FAILURE);
   }

   reserve_local(ORIGINAL, 40);
   reserve_remote(COPY, 40);
}

int main() {
   setup();

   auto assert = [](const bool b) {
      if (!b) {
         std::exit(24);
      }
   };

   std::cout << "writing to local resource.. ";
   for (int i = 0; i < 10; i++) {
      ORIGINAL[i] = i;
   }
   std::cout << "done\n";

   std::cout << "reading from remote resource.. ";
   for (int i = 0; i < 10; i++) {
      assert(COPY[i] == i);
   }
   std::cout << "done\n";

   std::cout << "unloading remote resources.. ";
   reset_remote(COPY, 40);
   std::cout << "done\n";

   std::cout << "reading from remote resources.. ";
   for (int i = 0; i < 10; i++) {
      assert(COPY[i] == i);
   }
   std::cout << "done\n";
}
