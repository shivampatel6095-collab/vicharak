# vicharak

## IOCTL Commands
- SET_SIZE_OF_QUEUE → set queue capacity (int)
- PUSH_DATA → push char buffer + length (struct)
- POP_DATA → pop data, blocks if queue empty (struct, length used as max buffer)

## Build & Run
## Queue Logic
- FIFO
- Circular index wrap using % size
- Mutex for safety
- Wait queue for blocking POP
