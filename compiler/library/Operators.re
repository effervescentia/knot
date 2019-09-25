/** compose functions together */
let (%) = (f, g, x) => f(x) |> g;

/* option operators */

let with_option = (x, f) =>
  fun
  | Some(y) => f(y)
  | None => x;
let with_no_option = f =>
  fun
  | Some(y) => y
  | None => f();

/** optionally pipe to transformer */
let (|?>) = (x, f) => with_option(None, f, x);
/** perform side effect if value found */
let (|*>) = (x, f) => with_option((), f, x);
/** unwrap, raise exception on no value */
let (|!>) = (x, e) => with_no_option(() => raise(e), x);

/** perform side effect g(f(x)) and return f(x) */
let (|-) = (f, g, x) =>
  f(x)
  |> (
    y => {
      g(x);
      y;
    }
  );
