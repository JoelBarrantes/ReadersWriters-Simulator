gcc initializer.c -o initializer -lpthread -lrt -l c
gcc writers.c -o writers -lpthread -lrt -l c
gcc readers.c -o readers -lpthread -lrt -l c
gcc spy.c -o spy -lpthread -lrt -l c
gcc finalizer.c -o finalizer -lpthread -lrt -l c

