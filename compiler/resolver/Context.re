open Knot.Core;
open Exception;

let declared = x => ref((x, Declared(false)));
let declared_mut = x => ref((x, Declared(true)));
let expected = x => ref((x, Expected));
let generic = x => Generic_t(Some(x));
let any = Generic_t(None);

let is_declared = x =>
  switch (x^) {
  | (_, Declared(_)) => true
  | _ => false
  };

let upwrap_type =
  fun
  | Some(t) => t
  | None => raise(UnanalyzedTypeReference);

let opt_type_ref = ((_, x)) => upwrap_type(x^);

let typeof_member = x => opt_type_ref(x) |> (^) |> fst;

let (<:=) = (x, y) => {
  Knot.Debug.print_type_ref(x)
  |> Log.debug("resolved  %s\n%s", Emoji.heavy_check_mark);

  y := Some(x);
};

let (=@=) = (x, y) =>
  switch (x^) {
  | Some(v) => v := y
  | None => x := Some(ref(y))
  };
