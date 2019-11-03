/** compose functions together */
let (%) = (f, g, x) => f(x) |> g;

/* option operators */
let some = x => Some(x);

let has_opt =
  fun
  | Some(_) => true
  | None => false;

/** if value found, pipe to transformer */
let (|?>) = (x, f) =>
  switch (x) {
  | Some(y) => f(y)
  | None => None
  };
/** if value found, perform side effect */
let (|*>) = (x, f) =>
  switch (x) {
  | Some(y) => f(y)
  | None => ()
  };
/** unwrap, raise exception if no value found */
let (|!>) = (x, e) =>
  switch (x) {
  | Some(y) => y
  | None => raise(e)
  };
/** check option and generate new option if no value found */
let (|=>) = (x, f) =>
  switch (x) {
  | Some(_) as res => res
  | None => f()
  };
/** check option and use fallback if no value found */
let (||>) = (x, y) =>
  switch (x) {
  | Some(_) as res => res
  | None => y
  };

/** perform side effect g(f(x)) and return f(x) */
let (|-) = (f, g, x) =>
  f(x)
  |> (
    y => {
      g(x);
      y;
    }
  );

/** generate range of numbers */
let rec (<::>) = (start, end_) =>
  if (start >= end_) {
    [];
  } else {
    [start, ...start + 1 <::> end_];
  };
