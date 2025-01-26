#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <thread>

#include "dumper.hpp"
#include "spsc_queue.hpp"
#include "tools.hpp"
#include "types.hpp"

using namespace test;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::printf("Usage: spsc_test <size>\n");
        return 0;
    }
    uint32_t size = std::strtoul(argv[1], nullptr, 10);

    /* case 1: basic function test */
    lfcq::SpscQueue<TrivialObj> queue1(size);

    std::thread writer([&queue1, size]() {
        Dumper writer("spsc/basic_writer.bin");

        // push to the queue and dump by order
        auto uid = random<uint32_t>(0, UINT32_MAX);
        for (uint32_t i = 0; i < size; i++) {
            TrivialObj obj(uid, i);
            queue1.push(obj);
            writer.stream() << obj.uid << ", " << obj.seq << std::endl;
        }
    });

    std::thread reader([&queue1, size]() {
        Dumper reader("spsc/basic_reader.txt");

        // read from the queue and dump by order
        for (uint32_t i = 0; i < size;) {
            queue1.pop([&i, &reader](TrivialObj& obj) {
                i++;
                reader.stream() << obj.uid << ", " << obj.seq << std::endl;
            });
        }
    });

    writer.join();
    reader.join();
    return 0;
}