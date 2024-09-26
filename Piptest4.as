    lw     0  2  break
    lw     0  2  11
    lw     2  3  six
    add    2  3  2
    add    3  2  3
    beq    2  3  3
    add    3  2  2
    noop
    noop
    halt
break    .fill   5
six      .fill   6