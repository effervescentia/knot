open Knot.Core;
open Exception;

let upwrap_type =
  fun
  | Some(t) => t
  | None => raise(UnanalyzedTypeReference);

let opt_type_ref = ((_, x)) => upwrap_type(x^);

let (<:=) = (x, y) => {
  Knot.Debug.print_member_type(x)
  |> Log.debug("resolved  %s\n%s", Emoji.heavy_check_mark);

  y := Some(x);
};

let (=@=) = (x, y) => x := Some(y);
