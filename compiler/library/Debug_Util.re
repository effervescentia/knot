open Globals;
open MemberType;
open Debug_MemberType;

/** print type as string */
let (|~>) = ((x, y), f) =>
  f(x)
  |> (
    switch (y^) {
    | None => Printf.sprintf("Unanalyzed(%s)")
    | Some(res) =>
      switch (res) {
      | t => print_member_type(t) |> Printf.sprintf("Typed(%s := %s)")
      }
    }
  );
