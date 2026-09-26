/* One line, compiled differently by each unit: IN_RANGE is defined by the
   unit that includes this header, so in_range() has different branches in
   narrow.c and wide.c. */
static inline int in_range(int value) { return IN_RANGE(value) ? 1 : 0; }
