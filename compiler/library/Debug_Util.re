open Globals;

let (|~>) = (x, f) =>
  abandon_ctx(x)
  |> f
  % (
    switch (x^) {
    | Pending(res) => Printf.sprintf("PENDING(%s)")
    | Resolved(res, _) => Printf.sprintf("RESOLVED(%s)")
    }
  );
