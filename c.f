: if immediate ' zbranch ,  >mark ; 

: else immediate ' branch ,  >mark swap >resolve ; 

: (?DO)  2dup 2>r rrot == if 1 rpick else 0 then ;

: ?do immediate  ' (?do) , <mark ' vbranch , >mark swap ;

: again ' branch , <resolve ;

: unloop r> 2r> 2drop >r ;
   
: repeat again >resolve ;

: +loop repeat ' unloop , ; 

: loop immediate 1 LITERAL +loop ; 

: fib-iter  0 1 rot 0 ?do over + swap loop drop ;

20 fib-iter . 

end

