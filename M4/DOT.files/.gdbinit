# add-auto-load-safe-path /home/m4/M4_trace/.gdbinit
add-auto-load-safe-path /home/m4/

#-----------------------------------------------------------------------------
define exit
  quit
end
#-----------------------------------------------------------------------------
define M4_cdb1
   set $c = (unsigned char [8])$arg0
   set $s = ((long)$arg1) & 0xffff
   printf "\n  sg_raw -r 0x%x ${DEV} %02x %02x %02x %02x %02x %02x %02x %02x",$s,$c[0],$c[1],$c[2],$c[3],$c[4],$c[5],$c[6],$c[7]
end
define M4_cdb2
   set $c = (char [8])$arg0
   printf " %02x %02x %02x %02x %02x %02x %02x %02x" ,$c[0],$c[1],$c[2],$c[3],$c[4],$c[5],$c[6],$c[7]
end
#-----------------------------------------------------------------------------
define M4_tb_display
  printf "%5d %-18s %3d %6d %s\n      %s:%u:%s  %s\n      0x%16.16lx", $arg0, M4_str_trace_type[M4_tb.trace_buf[$arg0].tp], M4_tb.trace_buf[$arg0].cpu, M4_tb.trace_buf[$arg0].pid, M4_tb.trace_buf[$arg0].comm, M4_tb.trace_buf[$arg0].file, M4_tb.trace_buf[$arg0].line, M4_tb.trace_buf[$arg0].function, M4_tb.trace_buf[$arg0].str, M4_tb.trace_buf[$arg0].arg1
  set $x = 1
  set $y = M4_tb.trace_buf[$arg0].n_linked_list
  while $x <= $y
    if $x % 4 == 0
      printf "\n     "
    end
    printf " 0x%16.16lx", M4_tb.trace_buf[$arg0].linked_list[$x-1]
    set $x = $x + 1
  end
  if (M4_tb.trace_buf[$arg0].tp == M4_type_iscsi_cmd_cdb && M4_tb.trace_buf[$arg0].n_linked_list >= 3)
    M4_cdb1 M4_tb.trace_buf[$arg0].linked_list[2] M4_tb.trace_buf[$arg0].linked_list[1]
  else
    if (M4_tb.trace_buf[$arg0].tp == M4_type_se_cmd_cdb && M4_tb.trace_buf[$arg0].n_linked_list >= 3)
      M4_cdb1 M4_tb.trace_buf[$arg0].linked_list[1] M4_tb.trace_buf[$arg0].linked_list[0]
      M4_cdb2 M4_tb.trace_buf[$arg0].linked_list[2] M4_tb.trace_buf[$arg0].linked_list[0]
    else
      if (M4_tb.trace_buf[$arg0].tp == M4_type_se_cmd_sz_cdb && M4_tb.trace_buf[$arg0].n_linked_list >= 3)
        M4_cdb1 M4_tb.trace_buf[$arg0].linked_list[1] M4_tb.trace_buf[$arg0].linked_list[0]
        M4_cdb2 M4_tb.trace_buf[$arg0].linked_list[2] M4_tb.trace_buf[$arg0].linked_list[0]
      end
    end
  end
  printf "\n"
end

document M4_tb_display
  Internal macro to print an M4_tb array entry.
end
#-----------------------------------------------------------------------------
define M4_tb
  printf "Index Type               CPU    PID Task-Name\n"
  printf "      FILE:LINE:FUNCTION  String-from-program\n"
  printf "          First-Argument    Second-Argument ...\n"
  set $i = M4_tb.next - 1
  while $i >= 0
    M4_tb_display $i
    set $i = $i - 1
  end
  if M4_tb.wrapped > 0
    set $i = 12000 - 1
    while $i > M4_tb.next
      M4_tb_display $i
      set $i = $i - 1
    end
  end
  echo Done with M4_tb macro.\n
end

document M4_tb
  Print out the M4_tb trace structure in reverse order.
end
#-----------------------------------------------------------------------------
define M4_list_head
    set $i = (struct list_head *)$arg0
    if $i == 0
	printf "list_head == 0?  0x%16.16lx\n", $i
    else if $i->next == 0
	printf "list_head(0x%16.16lx)->next == 0?  0x%16.16lx\n", $i, $i->next
    else if $i->prev == 0
	printf "list_head(0x%16.16lx)->prev == 0?  0x%16.16lx\n", $i, $i->prev
    else
	set $n = ((struct list_head *)$i)->next
	printf "list_head=0x%16.16lx  next=0x%16.16lx  prev=0x%16.16lx\n", $i, $n, $i->prev
	if $i == $n
	    printf "  End of empty list\n"
	    if $i != $i->prev
		printf "  NOTE: prev does not point to list_head!\n"
	    end
	else
	    while $n != $i
		printf "         =0x%16.16lx  next=0x%16.16lx  prev=0x%16.16lx\n", $n, $n->next, $n->prev
		if $n->next == 0
		    printf "   ->next == 0?\n"
		    set $n = $i
		else if $n->prev == 0
			printf "   ->prev == 0?\n"
			set $n = $i
		else
		    set $n = $n ->next
		end
	    end
	end
    end
end

document M4_list_head
  Print out the 'struct list_head *' at passed in argument.
end
#-----------------------------------------------------------------------------
