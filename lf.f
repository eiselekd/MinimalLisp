: if immediate ' zbranch ,  >mark ; 
: else immediate ' branch ,  >mark swap >resolve ; 
: again ' branch , <resolve ;
: while immediate _if swap ;
: begin immediate <mark ;
: repeat immediate again >resolve ;
: switch immediate >@markbeg ; 
: switchend immediate [ drop ]  >@resolve ;
: case immediate >@markbeg >@markbeg 
  begin next dup ' : ne 
  while 
    [ dup ] ?, 
    peek-next ' : ne if 
      [ !=  zbranch ] >@mark 
    else
      [ ==  zbranch ] swap >@mark swap >@resolve
    then
  repeat
  [ drop ] drop ; 
: caseend immediate [ branch ] swap >@mark swap 
  >@resolve ;
: lwhile ;
: llet ;
: lsetq ;

: llambda 
  mklambda  
;

: ldefine 
  genv> cdr
  n1-dup cadr eval
  n2-dup car 
  cons        
  cons        
  genv>
  rplacd      
;

: lif
  dcar eval
  ?true if 
   cadr eval
  else 
   caddr eval
  then
;

: evargs  
  dup ?nil if  
    dcdr evargs    
    swap car eval  
    cons
  then
  ;

: evbind
  rot
  begin n2for n2for dup ?nil 
  while
   cons
   cons
  repeat 2drop
  swap drop
  swap drop
;

: evlist 
  0 swap
  begin dup ?nil 
  while
    dcar eval 1 nset
    cdr
  repeat
  drop
; 

: eval            ( l )
   n0-dup tag switch
   case :INT :STR :
   caseend
   case :SYM :
     lassq
   caseend
   case :FUNC :
     nil
   caseend
   case :CONS :
     dcdr swap
     car eval
     apply
   caseend
   switchend
;

: apply 
  n0-dup flags :ARGEVAL & if
    n1-dup evargs n1-set    
  then
  n0-dup tag switch
  case :OP :
    elispop
  caseend
  case :FUNC :CONS : 
    etop>
    n1-dup flags :ARGBIND & if
      n2-dup n2-dup func car evbind
    then
    >e
    dup ?fword if
      execute
    else
      func cdr 1 nthdrop evlist
    then
    edrop
  caseend
  switchend
;
