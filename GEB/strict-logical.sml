(* Basic definitions *)

datatype logical
  = Not of logical
  | P of string
  | And of logical * logical
  | Or of logical * logical
  | Implies of logical * logical

infix And
infix Or
infix Implies

infix and'
infix or
infix implies
	  
fun op and' (s1 : string, s2 : string) : logical = P s1 And P s2
fun op or (s1 : string, s2 : string) : logical = P s1 Or P s2
fun op implies (s1 : string, s2 : string) : logical = P s1 Implies P s2
fun not' (s : string) : logical = Not (P s)
				     

	  
(* Represent as a string *)
	  
fun logicalToString (P s) = s
  | logicalToString (Not l) = "~" ^ logicalToString(l)
  | logicalToString (l1 And l2) = "<" ^ logicalToString(l1) ^ "^" ^ logicalToString(l2) ^ ">"
  | logicalToString (l1 Or l2) = "<" ^ logicalToString(l1) ^ "|v|" ^ logicalToString(l2) ^ ">"
  | logicalToString (l1 Implies l2) = "<" ^ logicalToString(l1) ^ "=>" ^ logicalToString(l2) ^ ">" 

(* A few functions one might want *)
												      
fun addDoubleNot l = Not (Not l)

fun rmDoubleNot (Not (Not l)) = l
  | rmDoubleNot l = l

fun rmDoubleNotAll (l1 And l2) = rmDoubleNotAll l1 And rmDoubleNotAll l2
  | rmDoubleNotAll (l1 Or l2) = rmDoubleNotAll l1 Or rmDoubleNotAll l2
  | rmDoubleNotAll (Not (Not l)) = rmDoubleNotAll l
  | rmDoubleNotAll l = l 
				   
fun deMorgan (Not (l1 And l2)) = Not l1 Or Not l2
  | deMorgan (Not (l1 Or l2)) = Not l1 And Not l2 
  | deMorgan l = l

(* Evaluation functions for different rules *)

(* This first section are the PC rules in GEB, more or less *)
		     
fun addNotx2Eval l = SOME (Not (Not l))

fun rmNotx2Eval (Not (Not l)) = SOME l
  | rmNotx2Eval _ = NONE

fun deMorganEval (Not (l1 Or l2)) = SOME (Not l1 And Not l2)
  | deMorganEval (Not l1 And Not l2) = SOME (Not (l1 Or l2))
  | deMorganEval _ = NONE

fun contraAddEval (l1 Implies l2) = SOME (Not l2 Implies Not l1)
  | contraAddEval _ = NONE

fun contraRmEval (Not l1 Implies Not l2) = SOME (l2 Implies l1)
  | contraRmEval _ = NONE 

(* Apparently the Switcheroo rule *)
fun switchEval (l1 Or l2) = SOME (Not l1 Implies l2)
  | switchEval (Not l1 Implies l2) = SOME (l1 Or l2)
  | switchEval _ = NONE 

(* Now find lists of possible steps *)
			
fun naiveEvalPossible (f : logical -> logical option, l : logical) : logical list =
  let
      val branch = (fn (Cons, l1, l2) => map (fn x => Cons (x, l2)) (naiveEvalPossible (f, l1)) @ map (fn x => Cons (l1, x)) (naiveEvalPossible (f, l2))) 
      val pred = case l 
	          of Not l'           => map Not (naiveEvalPossible (f, l'))
		   | l1 Implies l2    => branch (op Implies, l1, l2)   
	           | l1 Or l2         => branch (op Or, l1, l2)
	           | l1 And l2        => branch (op And, l1, l2)
	           | _                => []
  in
      case f l
       of SOME l' => l'::pred
        | NONE    => pred
  end

val evalOps = [addNotx2Eval, rmNotx2Eval, deMorganEval, contraAddEval, contraRmEval, switchEval]

fun naiveAllPossible (fs : (logical -> logical option) list, l : logical) : logical list =
  case fs
   of [] => []
    | f::fs' => naiveEvalPossible(f, l) @ naiveAllPossible (fs', l) @ [l]
		@ (case l of l1 And l2 => [l1, l2] | _ => [])
		@ (case l of l1 And (l1' Implies l2) => if l1 = l1' then [l2] else [] | _ => [])
		@ (case l of (l1' Implies l2) And l1 => if l1 = l1' then [l2] else [] | _ => []) 

fun mergeLists (comp : 'a * 'a -> int, ls1 : 'a list, ls2 : 'a list) : 'a list =
  case (ls1, ls2)
   of (_, [])              => ls1
    | ([], _)              => ls2
    | (l1::ls1', l2::ls2') => let
	val c = comp (l1, l2)
    in
	if c < 0 then
	    l1::mergeLists(comp, ls1', ls2)
	else if c = 0 then
	    l1::mergeLists(comp, ls1', ls2')
	else
	    l2::mergeLists(comp, ls1, ls2')
    end
				  

fun sortList (comp : 'a * 'a -> int, ls : 'a list, len : int) : 'a list =
  case ls
   of []         => []
    | l::[]      => [l]
    | l1::l2::[] => let
	val c = comp (l1, l2)
    in
	if c < 0 then
	    [l1, l2]
	else if c = 0 then
	    [l1]
	else
	    [l2, l1]
    end
    | _          => let
	val half = len div 2
    in
	mergeLists (comp,
		    sortList (comp, List.take(ls, half), half),
		    sortList (comp, List.drop(ls, half), len - half))
    end

fun inSorted (comp : 'a * 'a -> int, ls : 'a list, a : 'a, len : int) : bool =
  case ls
   of [] => false
    | l::[] => comp (l, a) = 0
    | _ => let
	val pivot = List.nth (ls, len div 2)
	val c = comp (a, pivot)
    in
	if c > 0 then
	    inSorted (comp, List.drop (ls, len div 2 + 1), a, len - len div 2 - 1)
	else if c = 0 then
	    true
	else
	    inSorted (comp, List.take (ls, len div 2), a, len div 2)
    end
	       
(* Some comparison functions and a normal form, in the hopes of creating an efficient
 * means of checking step possibility *)
						      
(* Compare expressions without normalizing. Should be used, for example, if the expressions
 * are already normalized *)
fun directCompare (l1 : logical, l2 : logical) : int =
  case (l1, l2)
   of (P s1, P s2)                       => if s1 > s2 then 1 else if s1 = s2 then 0 else ~1
    | (Not l1', Not l2')                 => directCompare (l1', l2')
    | ((l1' And l2', l3 And l4) |
       (l1' Or l2', l3 Or l4) |
       (l1' Implies l2', l3 Implies l4)) =>
      let
	  val c1 = directCompare (l1', l3)
      in
	  if c1 > 0 then 1 else if c1 < 0 then ~1 else directCompare (l2', l4)
      end
    | (P _, _)                           => ~1
    | (_, P _)                           => 1
    | (Not _, _)                         => ~1
    | (_, Not _)                         => 1
    | (op And _, _)                      => ~1
    | (_, op And _)                      => 1
    | (op Implies _, _)                  => ~1
    | (_, op Implies _)                  => 1
			    
						      
fun normalize (l : logical) : logical =
  let
      val symmetricNorm = (fn (Cons, l1, l2) => case compareAndNormalize (l1, l2)
						 of (i, l1', l2') => if i <= 0 then Cons (l1', l2') else Cons (l2', l1')
			  )							 
  in
      case l
       of l1 And l2 => symmetricNorm (op And, l1, l2)
	| l1 Or l2 => symmetricNorm (op Or, l1, l2)
	| Not l1 => Not (normalize l1)
	| l1 Implies l2 => normalize l1 Implies normalize l2
	| _ => l
  end			     
and compareAndNormalize (l1 : logical, l2 : logical) : int * logical * logical =
    let
	val symCompAndNorm = (fn (Cons, l1, l2, l3, l4) => let
				  val normFirst = case compareAndNormalize (l1, l2)
						   of (i, l1', l2') => if i <= 0 then (l1', l2') else (l2', l1')
				  val normLast = case compareAndNormalize (l3, l4)
						  of (i, l3', l4') => if i <= 0 then (l3', l4') else (l4', l3')
			      in
				  case (normFirst, normLast)
				   of ((l1', l2'), (l3', l4')) => let
				       val c1 = directCompare (l1', l3')
				   in
				       if c1 > 0 then
					   (1, Cons (l1', l2'), Cons (l3', l4'))
				       else if c1 < 0 then
					   (~1, Cons (l1', l2'), Cons (l3', l4'))
				       else
					   (directCompare(l2', l4'), Cons (l1', l2'), Cons (l3', l4'))
				   end
			      end
			     )
    in
	case (l1, l2)
	 of (P s1, P s2)                     => if s1 > s2 then
						    (1, l1, l2)
						else if s1 = s2 then
						    (0, l1, l2)
						else
						    (~1, l1, l2)
	  | (Not l1', Not l2')               => (case compareAndNormalize (l1', l2')
						  of (i, l1'', l2'') => (i, Not l1'', Not l2''))
	  | (l1' And l2', l3 And l4)         => symCompAndNorm (op And, l1', l2', l3, l4)
	  | (l1' Or l2', l3 Or l4)           => symCompAndNorm (op Or, l1', l2', l3, l4)
	  | (l1' Implies l2', l3 Implies l4) => let
	      val c1 = compareAndNormalize (l1', l3)
	  in
	      case c1
	       of (i, l1'', l3') => if i > 0 then
					(1, l1'' Implies normalize l2', l3' Implies normalize l4)
				    else if i < 0 then
					(~1, l1'' Implies normalize l2', l3' Implies normalize l4)
				    else
					(case compareAndNormalize (l2', l4)
					  of (j, l2'', l4') => (j, l1'' Implies l2'', l3' Implies l4'))
	  end
	  | (P _, _)                         => (~1, l1, normalize l2) 
	  | (_, P _)                         => (1, normalize l1, l2)
	  | (Not _, _)                       => (~1, normalize l1, normalize l2)
	  | (_, Not _)                       => (1, normalize l1, normalize l2)
	  | (op And _, _)                    => (~1, normalize l1, normalize l2)
	  | (_, op And _)                    => (1, normalize l1, normalize l2)
	  | (op Implies _, _)                => (~1, normalize l1, normalize l2)
	  | (_, op Implies _)                => (1, normalize l1, normalize l2)
    end
	
fun evalPossible (eval : logical -> logical option, l : logical) : logical list = 
  let
      val symBranch = (fn (Cons, l1, l2) => let
			   val l1' = map (fn x => Cons (x, l2)) (evalPossible (eval, l1))
			   val l2' = map (fn x => Cons (l1, x)) (evalPossible (eval, l2))
		       in
			   mergeLists (directCompare, sortList (directCompare, l1', List.length l1'), sortList (directCompare, l2', List.length l2'))
		       end
		      )
      val pred = case l
		  of Not l'        => map Not (evalPossible (eval, l'))
		   | l1 Implies l2 => mergeLists (directCompare,
						  map (fn x => x Implies l2) (evalPossible (eval, l1)),
						  map (fn x => l1 Implies x) (evalPossible (eval, l2)))
		   | l1 Or l2      => symBranch(op Or, l1, l2)
		   | l1 And l2     => symBranch(op And, l1, l2)
		   | _ => [] 					   
  in
      case eval l
       of SOME l' => mergeLists(directCompare, [l'], pred)
	| NONE => pred
  end


fun allPossible (evals : (logical -> logical option) list, l : logical) : logical list =
  case evals
   of [] => []
    | eval::evals' => mergeLists (directCompare,
				  mergeLists (directCompare, evalPossible (eval, l), allPossible (evals', l)),
				  mergeLists (directCompare,
					      mergeLists (directCompare, case l of l1 And l2 => sortList (directCompare, [l1, l2], 2) | _ => [],
							  l::[]),
					      mergeLists (directCompare, case l of l1 And (l1' Implies l2) => if l1 = l1' then [l2] else [] | _ => [],
							  case l of (l1' Implies l2) And l1 => if l1 = l1' then [l2] else [] | _ => []))) 
      
						      
fun symmetricallyEquivalent (l1 : logical, l2 : logical) : bool =
  case (l1, l2)
   of (Not l1, Not l2)                           => symmetricallyEquivalent (l1, l2)
    | (P s1, P s2)                               => s1 = s2
    | ((op And (l1, l2), op And (l3, l4)) |
       (op Or (l1, l2), op Or (l3, l4)))         => (symmetricallyEquivalent (l1, l3) andalso symmetricallyEquivalent (l2, l4)) orelse
						    (symmetricallyEquivalent (l1, l4) andalso symmetricallyEquivalent (l2, l3))
    | (op Implies (l1, l2), op Implies (l3, l4)) => symmetricallyEquivalent (l1, l3) andalso symmetricallyEquivalent (l2, l4)  
    | _                                          => false

datatype token =
	 PROP of string
	 | LT
	 | GT
	 | AND
	 | OR
	 | IMPLIES
	 | NOT
	       
							
fun lexString (l : char list) : token list =
  case l
   of #"<"::l' => [ LT ] @ lexString(l')
    | #">"::l' => [ GT ] @ lexString(l')
    | #"^"::l' => [ AND ] @ lexString(l')
    | #"|":: #"v":: #"|"::l' => [ OR ] @ lexString(l')
    | #"=":: #">"::l' => [ IMPLIES ] @ lexString(l')
    | #"~"::l' => [ NOT ] @ lexString(l')
    | [] => []
    | _ => lexProp(l)
and lexProp (l : char list) : token list =
    case l
     of [] => []
      | c::l' => (case lexString l'
		   of [] => PROP (String.str c)::[]
		    | (PROP s)::l'' => (PROP (String.str c ^ s))::l''
		    | t::l'' => (PROP (String.str c))::t::l'')
		     
fun parseTokens (l : token list) : (logical option) * (token list) =
  case l
   of LT::l' => (case parseTokens l'
		  of (SOME first, conj::l'') => let
		      val Conj = case conj
				  of AND     => SOME (op And)
				   | OR      => SOME (op Or)
				   | IMPLIES => SOME (op Implies)
				   | _       => NONE
		      val bite = parseTokens l''
		      val second = case bite
				    of (s, _) => s
		      val rest = case bite
				  of (_, GT::l''') => SOME l'''
				   | _             => NONE 
				      
		  in
		      case (second, rest, Conj)
		       of (SOME second', SOME rest', SOME Conj') => (SOME (Conj' (first, second')), rest')
			| _                                      => (NONE, l) 
		  end
		   | _                       => (NONE, l)
		)
    | NOT::l' => (case parseTokens l'
		   of (SOME log, rest) => (SOME (Not log), rest)
		    | _ => (NONE, l) 
		 )
    | (PROP s)::l' => (SOME (P s), l')
    | _ => (NONE, l) 
					       
fun parseString (s : string) : logical option =
  case parseTokens (lexString (explode s))
   of (SOME log, []) => SOME log
    | _               => NONE
			     
fun getLine (indent : string) : string option =
  (TextIO.output (TextIO.stdOut, indent);
   TextIO.flushOut (TextIO.stdOut);
   TextIO.inputLine TextIO.stdIn)

fun getIndent (n : int) : string =
  if n <= 0 then
      ""
  else
      "    " ^ getIndent (n - 1)

fun getPrepend (line : int, indent : int): string =
  let
      val spaces = if line < 9 then
		       "   "
		   else if line < 99 then
		       "  "
		   else if line < 999 then
		       " "
		   else
		       ""
  in
      spaces ^ Int.toString (line + 1) ^ ": " ^ getIndent indent
  end
			 
datatype action = PUSH
		| POP
		| LOG of logical option
		| AND of int option
      
fun parseLine (s : char list) : char list * action =
  case List.take (s, List.length s - 1)
   of []             => ([], LOG NONE)
    | #"["::[]       => ([], PUSH)
    | #"]"::[]       => ([], POP)
    | #":":: #":"::l => ([], AND (Int.fromString (implode l))) 
    | #":"::l        => ([], LOG (parseString (implode l)))
    | c::l           => if c = #"-" orelse c = #"0" orelse
			   (c >= #"1" andalso c <= #"9") then
			    case parseLine (l @ [ #"\n" ])
			     of (l', LOG (SOME log)) => (c::l', LOG (SOME log))
			      | (l', AND (SOME n)) => (c::l', AND (SOME n)) 
			      | _                    => ([], LOG NONE)
			else
			    ([#"0"], LOG (parseString (implode (c::l))))

fun checkFantasyError (thm : (int * int * action), stack : (int * action) list, depth : int) : bool =
  case (thm, stack)
   of ((line, d, _), [])             => d <> 0
    | ((line, d, _), (l, _)::stack') => if line >= l then
					    d <> depth
					else
					    checkFantasyError (thm, stack', depth - 1)
	       
fun step (thms : (int * int * action) list, stack : (int * action) list) : ((int * int * action) list) * ((int * action) list) =	  
  case thms
   of [] => (case getLine "   1: "
	      of NONE => (thms, stack)
	       | SOME s => (case parseLine (explode s)
			     of (_, LOG NONE) => (print "Error: Not well-formed\n";
						  (thms, stack))
			      | (_, LOG (SOME _)) => (print "Error: Does not follow\n";
						    (thms, stack))
			      | (_, PUSH) => ((1, 1, PUSH)::[], [])
			      | (_, POP) => (print "Error: Not in fantasy\n";
					     (thms, stack))
			      | (_, AND (SOME _)) => (print "Error: Reference out of scope\n";
						    (thms, stack))
			      | (_, AND NONE) => (print "Error: Could not interpret reference\n";
						  (thms, stack)) 
			   )
	    )
    | (line, depth, act):: _ =>
      case getLine (getPrepend (line, depth))
       of NONE => (thms, stack)
	| SOME s => (case parseLine (explode s)
		      of (_, LOG NONE) => (print "Error: Not well-formed\n";
					   (thms, stack))
		       | (r, LOG (SOME log)) => (case Int.fromString (implode r)
						  of SOME 0 => (case act
								 of LOG (SOME log') => let
								     val poss = allPossible(evalOps, log')
								 in
								     if inSorted(directCompare, poss, log, List.length poss) then
									 ((line + 1, depth, LOG (SOME log))::thms, stack)
								     else
									 (print "Error: Does not follow\n";
									  (thms, stack))
								 end
								  | PUSH => ((line + 1, depth, LOG (SOME log))::thms, (line + 1, LOG (SOME log))::stack)
								  | _ => (print "Error: Internal error (illegal preceding line)\n";
									  (thms, stack))
							       )
						   | SOME ref' => if (ref' < 0 andalso ref' >= ~1 * line andalso not (checkFantasyError (List.nth (thms, ~1 - ref'), stack, List.length stack))) orelse
								     (ref' > 0 andalso ref' <= line andalso not (checkFantasyError (List.nth (thms, line - ref'), stack, List.length stack))) then
								      let
									  val referred = if ref' < 0 then
											     List.nth (thms, ~1 - ref')
											 else
											     List.nth (thms, line - ref')
								      in
									  case referred
									   of (_, _, LOG (SOME log')) => let
									       val poss = allPossible (evalOps, log')
									   in
									       if inSorted (directCompare, poss, log, List.length poss) then
										   ((line + 1, depth, LOG (SOME log))::thms, stack)
									       else
										   (TextIO.output (TextIO.stdOut, "Error: Does not follow\n");
										    TextIO.flushOut TextIO.stdOut;
										    (thms, stack))
									   end
									    | _ => (TextIO.output (TextIO.stdOut, "Error: Referenced non-theorem\n");
										    TextIO.flushOut TextIO.stdOut;
										    (thms, stack))
								      end
								  else
								      (TextIO.output (TextIO.stdOut, "Error: Reference out of scope\n");
								       TextIO.flushOut TextIO.stdOut;
								       (thms, stack))
						   | _ => (TextIO.output (TextIO.stdOut, "Error: Could not interpret reference\n");
							   TextIO.flushOut TextIO.stdOut;
							   (thms, stack))
						)
		       | (_, AND NONE) => (print "Error: Could not interpret reference\n";
					   (thms, stack))
		       | (r, AND (SOME n)) => (case Int.fromString (implode r)
						of SOME 0 => (case act
							       of LOG (SOME log') => if (n < 0 andalso n >= ~1 * line andalso not (checkFantasyError (List.nth (thms, ~1 - n), stack, List.length stack))) orelse
											(n > 0 andalso n <= line andalso not (checkFantasyError (List.nth (thms, line - n), stack, List.length stack))) then
											 let
											     val referred = if n < 0 then
														List.nth (thms, ~1 - n)
													    else
														List.nth (thms, line - n)
											 in
											     case referred
											      of (_, _, LOG (SOME log'')) => (print (getPrepend (line + 1, depth) ^ logicalToString (log' And log'') ^ "\n");
															      ((line + 2, depth, LOG (SOME (log' And log'')))::(line + 1, depth, AND (SOME n))::thms, stack))
											       | _ => (print "Error: Referenced non-theorem\n";
												       (thms, stack))
											 end
										     else
											 (print "Error: Reference out of scope\n";
											  (thms, stack))
								| _ => (print "Error: Referenced non-theorem\n";
									(thms, stack))
							     )
						 | SOME ref' => if ((ref' < 0 andalso ref' >= ~1 * line andalso not (checkFantasyError (List.nth (thms, ~1 - ref'), stack, List.length stack))) orelse
								    (ref' > 0 andalso ref' <= line andalso not (checkFantasyError (List.nth (thms, line - ref'), stack, List.length stack)))) andalso
								   ((n < 0 andalso n >= ~1 * line andalso not (checkFantasyError (List.nth (thms, ~1 - n), stack, List.length stack))) orelse
								    (n > 0 andalso n <= line andalso not (checkFantasyError (List.nth (thms, line - n), stack, List.length stack)))) then
								    let
									val lReferred = if ref' < 0 then
											    List.nth (thms, ~1 - ref')
											else
											    List.nth (thms, line - ref')
									val rReferred = if n < 0 then
											    List.nth (thms, ~1 - n)
											else
											    List.nth (thms, line - n)
								    in
									case (lReferred, rReferred)
									 of ((_, _, LOG (SOME log')), (_, _, LOG (SOME log''))) => (print (getPrepend (line + 1, depth) ^ logicalToString (log' And log'') ^ "\n");
																    ((line + 2, depth, LOG (SOME (log' And log'')))::(line + 1, depth, AND (SOME n))::thms,
																     stack))
									  | _ => (print "Error: Referenced non-theorem\n";
										  (thms, stack))
								    end
								else
								    (print "Error: Reference out of scope\n";
								     (thms, stack))
						 | _ => (print "Error: Could not interpret reference\n";
							 (thms, stack))
					      )					      
		       | (_, PUSH) => ((line + 1, depth + 1, PUSH)::thms, stack)
		       | (_, POP) => if depth = 0 then
					 (TextIO.output (TextIO.stdOut, "Error: Not in fantasy\n");
					  TextIO.flushOut TextIO.stdOut;
					  (thms, stack))
				     else if List.null stack then
					 (TextIO.output (TextIO.stdOut, "Error: Internal error (stack empty with nonzero depth)\n");
					  TextIO.flushOut TextIO.stdOut;
					  (thms, stack))
				     else
					 case (act, hd stack)
					  of (LOG (SOME consq), (_, LOG (SOME pred))) => (print (getPrepend (line + 1, depth - 1) ^ (logicalToString (pred Implies consq)) ^ "\n");
											  ((line + 2, depth - 1, (LOG (SOME (pred Implies consq))))::(line + 1, depth - 1, POP)::thms,
											   tl stack))
					   | (PUSH, _) => (TextIO.output (TextIO.stdOut, "Error: Empty fantasy\n");
							   TextIO.flushOut TextIO.stdOut;
							   (thms, stack))
					   | _ => (TextIO.output (TextIO.stdOut, "Error: Internal error (illegal preceding line or predicate)\n");
						   TextIO.flushOut TextIO.stdOut;
						   (thms, stack))
		    )
				    
fun run () =
  let
      val state = ref ([], [])
  in
      while true do
	    (state := step (!state))
  end
											  
