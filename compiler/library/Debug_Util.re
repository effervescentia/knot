open Globals;
open MemberType;

let (|~>) = ((x, y), f) =>
  f(x)
  |> (
    switch (y^ ^) {
    | Unanalyzed => Printf.sprintf("Unanalyzed(%s)")
    | Synthetic(_) => Printf.sprintf("Synthetic(%s)")
    | Resolved(_) => Printf.sprintf("Resolved(%s)")
    }
  );
