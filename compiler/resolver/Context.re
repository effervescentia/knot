open Knot.Core;
open Exception;

let (<:=) = (x, y) => {
  Knot.Debug.print_member_type(x)
  |> Log.debug("resolved  %s\n%s", Emoji.heavy_check_mark);

  y := Some(x);
};

let (=@=) = (x, y) => x := Some(y);
