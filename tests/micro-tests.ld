SECTIONS
{
  .micro_tests :
  {
    __micro_tests_start = .;
    KEEP(*(.micro_tests))
    __micro_tests_stop = .;
  }
}
INSERT AFTER .data;
