open Globals;

let (|~>) = (x, f) =>
  abandon_ctx(x)
  |> f
  % (
    switch (x^) {
    | Pending(res) => Printf.sprintf("Promise.pending(%s)")
    | Resolved(res, _) => Printf.sprintf("Promise.resolved(%s)")
    }
  );
