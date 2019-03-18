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
      | Inferred(t) => (
          s =>
            print_member_type(t) |> Printf.sprintf("Inferred(%s := %s)", s)
        )
      | Declared(t) => (
          s =>
            print_member_type(t) |> Printf.sprintf("Declared(%s := %s)", s)
        )
      }
    }
  );
