open Globals;
open MemberType;
open Debug_MemberType;

let (|~>) = ((x, y), f) =>
  f(x)
  |> (
    switch (y^) {
    | None => Printf.sprintf("Unanalyzed(%s)")
    | Some(res) =>
      switch (res^) {
      | (t, Expected) => (
          s =>
            print_member_type(t) |> Printf.sprintf("Expected(%s := %s)", s)
        )
      | (t, Declared(is_mutable)) => (
          s =>
            print_member_type(t)
            |> Printf.sprintf(
                 "Declared%s(%s := %s)",
                 is_mutable ? "?" : "",
                 s,
               )
        )
      }
    }
  );
