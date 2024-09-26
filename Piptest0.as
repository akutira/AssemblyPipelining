        lw	0	1	seven	
        lw  0   2   four
        sw  0   0   zero
        add 3   3   3
        beq 1   2   end
        nor 2   4   5
end     halt
seven	.fill	7
four    .fill   4
zero    .fill   0