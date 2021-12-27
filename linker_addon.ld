SECTIONS
{
    .ipc_array : ALIGN(8)
    {
        PROVIDE (__ipc_array_start = .);
        KEEP (*(.ipc_array))
        PROVIDE (__ipc_array_end = .);
    }
}
INSERT AFTER .rodata;