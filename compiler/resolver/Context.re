open Knot.Core;
open Exception;

let declared = x => ref(Declared(x));
let inferred = x => ref(Inferred(x));
let generic = x => Generic_t(Some(x));
let any = Generic_t(None);

let is_declared = x =>
  switch (x^) {
  | Declared(_) => true
  | _ => false
  };

let upwrap_type =
  fun
  | Some(t) => t
  | None => raise(UnanalyzedTypeReference);

let opt_type_ref = ((_, x)) => upwrap_type(x^);

let typeof_ref = x => x^ |> typeof;

let typeof_member = x => opt_type_ref(x) |> typeof_ref;

let (<:=) = (x, y) => {
  switch (typeof_ref(x)) {
  | Module_t(_) as res =>
    Knot.Debug.print_member_type(res) |> Log.info("RESOLVED: %s")
  | _ => ()
  };
  y := Some(x);
};

let (=@=) = (x, y) =>
  switch (x^) {
  | Some(v) => v := y
  | None => x := Some(ref(y))
  };
let (=:=) = (x, y) => x =@= Declared(y);
let (=.=) = (x, y) => x =@= Inferred(y);
