

fun addU (s : string) : string option =
  if String.sub (s, String.size s - 1) = #"I" then
      SOME (s ^ "U")
  else
      NONE

fun double (s : string) : string option =
  if s = "M" then
      NONE
  else
      SOME (s ^ String.extract (s, 1, NONE))

fun replaceIII (s : string, n : int) : string list =
  if String.size (String.extract (s, n, NONE)) < 3 then
      []
  else if String.substring (s, n, 3) = "III" then
      (String.substring (s, 0, n) ^ "U" ^ String.extract (s, n + 3, NONE))::(replaceIII (s, n + 1))
  else
      replaceIII (s, n + 1)

fun removeUU (s : string, n : int) : string list =
  if String.size (String.extract (s, n, NONE)) < 2 then
      []
  else if String.substring (s, n, 2) = "UU" then
      (String.substring (s, 0, n) ^ String.extract (s, n + 2, NONE))::(removeUU (s, n + 1))
  else
      removeUU (s, n + 1)

fun step (s : string) : string =
  let
      val poss = replaceIII (s, 1) @ removeUU (s, 1) @
		 (case addU s of SOME s' => [s'] | NONE => []) @
		 (case double s of SOME s' => [s'] | NONE => [])
  in
      case TextIO.inputLine TextIO.stdIn
       of SOME s' => (case List.find (fn x => (x ^ "\n") = s') poss
		       of SOME _ => String.substring (s', 0, String.size s' - 1)
			| NONE => (print "Invalid\n"; s))
  end

fun run () =
  let
      val state = (print "MI\n"; ref "MI")
  in
      while true do
	    (state := step (!state))
  end
      
			       
